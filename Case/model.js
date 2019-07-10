const React = require("react");
const { Model } = require("modeler-csg");

const range = n => Array.from({ length: n }).map((_, i) => i);

const USE_ENV =
  process.env.RENDER_TOP || process.env.RENDER_BOTTOM || process.env.SIZE;

const RENDER_TOP = USE_ENV ? process.env.RENDER_TOP === "true" : true;
const RENDER_BOTTOM = USE_ENV ? process.env.RENDER_BOTTOM === "true" : false;
const SIZE = USE_ENV
  ? (process.env.SIZE || "8x8").split("x").map(n => parseInt(n))
  : [16, 8];

// in CM
const PLATE_MARGIN = 0.7;
const PLATE_HEIGHT = 0.3;
const MONOME_SIZE = [SIZE[0] * (1 + 0.5) - 0.5, SIZE[1] * (1 + 0.5) - 0.5];

const Plate = () => (
  <cube
    radius={[
      MONOME_SIZE[0] / 2 + PLATE_MARGIN,
      PLATE_HEIGHT / 2,
      MONOME_SIZE[1] / 2 + PLATE_MARGIN
    ]}
    position={[0, , 0]}
    center={[0, PLATE_HEIGHT / 2]}
  />
);

const PlateScrewes = () => {
  const r = 0.4;
  const offset = PLATE_MARGIN / 2;

  return (
    <>
      {[
        [-offset, -offset],
        [-offset, MONOME_SIZE[1] + offset],
        [MONOME_SIZE[0] + offset, -offset],
        [MONOME_SIZE[0] + offset, MONOME_SIZE[1] + offset]
      ].map(([ox, oy]) => {
        const x = -MONOME_SIZE[0] / 2 + ox;
        const y = -MONOME_SIZE[1] / 2 + oy;

        return <cylinder radius={r / 2} start={[x, 0, y]} end={[x, 1, y]} />;
      })}
    </>
  );
};

const MonomeButtons = () => (
  <>
    {range(SIZE[0]).map(i =>
      range(SIZE[1]).map(j => {
        const size = 1;
        const x = -MONOME_SIZE[0] / 2 + size / 2 + i * (size + 0.5);
        const y = -MONOME_SIZE[1] / 2 + size / 2 + j * (size + 0.5);

        return (
          <cube key={`${i}-${j}`} radius={size / 2} center={[x, size / 2, y]} />
        );
      })
    )}
  </>
);

const MonomeScrewes = () => (
  <>
    {range(SIZE[0] / 2).map(i =>
      range(SIZE[1] / 2).map(j => {
        const r = 0.2;
        const x = -MONOME_SIZE[0] / 2 + 1.25 + i * 3;
        const y = -MONOME_SIZE[1] / 2 + 1.25 + j * 3;

        return <cylinder radius={r / 2} start={[x, 0, y]} end={[x, 1, y]} />;
      })
    )}
  </>
);

module.exports = () => (
  <Model showParts={false}>
    <subtract>
      <Plate />

      {RENDER_TOP && <MonomeButtons />}
      {RENDER_TOP && <MonomeScrewes />}

      {(RENDER_TOP || RENDER_BOTTOM) && <PlateScrewes />}
    </subtract>
  </Model>
);
