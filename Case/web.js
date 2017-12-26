const createPreview = require("./preview");

const sparknomeTop = require("./sparknome-top");
const sparknomeBottom = require("./sparknome-bottom");
const trellinomeTop = require("./trellinome-top");
const trellinomeBottom = require("./trellinome-bottom");

const models = {
  "sparknome-top": sparknomeTop,
  "sparknome-bottom": sparknomeBottom,
  "trellinome-top": trellinomeTop,
  "trellinome-bottom": trellinomeBottom
};

const searchValue = document.location.search.replace("?", "");

if (!searchValue) {
  document.body.innerHTML = Object.keys(models)
    .map(
      key => `<div><a href="${document.location.href}?${key}">${key}</a></div>`
    )
    .join("\n");
} else {
  const { model, parts } = models[searchValue]();
  createPreview(model, parts);
}
