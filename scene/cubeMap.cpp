#include "cubeMap.h"
#include "../scene/material.h"
#include "../ui/TraceUI.h"
#include "ray.h"
extern TraceUI *traceUI;

glm::dvec3 CubeMap::getColor(ray r) const {
  // YOUR CODE HERE
  // FIXME: Implement Cube Map here
  
  // get direction of ray and figure out which face of cube
  glm::dvec3 dir = glm::normalized(r.getDirection());
  double x = dir.x;
  bool xIsPos = x > 0 ? true : false;
  double y = dir.y;
  bool yIsPos = y > 0 ? true : false;
  double z = dir.z;
  bool zIsPos = z > 0 ? true : false;

  // gotta choose greatest value for the face : )
  // can choose order of priority
  double posx = fabs(x); // 1
  double posy = fabs(y); // 2
  double posz = fabs(z); // 3

  int tMapNum = 0;
  double u = 0;
  double v = 0;

  // check texture coordinates: cube image from presentation
  // didnt really understand how to find coordinates so watched this video
  // https://youtu.be/oYmfPZE4THY?si=n9Wk0FAqqug_Mi3I
  if (posx >= posy && posx >= posz) {
    // X
    if (xIsPos) {
      // + X -> u = -z; v = y;
      tMapNum = 0;
      u = -z / posx;
      v = y /posx;
    } else {
      // -X -> u = z; v = y;
      tMapNum = 1;
      u = z / posx;
      v = y / posx;
    }
  } else if (posy >= posx && posy >= posz) {
    // Y
    if (yIsPos) {
      // +Y -> u = x; v = -z
      tMapNum = 2;
      u = x / posy;
      v = -z /posy;
    } else {
      // -Y -> u = x; v = z;
      tMapNum = 3;
      u = x / posy;
      v = z / posy;
    }
  } else {
    // Z
    if (zIsPos) {
      // +Z -> u = x; v = y;
      tMapNum = 4;
      u = x / posz;
      v = y /posz;
    } else {
      // -Z -> u = -x; v = y;
      tMapNum = 5;
      u = -x / posz;
      v = y / posz;
    }
  }

  // converting to [0, 1] range since u, v are <= 1
  u = (u + 1) / 2;
  v = (u + 1) / 2;

  // Return the mapped value; here the coordinate is assumed to be within
  // the parametrization space:
  // [0, 1] x [0, 1]
  // (i.e., {(u, v): 0 <= u <= 1 and 0 <= v <= 1}
  // glm::dvec3 getMappedValue(const glm::dvec2 &coord) co
  return tMap[tMapNum]->getMappedValue(glm::dvec2(u , v));
}

CubeMap::CubeMap() {}

CubeMap::~CubeMap() {}

void CubeMap::setNthMap(int n, TextureMap *m) {
  if (m != tMap[n].get())
    tMap[n].reset(m);
}
