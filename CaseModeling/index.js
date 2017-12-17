const times = require("lodash.times");
const { CSG } = require("@jscad/csg");
const createPreview = require("./preview");

const BUTTON_SIZE = 1.5;
const BUTTON_OFFSET = 1.0;
const BUTTON_CASE_OFFSET = 0.5;
const CASE_SIZE = 10;

const casing = CSG.cube({
  corner1: [0, 0, -0.1],
  corner2: [CASE_SIZE, CASE_SIZE, 0.0]
});

let buttons = [];

let model = casing;

times(4).forEach(i =>
  times(4).forEach(j => {
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

const transformModel = model =>
  model
    .transform(CSG.Matrix4x4.translation([-CASE_SIZE / 2, -CASE_SIZE / 2, 0]))
    .transform(CSG.Matrix4x4.rotationX(90));

createPreview(transformModel(model), [...buttons].map(transformModel));
