

#include "Ray.h"

Ray::Ray(vec3 origin, vec3 d) {

	this->origin = origin;
	this->direction = d;
	this->t = -1000000.0f;
	this->invertDir = vec3(1.0f / this->direction.x, 1.0f / this->direction.y, 1.0f / this->direction.z);
}

float Ray::getT() {
	return this->t;
}

