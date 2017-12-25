const {
  AmbientLight,
  DirectionalLight,
  Face3,
  Geometry,
  GridHelper,
  LineBasicMaterial,
  LineSegments,
  Mesh,
  MeshLambertMaterial,
  PerspectiveCamera,
  Scene,
  Vector3,
  WebGLRenderer,
  WireframeGeometry
} = require("THREE");
const OrbitControls = require("three-orbitcontrols");

const geometryFromPolygons = polygons => {
  const geometry = new Geometry();

  const getGeometryVertice = (geometry, v) => {
    geometry.vertices.push(new Vector3(v.x, v.y, v.z));
    return geometry.vertices.length - 1;
  };

  for (i = 0; i < polygons.length; i++) {
    let vertices = [];

    for (let j = 0; j < polygons[i].vertices.length; j++) {
      vertices.push(getGeometryVertice(geometry, polygons[i].vertices[j].pos));
    }

    if (vertices[0] === vertices[vertices.length - 1]) {
      vertices.pop();
    }

    for (let j = 2; j < vertices.length; j++) {
      face = new Face3(
        vertices[0],
        vertices[j - 1],
        vertices[j],
        new Vector3().copy(polygons[i].plane.normal)
      );

      geometry.faces.push(face);
    }
  }

  geometry.computeVertexNormals();
  geometry.computeBoundingBox();

  return geometry;
};

module.exports = (model, parts) => {
  // camera
  const camera = new PerspectiveCamera(
    70,
    window.innerWidth / window.innerHeight,
    0.01,
    1000
  );

  camera.position.x = 10;
  camera.position.y = 18;
  camera.position.z = 10;

  const scene = new Scene();

  // lights
  const light = new AmbientLight(0x404040);
  const directionalLight = new DirectionalLight(0xffffff, 0.5);
  scene.add(light);
  scene.add(directionalLight);

  // mesh
  if (model) {
    const modelPolygons = model.toPolygons();
    const modelGeometry = geometryFromPolygons(modelPolygons);

    const material = new MeshLambertMaterial();
    const mesh = new Mesh(modelGeometry, material);
    scene.add(mesh);
  }

  // wireframes
  if (parts) {
    const partsPolygons = parts.map(part => part.toPolygons());
    const partsGeometries = partsPolygons.map(geometryFromPolygons);

    partsGeometries.forEach(geometry => {
      const wireframe = new WireframeGeometry(geometry);
      const line = new LineSegments(
        wireframe,
        new LineBasicMaterial({ color: 0x333333 })
      );
      scene.add(line);
    });
  }

  // grid
  const gridMm = new GridHelper(100, 1000, 0xbbbbbb, 0xbbbbbb);
  const gridCm = new GridHelper(100, 100, 0x888888, 0x888888);

  scene.add(gridMm);
  scene.add(gridCm);

  // renderer
  const renderer = new WebGLRenderer({ antialias: true });
  renderer.setClearColor(0xffffff);
  renderer.setSize(window.innerWidth, window.innerHeight);
  document.body.appendChild(renderer.domElement);

  // orbit controls
  const controls = new OrbitControls(camera, renderer.domElement);
  controls.enableDamping = false;
  controls.enableZoom = true;

  // css
  document.body.style.margin = 0;

  function loop() {
    requestAnimationFrame(loop);
    renderer.render(scene, camera);
  }

  loop();
};
