const times = require("lodash.times");
const { CSG } = require("@jscad/csg");

const CASE_SIZE = 10;
const CASE_HEIGHT = 0.5;
const BUTTON_SIZE = 1.6;
const BUTTON_OFFSET = 0.9;
const BUTTON_CASE_OFFSET = 0.45;
const SCREW_SIZE = 0.3;
const MONOME_SIZE_MOD = 2; // 1 = 4x4, 2 = 8x8

const genCasing = () => {
  return CSG.cube({
    corner1: [0, 0, -CASE_HEIGHT],
    corner2: [CASE_SIZE * MONOME_SIZE_MOD, CASE_SIZE * MONOME_SIZE_MOD, 0.0]
  });
};

const genScrewes = model => {
  // screw holes
  let screwes = [];

  const fourScrewes = (offset = [0, 0]) => {
    times(2 * MONOME_SIZE_MOD).forEach(i => {
      times(2 * MONOME_SIZE_MOD).forEach(j => {
        const ox = offset[0] + 0.1 + i * CASE_SIZE / 2 + SCREW_SIZE / 2;
        const oy = offset[1] + 0.1 + j * CASE_SIZE / 2 + SCREW_SIZE / 2;

        const screw = CSG.cylinder({
          start: [0, 0, 1],
          end: [0, 0, -1],
          radius: SCREW_SIZE / 2,
          resolution: 8
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

  return { screwes, model };
};

const transformModelForPreview = model =>
  model
    .transform(
      CSG.Matrix4x4.translation([
        -CASE_SIZE * MONOME_SIZE_MOD / 2,
        -CASE_SIZE * MONOME_SIZE_MOD / 2,
        0
      ])
    )
    .transform(CSG.Matrix4x4.rotationX(90));

module.exports = {
  genCasing,
  genScrewes,
  transformModelForPreview,

  CASE_SIZE,
  CASE_HEIGHT,
  BUTTON_SIZE,
  BUTTON_OFFSET,
  BUTTON_CASE_OFFSET,
  SCREW_SIZE,
  MONOME_SIZE_MOD
};
