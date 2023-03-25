#pragma once

#include <glm/glm.hpp>
#include "Ray.h"
#include <utility>



typedef glm::vec3 vec3;
typedef glm::vec3 point3;



class BoundingBox {
public:

	BoundingBox();
	BoundingBox(vec3 min, vec3 max);

	vec3 min; vec3 max; vec3 centre;
	int currAxis = -1;

	vec3 calcCentre();
	bool inside(point3 p);
	void join(BoundingBox b);
	void join(point3 p);
	int axis();
	double split(int a);

	bool intersect(Ray& ray);
};