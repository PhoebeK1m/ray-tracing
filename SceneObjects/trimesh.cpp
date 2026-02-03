#include "trimesh.h"
#include <algorithm>
#include <assert.h>
#include <cmath>
#include <float.h>
#include <string.h>
#include "../ui/TraceUI.h"
extern TraceUI *traceUI;
extern TraceUI *traceUI;

using namespace std;

Trimesh::~Trimesh() {
  for (auto f : faces)
    delete f;
}

// must add vertices, normals, and materials IN ORDER
void Trimesh::addVertex(const glm::dvec3 &v) { vertices.emplace_back(v); }

void Trimesh::addNormal(const glm::dvec3 &n) { normals.emplace_back(n); }

void Trimesh::addColor(const glm::dvec3 &c) { vertColors.emplace_back(c); }

void Trimesh::addUV(const glm::dvec2 &uv) { uvCoords.emplace_back(uv); }

// Returns false if the vertices a,b,c don't all exist
bool Trimesh::addFace(int a, int b, int c) {
  int vcnt = vertices.size();

  if (a >= vcnt || b >= vcnt || c >= vcnt)
    return false;

  TrimeshFace *newFace = new TrimeshFace(this, a, b, c);
  if (!newFace->degen)
    faces.push_back(newFace);
  else
    delete newFace;

  // Don't add faces to the scene's object list so we can cull by bounding
  // box
  return true;
}

// Check to make sure that if we have per-vertex materials or normals
// they are the right number.
const char *Trimesh::doubleCheck() {
  if (!vertColors.empty() && vertColors.size() != vertices.size())
    return "Bad Trimesh: Wrong number of vertex colors.";
  if (!uvCoords.empty() && uvCoords.size() != vertices.size())
    return "Bad Trimesh: Wrong number of UV coordinates.";
  if (!normals.empty() && normals.size() != vertices.size())
    return "Bad Trimesh: Wrong number of normals.";

  return 0;
}

bool Trimesh::intersectLocal(ray &r, isect &i) const {
  bool have_one = false;
  for (auto face : faces) {
    isect cur;
    if (face->intersectLocal(r, cur)) {
      if (!have_one || (cur.getT() < i.getT())) {
        i = cur;
        have_one = true;
      }
    }
  }
  if (!have_one)
    i.setT(1000.0);
  return have_one;
}

bool TrimeshFace::intersect(ray &r, isect &i) const {
  return intersectLocal(r, i);
}


// Intersect ray r with the triangle abc.  If it hits returns true,
// and put the parameter in t and the barycentric coordinates of the
// intersection in u (alpha) and v (beta).
bool TrimeshFace::intersectLocal(ray &r, isect &i) const {
  // YOUR CODE HERE
  //
  // FIXME: Add ray-trimesh intersection

  /* To determine the color of an intersection, use the following rules:
     - If the parent mesh has non-empty `uvCoords`, barycentrically interpolate
       the UV coordinates of the three vertices of the face, then assign it to
       the intersection using i.setUVCoordinates().
     - Otherwise, if the parent mesh has non-empty `vertexColors`,
       barycentrically interpolate the colors from the three vertices of the
       face. Create a new material by copying the parent's material, set the
       diffuse color of this material to the interpolated color, and then 
       assign this material to the intersection.
     - If neither is true, assign the parent's material to the intersection.
  */

  // point and direction-> need to find t
  // Q = P + dt
  glm::dvec3 p = r.getPosition();
  glm::dvec3 d = r.getDirection();

  // triangle vertices
  glm::dvec3 a = parent->vertices[ids[0]];
  glm::dvec3 b = parent->vertices[ids[1]];
  glm::dvec3 c = parent->vertices[ids[2]];

  // triangel norm
  glm::dvec3 norm = glm::cross(b - a, c - a);

  // given ray could be parallel -> dot product with norm is 0
  // N*D -> also shouldn't divide by zero
  double normdir = glm::dot(norm, d);
  if (fabs(normdir) < RAY_EPSILON){
    return false;
  }

  // t = -N*P+d / N*D
  // d = -N*a (a is any known point on the plane)
  // t = -(N*P-N*a) / N*D = N*(a-P)/N*D
  double t = glm::dot(norm, a - p) / normdir;

  // check that point is in front of camera
  if (t < RAY_EPSILON) {
    return false;
  }
  glm::dvec3 q = p + (d * t); // intersection point on the plane containing tirangle

  // barycentric coords
  double total_area = 0.5 * glm::length(norm);
  if (total_area < RAY_EPSILON) { // can't divide by 0 also that means the triangle is a line
    return false;
  }

  // labeling these off the lecture slide image
  double alpha = (0.5 * glm::length(glm::cross(c - b, q - b))) / total_area;
  double beta = (0.5 * glm::length(glm::cross(a - c, q - c))) / total_area;
  double gamma = (0.5 * glm::length(glm::cross(b - a, q - a))) / total_area;

  // check if q is in triangle
  if (alpha < -RAY_EPSILON || beta < -RAY_EPSILON  || gamma < -RAY_EPSILON ) {
    return false;
  }
  if (fabs(alpha + beta + gamma - 1.0) > RAY_EPSILON) {
    return false;
  }

  i.setT(t);
  i.setN(glm::normalize(norm));
  i.setObject(this->parent);

  // barycentrically interpolating

  // case 1: nonempty uv-coords
  if (!parent->uvCoords.empty()) {
    glm::dvec2 uv = (alpha * parent->uvCoords[ids[0]]) + (beta * parent->uvCoords[ids[1]]) 
                    + gamma * parent->uvCoords[ids[2]];
    i.setUVCoordinates(uv);
    i.setMaterial(parent->material);
  }
  // case 2: nonempty vertex colors
  else if (!parent->vertColors.empty()) {
    glm::dvec3 color = (alpha * parent->vertColors[ids[0]]) + (beta * parent->vertColors[ids[1]]) 
                    + gamma * parent->vertColors[ids[2]];
    // copy parent's material, set diffuse color of this material to the interpolated color
    Material m = parent->material;
    m.setDiffuse(color);
    i.setMaterial(m);
  }
  // case 3: neither
  else {
    i.setMaterial(parent->material);
  }

  return true;
}

// Once all the verts and faces are loaded, per vertex normals can be
// generated by averaging the normals of the neighboring faces.
void Trimesh::generateNormals() {
  int cnt = vertices.size();
  normals.resize(cnt);
  std::vector<int> numFaces(cnt, 0);

  for (auto face : faces) {
    glm::dvec3 faceNormal = face->getNormal();

    for (int i = 0; i < 3; ++i) {
      normals[(*face)[i]] += faceNormal;
      ++numFaces[(*face)[i]];
    }
  }

  for (int i = 0; i < cnt; ++i) {
    if (numFaces[i])
      normals[i] /= numFaces[i];
  }

  vertNorms = true;
}

