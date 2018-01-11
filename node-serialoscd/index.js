const SerialPort = require("serialport");
const freeUdpPort = require("udp-free-port");
const { UdpReceiver, UdpSender } = require("omgosc");

const MASTER_RECEIVER_PORT = 12002;
const BAUD_RATE = 115200;
const PREFIX = "/monome";

const masterReceiver = new UdpReceiver(MASTER_RECEIVER_PORT);
let connections = {};

const port = new SerialPort("/dev/tty.usbmodem1411", { baudRate: BAUD_RATE });

port.on("data", d => {
  const hex = d.toString("hex");
  if (hex.length !== 6) return;

  const msg = hex.substring(0, 2);
  const x = hex.substring(2, 4);
  const y = hex.substring(4, 6);

  console.log(msg, x, y);

  Object.keys(connections).forEach(key => {
    // FIXME: what is sender2??
    connections[key].sender2.send(`${PREFIX}/grid/key`, "iii", [
      parseInt(x),
      parseInt(y),
      msg === "20" ? 0 : 1
    ]);
  });
});

port.on("open", err => {
  if (err) throw err;

  const hardwareHandlers = {
    "/grid/led/set": params => [params[2] === 0 ? 0x10 : 0x11, params[0], params[1]],
    "/grid/led/all": params => [params[0] === 0 ? 0x12 : 0x13],
    "/grid/led/map": params => [0x14, params[0], params[1], params[2]],
    "/grid/led/row": params => [0x15, params[0], params[1], params[2]],
    "/grid/led/col": params => [0x16, params[0], params[1], params[2]],
    "/grid/led/intensity": params => [0x17, params[0]],
    "/grid/led/level/set": params => [0x18, params[0], params[1], params[2]],
    "/grid/led/level/all": params => [0x19, params[0]],
    "/grid/led/level/map": params => [0x1A, params[0], params[1], params[2]],
    "/grid/led/level/row": params => [0x1B, params[0], params[1], params[2]],
    "/grid/led/level/col": params => [0x1C, params[0], params[1], params[2]],
  };

  masterReceiver.on("", e => {
    console.log("=== master ===");
    console.log(e);
    console.log();

    if (e.path === "/serialosc/list") {
      const oscHost = e.params[0];
      const oscPort = e.params[1];

      freeUdpPort((err, udpPort) => {
        if (err) throw err;

        const oscAddress = `${oscHost}:${oscPort}`;

        const receiver = new UdpReceiver(udpPort);
        const sender = new UdpSender(oscHost, oscPort);

        connections[oscAddress] = { sender, receiver, udpPort };

        sender.send("/serialosc/device", "ssi", [
          "1411", // TODO
          "monome", // TODO
          udpPort
        ]);

        let sender2;

        receiver.on("", e => {
          console.log("=== receiver ===");
          console.log(e)
          console.log();

          if (e.path === "/sys/port") {
            const oscPort2 = e.params[0]

            sender2 = new UdpSender("localhost", oscPort2);
            sender2.send("/sys/port", "i", [oscPort2])

            connections[oscAddress].sender2 = sender2;

            return;
          }

          if (e.path === "/sys/host") {
            const oscHost2 = e.params[0];

            sender2.send("/sys/host", "i", [oscHost2])

            return;
          }

          if (e.path === "/sys/info") {
            const sysMessages = [
              { path: "/sys/id", typetag: "s", params: ["1411"] },
              { path: "/sys/size", typetag: "ii", params: [8, 8] },
              // { path: "/sys/host", typetag: "s", params: ["localhost"] },
              // { path: "/sys/port", typetag: "i", params: [13297] }, // TODO: ?
              { path: "/sys/prefix", typetag: "s", params: [PREFIX] },
              { path: "/sys/rotation", typetag: "i", params: [0] }
            ];

            sysMessages.forEach(({ path, typetag, params }) => {
              sender2.send(path, typetag, params);
            });
          }
          else {
            const pathWithoutPrefix = e.path.replace(PREFIX, "");

            if (!hardwareHandlers[pathWithoutPrefix]) {
              console.log(`unhandled path: ${pathWithoutPrefix}`);
              return;
            }

            const buffer = Buffer.from(hardwareHandlers[pathWithoutPrefix](e.params));
            port.write(buffer);
          }
        });
      });
    }
  });
});
