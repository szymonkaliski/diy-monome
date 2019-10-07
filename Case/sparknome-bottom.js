const { genCasing, transformModelForPreview } = require("./common");
const {
  genScrewes,
  CASE_SIZE,
  CASE_HEIGHT,
  MONOME_SIZE_MOD
} = require("./sparknome-common");

module.exports = () => {
  let casing = genCasing({
    caseHeight: CASE_HEIGHT,
    caseSize: CASE_SIZE * MONOME_SIZE_MOD
  });

  // screwes
  const { model, screwes } = genScrewes(casing);

  // final model
  const finalModel = transformModelForPreview(model, {
    caseSize: CASE_SIZE * MONOME_SIZE_MOD
  });

  const finalParts = [...screwes].map(model =>
    transformModelForPreview(model, { caseSize: CASE_SIZE * MONOME_SIZE_MOD })
  );

  return {
    model: finalModel,
    parts: finalParts
  };
};
