/*
* Bounding Box
* 
* Axis alligned boungind box 
* 
* 
*/

#include "BoundingBox.h"

BoundingBox::BoundingBox() {
	this->min = vec3(10000000,1000000,100000);
	this->max = vec3(-10000000,-1000000,-1000000);
}

BoundingBox::BoundingBox(vec3 min, vec3 max) {
	this->min = min;
	this->max = max; 
	this->centre = this->min + 0.5f * (this->max - this->min);
}

vec3 BoundingBox::calcCentre() { return this->min + 0.5f * (this->max - this->min); }

bool BoundingBox::inside(point3 p) {
	return (p.x >= this->min.x && p.x <= this->max.x) && (p.y >= this->min.y && p.y <= this->max.y) && (p.z >= this->min.z && p.z <= this->max.z);
}

void BoundingBox::join(BoundingBox box) {


	this->min = glm::min(this->min, box.min);
	this->max = glm::max(this->max, box.max);
	
}

void BoundingBox::join(point3 p) {
	

	this->min= glm::min(this->min, p);
	this->max = glm::max(this->max, p);

}

int BoundingBox::axis() {
	switch (this->currAxis) {
	case -1:
		this->currAxis = 0;
		return this->currAxis;
	case 0:
		this->currAxis = 1;
		return this->currAxis;
	case 1:
		this->currAxis = 2;
		return this->currAxis;
	case 2:
		this->currAxis = 0;
		return this->currAxis;
	}
}

double BoundingBox::split(int a) {
	switch (a) {
	case 0:
		return (this->min.x + this->max.x) / 2.0;
	case 1:
		return (this->min.y + this->max.y) / 2.0;
	case 2:
		return (this->min.z + this->max.z) / 2.0;
	default:
		return -1;
	}
}

bool BoundingBox::intersect(Ray& ray) {

	float tmin = (this->min.x - ray.origin.x) * ray.invertDir.x;
	float tmax = (this->max.x - ray.origin.x) * ray.invertDir.x; 

	if (tmin > tmax) { std::swap(tmin, tmax); }
	
	float ymin = (this->min.y - ray.origin.y) * ray.invertDir.y; 
	float ymax = (this->max.y - ray.origin.y) * ray.invertDir.y;

	if (ymin > ymax) { std::swap(ymin, ymax); }

	if ((tmin > ymax) || (ymin > tmax)) { return false;  }
	if (ymin > tmin) {tmin = ymin;} if (ymax < tmax) {tmax = ymax; }

	float zmin = (this->min.z - ray.origin.z) * ray.invertDir.z;
	float zmax = (this->max.z - ray.origin.z) * ray.invertDir.z;

	if (zmin > zmax) { std::swap(zmin, zmax);  }

	if ((tmin > zmax) || zmin > tmax) {return false;}

	if (zmin > tmin) {tmin = zmin;} if (zmax < tmax) { tmax = zmax; }
	return true; 
}