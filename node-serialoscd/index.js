const SerialPort = require("serialport");
const freeUdpPort = require("udp-free-port");
const fs = require("fs");
const program = require("commander");
const { UdpReceiver, UdpSender } = require("omgosc");

// init

program
  .version(require("./package.json").version)
  .usage("node-serialoscd MONOME_TTY")
  .option("-d, --debug", "show debugging information")
  .parse(process.argv);

if (!program.args.length) {
  program.help();
  process.exit(0);
}

// logging

const log = program.debug ? console.log : () => {};
const stringify = json => JSON.stringify(json, null, 2);

// consts

const BAUD_RATE = 115200;
const MASTER_RECEIVER_PORT = 12002;
const DEVICE = "monome";
const DEFAULT_PREFIX = `/${DEVICE}`;

const HARDWARE_HANDLERS = {
  "/grid/led/set": params => [
    params[2] === 0 ? 0x10 : 0x11,
    params[0],
    params[1]
  ],
  "/grid/led/all": params => [params[0] === 0 ? 0x12 : 0x13],
  "/grid/led/map": params => [0x14, params[0], params[1], params[2]],
  "/grid/led/row": params => [0x15, params[0], params[1], params[2]],
  "/grid/led/col": params => [0x16, params[0], params[1], params[2]],
  "/grid/led/intensity": params => [0x17, params[0]],
  "/grid/led/level/set": params => [0x18, params[0], params[1], params[2]],
  "/grid/led/level/all": params => [0x19, params[0]],
  "/grid/led/level/map": params => [0x1a, params[0], params[1], params[2]],
  "/grid/led/level/row": params => [0x1b, params[0], params[1], params[2]],
  "/grid/led/level/col": params => [0x1c, params[0], params[1], params[2]]
};

// serial

const ttyFile = program.args[0];
const sysId = ttyFile.split("/")[2].replace(/(tty|cu)./, "");

if (!fs.existsSync(ttyFile)) {
  console.log(`${ttyFile} doesn't exist`);
  process.exit(1);
}

console.log(`opening ${ttyFile}...`);
const port = new SerialPort(ttyFile, { baudRate: BAUD_RATE });

// osc

const masterReceiver = new UdpReceiver(MASTER_RECEIVER_PORT);
let connections = {};

port.on("open", err => {
  if (err) throw err;

  console.log("ready!");

  masterReceiver.on("", e => {
    log(">>> master");
    log(stringify(e));
    log();

    // start communicating on /serialosc/list
    if (e.path === "/serialosc/list") {
      const oscHost = e.params[0];
      const oscPort = e.params[1];

      freeUdpPort((err, sysOscPort) => {
        if (err) throw err;

        const oscAddress = `${oscHost}:${oscPort}`;

        // re-use existing values
        const deviceOscHost = connections[oscAddress]
          ? connections[oscAddress].deviceOscHost
          : oscHost;

        const deviceOscPort = connections[oscAddress]
          ? connections[oscAddress].deviceOscPort
          : oscPort;

        // create communication channels
        const receiver = new UdpReceiver(sysOscPort);
        const sysSender = new UdpSender(oscHost, oscPort);
        const deviceSender = new UdpSender(deviceOscHost, deviceOscPort);

        // store the connection
        connections[oscAddress] = {
          prefix: DEFAULT_PREFIX,
          sysSender,
          deviceSender,
          receiver,
          sysOscPort,
          deviceOscHost,
          deviceOscPort
        };
        const connection = connections[oscAddress];

        // notify listener about our device
        sysSender.send("/serialosc/device", "ssi", [sysId, DEVICE, sysOscPort]);

        // listen to sys messages
        receiver.on("", e => {
          log(">>> receiver");
          log(stringify(e));
          log();

          // update port if needed
          if (e.path === "/sys/port") {
            const newDeviceOscPort = e.params[0];
            const newDeviceSender = new UdpSender(
              connection.oscHost,
              newDeviceOscPort
            );

            connection.deviceSender.close();
            connection.deviceSender = newDeviceSender;
            connection.deviceOscPort = newDeviceOscPort;

            newDeviceSender.send("/sys/port", "i", [newDeviceOscPort]);

            return;
          }

          // update host if needed
          if (e.path === "/sys/host") {
            const newDeviceOscHost = e.params[0];
            const newDeviceSender = new UdpSender(
              newDeviceOscHost,
              connection.deviceOscPort
            );

            connection.deviceSender.close();
            connection.deviceSender = newDeviceSender;
            connection.deviceOscHost = newDeviceOscHost;

            newDeviceSender.send("/sys/host", "i", [newDeviceOscHost]);

            return;
          }

          if (e.path === "/sys/prefix") {
            connection.prefix = e.params[0];

            return;
          }

          // dump all the values we have
          if (e.path === "/sys/info") {
            const sysMessages = [
              { path: "/sys/id", typetag: "s", params: [sysId] },
              { path: "/sys/size", typetag: "ii", params: [8, 8] },
              {
                path: "/sys/host",
                typetag: "s",
                params: [connection.deviceOscHost]
              },
              {
                path: "/sys/port",
                typetag: "i",
                params: [connection.deviceOscPort]
              },
              {
                path: "/sys/prefix",
                typetag: "s",
                params: [connection.prefix]
              },
              { path: "/sys/rotation", typetag: "i", params: [0] }
            ];

            sysMessages.forEach(({ path, typetag, params }) => {
              connection.deviceSender.send(path, typetag, params);
            });

            return;
          }

          // otherwise handle hardware communications

          const pathWithoutPrefix = e.path.replace(connection.prefix, "");

          if (HARDWARE_HANDLERS[pathWithoutPrefix]) {
            const buffer = Buffer.from(
              HARDWARE_HANDLERS[pathWithoutPrefix](e.params)
            );

            // send hardware data to monome
            port.write(buffer);

            return;
          }
        });
      });
    }
  });
});

// key events from monome

port.on("data", d => {
  const hex = d.toString("hex");
  if (hex.length !== 6) return;

  const msg = hex.substring(0, 2);
  const x = hex.substring(2, 4);
  const y = hex.substring(4, 6);

  log(">>> key");
  log(stringify({ msg, x, y }));
  log();

  // notify all conections
  Object.keys(connections).forEach(key => {
    const { deviceSender, prefix } = connections[key];

    deviceSender.send(`${prefix}/grid/key`, "iii", [
      parseInt(x),
      parseInt(y),
      msg === "20" ? 0 : 1
    ]);
  });
});
