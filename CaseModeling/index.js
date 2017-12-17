const times = require("lodash.times");
const { CSG, CAG } = require("@jscad/csg");
const createPreview = require("./preview");

const RESOLUTION = 50;

const BUTTON_SIZE = 1.4;
const BUTTON_OFFSET = 0.8;

const casing = CSG.cube({ radius: [10, 10, 0.1] });

let model = casing;

times(4).forEach(i =>
  times(4).forEach(j => {
    const x = i * (BUTTON_SIZE * 2 + BUTTON_OFFSET);
    const y = j * (BUTTON_SIZE * 2 + BUTTON_OFFSET);

    const button = CSG.cube({
      radius: [BUTTON_SIZE, BUTTON_SIZE, 2],
      center: [x - 3.2, y - 3.2, -1] // TODO: why 3.2?
    });

    model = model.subtract(button);
  })
);

createPreview(model.transform(CSG.Matrix4x4.rotationX(90)));
