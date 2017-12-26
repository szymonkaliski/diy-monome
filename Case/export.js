if (!process.argv[2]) {
  console.log("pass model.js script path as argument to export to stl");
  process.exit(0);
}

const path = require("path");
const modelScriptPath = `./${path.basename(process.argv[2])}`;

const { CSG } = require("@jscad/csg");
const { stlSerializer } = require("@jscad/io");
const { model } = require(modelScriptPath)();
const fs = require("fs");

const MM = 10;

const scaledModel = model
  .transform(CSG.Matrix4x4.rotationX(-90))
  .transform(CSG.Matrix4x4.scaling([MM, MM, MM]));

const rawData = stlSerializer.serialize(scaledModel, { binary: false });

const stlName = `${path.basename(
  modelScriptPath,
  path.extname(modelScriptPath)
)}.stl`;
const stlPath = `data/${stlName}`;

fs.writeFileSync(stlPath, rawData.join());

console.log(`exported ${path.basename(modelScriptPath)} to ${stlPath}`);
