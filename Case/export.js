const { CSG } = require("@jscad/csg");
const { stlSerializer } = require("@jscad/io");
const { model } = require("./model");
const fs = require("fs");

const MM = 10;

const scaledModel = model
  .transform(CSG.Matrix4x4.rotationX(-90))
  .transform(CSG.Matrix4x4.scaling([MM, MM, MM]));

const rawData = stlSerializer.serialize(scaledModel, { binary: false });

fs.writeFileSync("data/monome-case.stl", rawData.join());
