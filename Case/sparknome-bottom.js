const {
  genCasing,
  genScrewes,
  transformModelForPreview
} = require("./sparknome-common");

module.exports = () => {
  // screwes
  const { model, screwes } = genScrewes(genCasing());

  // final model
  const finalModel = transformModelForPreview(model);
  const finalParts = [...screwes].map(transformModelForPreview);

  return {
    model: finalModel,
    parts: finalParts
  };
};
