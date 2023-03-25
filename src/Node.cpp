// Node class for BVH 

#include "Node.h"
#include <algorithm>



Node::Node() {
	this->leaf = false;
	this->isRoot= false;
	this->boundingBox = new BoundingBox();
	this->left = NULL;
	this->right = NULL;
	
}

// Ray AABB intersection 
bool Node::intersect(Ray* ray) {
	float tmin, tmax, xmin, xmax, ymin, ymax, zmin, zmax;

	xmin = (this->boundingBox->min.x - ray->origin.x) * ray->invertDir.x;
	xmax = (this->boundingBox->max.x - ray->origin.x) * ray->invertDir.x;

	ymin = (this->boundingBox->min.y - ray->origin.y) * ray->invertDir.y;
	ymax = (this->boundingBox->max.y - ray->origin.y) - ray->invertDir.y;

	zmin = (this->boundingBox->min.z - ray->origin.z) * ray->invertDir.z;
	zmax = (this->boundingBox->max.z - ray->origin.z) * ray->invertDir.z;

	tmin = std::min(xmin, xmax);
	tmax = std::max(xmin, xmax);

	tmin = std::max(tmin, std::min(ymin, ymax));
	tmax = std::min(tmax, std::max(ymin, ymax));

	tmin = std::max(tmin, std::min(zmin, zmax));
	tmax = std::min(tmax, std::max(zmin, zmax));

	if (tmin > ray->t) { return false; }

	return tmax >= tmin && tmax >= 0;
}