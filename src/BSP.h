#pragma once
#include "Object.h"

class BSP {
private:
	struct BSPNode {
		BSPNode* left;
		BSPNode* right;
		std::vector<Object*> objects;
	};

	BSPNode* root;

	int frags;

public:
	int depth;
	int axisToggles;
	int numNodes;

	int axis;
	BoundingBox box;

	std::vector<Object*> objects;
	BSP* left;
	BSP* right;

	void build();
	void recursiveBuild(std::vector<Object*>objects, BSPNode *node);
	int toggleAxis();
	void getHyperPlane(Object ob, glm::vec4& plane);
	BSP();
	BSP(int depth, int axis, std::vector<Object*> objects);
};