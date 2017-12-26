const times = require("lodash.times");
const { CSG } = require("@jscad/csg");
const {
  genCasing,
  genScrewes,
  transformModelForPreview,

  BUTTON_SIZE,
  BUTTON_OFFSET,
  BUTTON_CASE_OFFSET,
  MONOME_SIZE_MOD
} = require("./sparknome-common");

module.exports = () => {
  let casing = genCasing();

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
  const finalModel = transformModelForPreview(model);
  const finalParts = [...buttons, ...screwes].map(transformModelForPreview);

  return {
    model: finalModel,
    parts: finalParts
  };
};
