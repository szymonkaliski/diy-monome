const times = require("lodash.times");
const { CSG } = require("@jscad/csg");

const BUTTON_SIZE = 1.5;
const BUTTON_OFFSET = 1.0;
const BUTTON_CASE_OFFSET = 0.5;
const CASE_SIZE = 10;
const SCREW_SIZE = 0.2;
const MONOME_SIZE_MOD = 2; // 1 = 4x4, 2 = 8x8

const casing = CSG.cube({
  corner1: [0, 0, -0.1],
  corner2: [CASE_SIZE * MONOME_SIZE_MOD, CASE_SIZE * MONOME_SIZE_MOD, 0.0]
});

let buttons = [];

// button holes
let model = casing;

times(4 * MONOME_SIZE_MOD).forEach(i =>
  times(4 * MONOME_SIZE_MOD).forEach(j => {
    const x = i * (BUTTON_SIZE + BUTTON_OFFSET) + BUTTON_CASE_OFFSET;
    const y = j * (BUTTON_SIZE + BUTTON_OFFSET) + BUTTON_CASE_OFFSET;

    const button = CSG.cube({
      corner1: [0, 0, 1],
      corner2: [BUTTON_SIZE, BUTTON_SIZE, -1]
    }).transform(CSG.Matrix4x4.translation([x, y, 0]));

    buttons.push(button);

    model = model.subtract(button);
  })
);

// screw holes
let screwes = [];

const fourScrewes = (offset = [0, 0]) => {
  times(2 * MONOME_SIZE_MOD).forEach(i => {
    times(2 * MONOME_SIZE_MOD).forEach(j => {
      const ox = offset[0] + 0.1 + i * CASE_SIZE / 2;
      const oy = offset[1] + 0.1 + j * CASE_SIZE / 2;

      const screw = CSG.cylinder({
        start: [0, 0, 1],
        end: [SCREW_SIZE, SCREW_SIZE, -1],
        radius: SCREW_SIZE / 2
      }).transform(CSG.Matrix4x4.translation([ox, oy, 0]));

      screwes.push(screw);

      model = model.subtract(screw);
    });
  });
};

fourScrewes([0, 0]);
fourScrewes([CASE_SIZE / 2 - 0.1 * 2 - SCREW_SIZE, 0]);
fourScrewes([0, CASE_SIZE / 2 - 0.1 * 2 - SCREW_SIZE]);
fourScrewes([
  CASE_SIZE / 2 - 0.1 * 2 - SCREW_SIZE,
  CASE_SIZE / 2 - 0.1 * 2 - SCREW_SIZE
]);

// final model
const transformModel = model =>
  model
    .transform(
      CSG.Matrix4x4.translation([
        -CASE_SIZE * MONOME_SIZE_MOD / 2,
        -CASE_SIZE * MONOME_SIZE_MOD / 2,
        0
      ])
    )
    .transform(CSG.Matrix4x4.rotationX(90));

const finalModel = transformModel(model);
const finalParts = [...buttons, ...screwes].map(transformModel);

module.exports = {
  model: finalModel,
  parts: finalParts
};
