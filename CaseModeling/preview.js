const {
  LineBasicMaterial,
  DirectionalLight,
  AmbientLight,
  MeshLambertMaterial,
  Mesh,
  LineSegments,
  Scene,
  Geometry,
  WireframeGeometry,
  PerspectiveCamera,
  Vector3,
  Face3,
  WebGLRenderer
} = require("THREE");
const OrbitControls = require("three-orbitcontrols");

module.exports = model => {
  const polygons = model.toPolygons();
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
  const material = new MeshLambertMaterial();
  const mesh = new Mesh(geometry, material);
  scene.add(mesh);

  // wireframe
  const wireframe = new WireframeGeometry(geometry);
  const line = new LineSegments(
    wireframe,
    new LineBasicMaterial({ color: 0xffffff })
  );
  scene.add(line);

  const renderer = new WebGLRenderer({ antialias: true });
  renderer.setSize(window.innerWidth, window.innerHeight);
  document.body.appendChild(renderer.domElement);

  const controls = new OrbitControls(camera, renderer.domElement);
  controls.enableDamping = true;
  controls.dampingFactor = 0.25;
  controls.enableZoom = true;

  document.body.style.margin = 0;

  function loop() {
    requestAnimationFrame(loop);
    renderer.render(scene, camera);
  }

  loop();
};
