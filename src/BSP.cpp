#include "BSP.h"
#include "BSP.h"

BSP::BSP() {
	this->axisToggles = 0;
	this->left = this->right = NULL;
	build();
}

BSP::BSP(int depth, int axis, std::vector<Object*> objects) {
	this->axis = axis;
	this->depth = depth;
	this->objects = objects; 
	this->left = this->right = NULL;
	this->axisToggles = 0; 
	this->build();
}

//void BSP::build() {
//	
//	for (int i = 0; i < this->objects.size(); i++) {
//		BoundingBox b = this->objects.at(i)->getBounds();
//		this->box.min = vec3(glm::min(this->box.min.x, b.min.x), glm::min(this->box.min.y, b.min.y), glm::min(this->box.min.z, b.min.z));
//		this->box.max = vec3(glm::max(this->box.max.x, b.max.x), glm::max(this->box.max.y, b.max.y), glm::max(this->box.max.z, b.max.z));
//	}
//
//
//	if (objects.size() <= 2) {
//		// leaf node
//		return;
//	}
//	double split = this->box.split(this->axis);
//
//	std::vector<Object*> leftObjects; 
//	std::vector<Object* > rightObjects; 
//
//	for (int i = 0; i < this->objects.size(); i++) {
//		auto& ob = objects.at(i);
//		BoundingBox b = ob->getBounds();
//
//		float min, max; 
//
//		switch (axis) {
//		case 0:
//			min = b.min.x;
//			max = b.max.x;
//			break;
//		case 1:
//			min = b.min.y;
//			max = b.max.y;
//			break;
//		case 2: 
//			min = b.min.z;
//			max = b.max.z;
//			break;
//		}
//		if (min < split) {
//			leftObjects.push_back(ob);
//		} if (max > split) {
//			rightObjects.push_back(ob);
//		}
//	}
//	int newAxis = this->toggleAxis();
//
//	if (leftObjects.size() != this->objects.size() && rightObjects.size() != this->objects.size()) {
//		// split geometry
//		this->left = new BSP(depth + 1, newAxis, leftObjects);
//		this->right = new BSP(depth + 1, newAxis, rightObjects);
//	}
//	else if (this->axisToggles < 2) {
//		this->axis = this->toggleAxis();
//		this->axisToggles++; 
//		this->build();
//	}
//	else {
//
//	}
//
//}

void BSP::build() {
	if (this->objects.size() == 0) {
		return;
	}
	this->root = new BSPNode();
	this->numNodes++;
	this->recursiveBuild(this->objects, root);
}

void BSP::recursiveBuild(std::vector<Object*> objects, BSPNode* node) {
	int index = rand() % objects.size(); 
	node->objects.push_back(objects.at(index));
	glm::vec4 temp; 


}

int BSP::toggleAxis() {
	switch (this->axis) {
	case 0:
		return 1;
	case 1:
		return 2;
	case 2: 
		return 0;
	default:
		return 0;
	}
}

void BSP::getHyperPlane(Object ob, glm::vec4& plane) {

}