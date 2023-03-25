#pragma once

#include "Object.h"
class Node {

public:

	Node();

	BoundingBox *boundingBox; 
	bool leaf;
	bool isRoot;
	Node *left;
	Node *right;

	std::vector<Object*> leafObjects;
 
	bool intersect(Ray* ray);
};