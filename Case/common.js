const { CSG } = require("@jscad/csg");

const genCasing = ({ caseHeight, caseSize }) => {
  return CSG.cube({
    corner1: [0, 0, -caseHeight],
    corner2: [caseSize, caseSize, 0.0]
  });
};

const transformModelForPreview = (model, { caseSize }) =>
  model
    .transform(CSG.Matrix4x4.translation([-caseSize / 2, -caseSize / 2, 0]))
    .transform(CSG.Matrix4x4.rotationX(90));

module.exports = {
  genCasing,
  transformModelForPreview
};
