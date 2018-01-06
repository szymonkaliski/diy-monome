const times = require("lodash.times");
const { CSG } = require("@jscad/csg");
const { genCasing, transformModelForPreview } = require("./common");

const {
  genScrewes,
  CASE_SIZE,
  CASE_HEIGHT,
  BUTTON_SIZE,
  BUTTON_OFFSET,
  BUTTON_CASE_OFFSET,
  MONOME_SIZE_MOD
} = require("./sparknome-common");

module.exports = () => {
  let casing = genCasing({
    caseHeight: CASE_HEIGHT,
    caseSize: CASE_SIZE * MONOME_SIZE_MOD
  });

  // button holes
  let buttons = [];

  times(4 * MONOME_SIZE_MOD).forEach(i =>
    times(4 * MONOME_SIZE_MOD).forEach(j => {
      const x = i * (BUTTON_SIZE + BUTTON_OFFSET) + BUTTON_CASE_OFFSET;
      const y = j * (BUTTON_SIZE + BUTTON_OFFSET) + BUTTON_CASE_OFFSET;

      const button = CSG.cube({
        corner1: [0, 0, 1],
        corner2: [BUTTON_SIZE, BUTTON_SIZE, -1]
      }).transform(CSG.Matrix4x4.translation([x, y, 0]));

      buttons.push(button);

      casing = casing.subtract(button);
    })
  );

  // screwes
  const { model, screwes } = genScrewes(casing);

  // final model
  const finalModel = transformModelForPreview(model, {
    caseSize: CASE_SIZE * MONOME_SIZE_MOD
  });

  const finalParts = [...buttons, ...screwes].map(model =>
    transformModelForPreview(model, { caseSize: CASE_SIZE * MONOME_SIZE_MOD })
  );

  return {
    model: finalModel,
    parts: finalParts
  };
};
