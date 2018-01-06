const times = require("lodash.times");
const { CSG } = require("@jscad/csg");

const CASE_SIZE = 6;
const CASE_HEIGHT = 0.2;
const MONOME_SIZE_MOD = 2; // 1 = 4x4, 2 = 8x8

const BUTTON_SIZE = 1.2;
const BUTTON_OFFSET = 0.3;
const BUTTON_CASE_OFFSET = 0.15;
const SCREW_SIZE = 0.3;
const SCREW_OFFSET = 1.5;

const genScrewes = model => {
  let screwes = [];

  const makeScrew = (x, y) => {
    const screw = CSG.cylinder({
      start: [0, 0, 1],
      end: [0, 0, -1],
      radius: SCREW_SIZE / 2,
      resolution: 8
    }).transform(CSG.Matrix4x4.translation([x, y, 0]));

    screwes.push(screw);

    model = model.subtract(screw);
  };

  times(MONOME_SIZE_MOD, i => {
    times(MONOME_SIZE_MOD, j => {
      const ox = i * CASE_SIZE;
      const oy = j * CASE_SIZE;

      makeScrew(ox + SCREW_OFFSET, oy + SCREW_OFFSET);
      makeScrew(ox + CASE_SIZE - SCREW_OFFSET, oy + SCREW_OFFSET);
      makeScrew(ox + SCREW_OFFSET, oy + CASE_SIZE - SCREW_OFFSET);
      makeScrew(ox + CASE_SIZE - SCREW_OFFSET, oy + CASE_SIZE - SCREW_OFFSET);
    });
  });

  return { screwes, model };
};

module.exports = {
  CASE_HEIGHT,
  CASE_SIZE,
  MONOME_SIZE_MOD,
  BUTTON_SIZE,
  BUTTON_OFFSET,
  BUTTON_CASE_OFFSET,
  SCREW_SIZE,
  genScrewes
};
