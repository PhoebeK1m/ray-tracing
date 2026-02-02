#include <cmath>
#include <iostream>

#include "light.h"
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>

using namespace std;

double DirectionalLight::distanceAttenuation(const glm::dvec3 &) const {
  // distance to light is infinite, so f(di) goes to 0.  Return 1.
  return 1.0;
}

glm::dvec3 DirectionalLight::shadowAttenuation(const ray &r,
                                               const glm::dvec3 &p) const {
  // YOUR CODE HERE:
  // You should implement shadow-handling code here.
  glm::dvec3 lightDir = getDirection(p);
  ray shadowRay(p+ RAY_EPSILON * lightDir, lightDir, glm::dvec3(1,1,1), ray::SHADOW);

  isect i;
  if (scene->intersect(shadowRay, i)) {
    // L->obj->Cam/View then shadow
    return glm::dvec3(0.0, 0.0, 0.0);
  }
  return glm::dvec3(1.0, 1.0, 1.0);
}

glm::dvec3 DirectionalLight::getColor() const { return color; }

glm::dvec3 DirectionalLight::getDirection(const glm::dvec3 &) const {
  return -orientation;
}

double PointLight::distanceAttenuation(const glm::dvec3 &P) const {
  // YOUR CODE HERE

  // You'll need to modify this method to attenuate the intensity
  // of the light based on the distance between the source and the
  // point P.  For now, we assume no attenuation and just return 1.0

  double dist = glm::length(position - P);

  // f(d) = min( 1, 1/( a + b d + c d^2 ) )
  double denom = 1.0 / (constantTerm + (linearTerm * dist) + (quadraticTerm * dist *dist));

  if (denom > 0) {
    double distAtten = 1.0/denom;
    return std::min(1.0, distAtten);
  }
  return 1.0;
}

glm::dvec3 PointLight::getColor() const { return color; }

glm::dvec3 PointLight::getDirection(const glm::dvec3 &P) const {
  return glm::normalize(position - P);
}

glm::dvec3 PointLight::shadowAttenuation(const ray &r,
                                        const glm::dvec3 &p) const {
  // YOUR CODE HERE:
  // You should implement shadow-handling code here.
  glm::dvec3 lightDistVec = position - p; // need magnitude or actual distance
  double lightDistMag = glm::length(lightDistVec);
  glm::dvec3 lightDir = getDirection(p);

  ray shadowRay(p + RAY_EPSILON * lightDir, lightDir, glm::dvec3(1,1,1), ray::SHADOW);

  isect i;
  if (scene->intersect(shadowRay, i)) {

    if (i.getT() < lightDistMag) {
      return glm::dvec3(0.0, 0.0, 0.0);
    }
  }
  return glm::dvec3(1.0, 1.0, 1.0);
}

#define VERBOSE 0

