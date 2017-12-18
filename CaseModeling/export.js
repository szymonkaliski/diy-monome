const { stlSerializer } = require("@jscad/io");
const { model } = require("./model");
const fs = require("fs");

const rawData = stlSerializer.serialize(model, { binary: false });

fs.writeFileSync("monome-case.stl", rawData.join());
