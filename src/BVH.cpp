#include"BVH.h"

const bool DISABLE_SHADOW_TRANSPARENCY = false;

BVH::BVH() {
	this->root = NULL;
}

BVH::BVH(std::vector<Object*> objects) {
	this->objects = objects;
	this->root = NULL;
	this->isMesh = false;

	BoundingBox temp(vec3(0, 0, 0), vec3(0, 0, 0));
	for (auto& ob : objects) {
		this->worldBox.join(ob->bbox);
	}
	this->build();
}

BVH::BVH(std::vector<Triangle*> triangles) {
	this->triangles = triangles;
	this->root = NULL;
	this->isMesh = true;
	BoundingBox temp(vec3(0, 0, 0), vec3(0, 0, 0));
	for (Triangle* tri : triangles) {
		this->worldBox.join(tri->box);
	}
	this->build();
}


void BVH::setWorldBound(BoundingBox b) {
	this->worldBox = b;
}

void BVH::build() {
	if (!this->isMesh) {
		this->root = new Node();
		this->root->boundingBox = &this->worldBox;
		this->root->isRoot = true;

		recursive_build(this->root, 0, this->objects);
		printf("BVH Completed");
	}
	else {
		this->root = new Node();
		this->root->boundingBox = &this->worldBox;
		this->root->isRoot = true;

		recursive_mesh_build(this->root, 0, this->triangles);
		printf("meshBvh complete\n");
	}
}

void BVH::recursive_mesh_build(Node*& node, int depth, std::vector<Triangle*> tris) {
	if (tris.size() <= 1000 || depth > 25) {
		node->leaf = true;
		for (Triangle* t : tris) {
			node->leafObjects.push_back(t);
			node->boundingBox->join(t->box);
		}
		return;
	}
	else {
		std::vector<Triangle*> leftTriangles, rightTriangles;
		BoundingBox centreBox;
		for (int i = 0; i < tris.size(); i++) {
			centreBox.join(tris.at(i)->box);
		}
		int axis = node->boundingBox->axis();
		float midPoint = (centreBox.min[axis] + centreBox.max[axis]) / 2.0f;
		for (int i = 0; i < tris.size(); i++) {
			if (axis != 2) {
				if (tris.at(i)->box.centre[axis] <= midPoint) {
					leftTriangles.push_back(tris.at(i));
				}
				else {
					rightTriangles.push_back(tris.at(i));
				}
			}
			else {
				if (tris.at(i)->box.centre[axis] >= midPoint) {
					leftTriangles.push_back(tris.at(i));
				}
				else {
					rightTriangles.push_back(tris.at(i));
				}
			}


		}

		BoundingBox leftBox, rightBox;
		Node* left = new Node();
		Node* right = new Node();
		for (Triangle* t : leftTriangles) {
			leftBox.join(t->box);
		}
		for (Triangle* t : rightTriangles) {
			rightBox.join(t->box);
		}
		*left->boundingBox = leftBox;
		*right->boundingBox = rightBox;
		node->left = left;
		node->right = right;
		recursive_mesh_build(node->left, depth + 1, leftTriangles);
		recursive_mesh_build(node->right, depth + 1, rightTriangles);

	}
	return;
}

void BVH::recursive_build(Node*& node, int depth, std::vector<Object*> objects) {

	if (objects.size() <= 3 || depth > 25) {
		node->leaf = true;
		for (auto& ob : objects) {
			node->leafObjects.push_back(ob);
			node->boundingBox->join(ob->bbox);
		}
		return;
	}
	else {
		std::vector<Object*> leftObjects, rightObjects;
		BoundingBox centreBox;

		for (int i = 0; i < objects.size(); i++) {
			centreBox.join(objects.at(i)->bbox.calcCentre());

			//if (objects.at(i)->bbox.calcCentre().x < centre.x && objects.at(i)->bbox.calcCentre().y < centre.y && objects.at(i)->bbox.calcCentre().z < centre.z) { // object is left of centre 
			//	leftObjects.push_back(objects.at(i));
			//}
			//else { // right of centre
			//	rightObjects.push_back(objects.at(i));
			//}
		}
		int axis = node->boundingBox->axis();
		float midPoint = (centreBox.min[axis] + centreBox.max[axis]) / 2.0;
		for (int i = 0; i < objects.size(); i++) {
			if (axis != 2) {
				if (objects.at(i)->bbox.centre[axis] <= midPoint) {
					leftObjects.push_back(objects.at(i));
				}
				else {
					rightObjects.push_back(objects.at(i));
				}
			}
			else {
				if (objects.at(i)->bbox.centre[axis] >= midPoint) {
					leftObjects.push_back(objects.at(i));
				}
				else {
					rightObjects.push_back(objects.at(i));
				}
			}
		}
		BoundingBox leftBox, rightBox;
		Node* left = new Node();
		Node* right = new Node();

		for (auto& ob : leftObjects) {
			leftBox.join(ob->bbox);
		}
		for (auto& ob : rightObjects) {
			rightBox.join(ob->bbox);
		}
		*left->boundingBox = leftBox;
		*right->boundingBox = rightBox;


		node->left = left;
		node->right = right;
		recursive_build(node->left, depth + 1, leftObjects);

		recursive_build(node->right, depth + 1, rightObjects);


	}
	return;
}

float BVH::traverse(Ray& ray, Node* node, vec3 d, float nearT, float farT, vec3& hit_at, vec3& hit_normal, Object*& hit_object, vec3* opacity_sum, bool pick, std::string prefix)
{
	float t, nearest_t = -1;
	vec3 at;
	vec3 normal;
	std::stack<Node*> theStack;
	theStack.push(this->root);
	while (!theStack.empty()) {
		Node* currNode = theStack.top(); theStack.pop();
		if (currNode->boundingBox->intersect(ray)) {
			if (currNode->leaf) {
				for (auto& object : currNode->leafObjects) {
					if (object->bbox.intersect(ray)) {
						switch (object->getType()) {
							Sphere* sphere;
							Plane* plane;
							Mesh* mesh;
						
							Partical* partical;
						case SPHERE:
							sphere = (Sphere*)object;
							t = sphere->intersect(&ray, sphere, nearT, farT, at, normal, pick, prefix);
							break;
						case PLANE:
							plane = (Plane*)object;
							t = plane->intersect(&ray, plane, nearT, farT, at, normal, pick, prefix);
							break;
						case MESH:
							mesh = (Mesh*)object;
							t = mesh->meshBVH->traverseMeshBVH(ray, mesh->meshBVH, nearT, farT, at, normal, pick, prefix);
							break;
						
						case PARTICAL:
							partical = (Partical*)object;
							bool aHit = false;
							int j = 0;
							for (int i = 0; i < partical->particals.size(); i++) {
								Sphere* s = partical->particals.at(i);
								if (s->bbox.intersect(ray)) {
									t = s->intersect(&ray, s, nearT, farT, at, normal, pick, prefix);
									if (t > 0 && (nearest_t < 0 || t < nearest_t)) {
										aHit = true;
										nearest_t = t;
										j = i;
										if (NULL == opacity_sum) {
											// if we're calculating shadow transparency, we can't skip anything
											farT = t;
										}
										hit_at = at;
										hit_normal = normal;
										hit_object = s;
									}
								}


							}
							if (!aHit) {
								t = -1;
							}
							else {
								partical->hitIndex = j;
							}


						}
						if (opacity_sum != NULL) {
							if (t >= nearT && (farT < nearT || t <= farT)) {
								if (DISABLE_SHADOW_TRANSPARENCY) {
									*opacity_sum = vec3(1, 1, 1);
									hit_object = object;
									return 1;
								}
								else {
									vec3 opacity = vec3(1, 1, 1) - object->material.transmissive;
									*opacity_sum += opacity;
									*opacity_sum = glm::clamp(*opacity_sum, 0.0f, 1.0f);
									if (pick) std::cout << prefix << "shadow adding opacity " << glm::to_string(opacity) << " to get sum " << glm::to_string(*opacity_sum) << std::endl;
									if (*opacity_sum == vec3(1, 1, 1)) {
										return 1;
									}
								}
							}
						}
						if (t > 0 && (nearest_t < 0 || t < nearest_t)) {
							nearest_t = t;
							if (NULL == opacity_sum) {
								// if we're calculating shadow transparency, we can't skip anything
								farT = t;
							}
							hit_at = at;
							hit_normal = normal;
							if (object->getType() != PARTICAL) {
								hit_object = object;
							}
							else {
								Partical* partical = (Partical*)object;
								hit_object = partical->particals.at(partical->hitIndex);
							}
						}
					}
				}
			}
			else {
				theStack.push(currNode->right);
				theStack.push(currNode->left);
			}
		}
	}
	return nearest_t;

}

float BVH::traverseMeshBVH(Ray& ray, BVH* meshBVH, float nearT, float farT, vec3& hp, vec3& norm, bool pick, std::string prefix) {
	float t, nearest_t = -1;
	vec3 at;
	vec3 normal;
	int hit_i = -1;
	std::stack<Node*> theStack;
	theStack.push(meshBVH->root);
	while (!theStack.empty()) {
		Node* currNode = theStack.top(); theStack.pop();
		if (currNode->boundingBox->intersect(ray)) {
			if (currNode->leaf) {
				int i = 0;
				for (auto& triangle : currNode->leafObjects) {
					Triangle* tri = (Triangle*)triangle;
					if (tri->box.intersect(ray)) {
						i++;
						t = tri->intersect(&ray, *tri, nearT, farT, at, normal, pick, prefix);
						if (t >= nearT && (nearest_t < 0 || t < nearest_t)) {
							nearest_t = t;
							farT = t;
							hp = at;
							norm = normal;
							hit_i = i;
						}
						if (pick && hit_i >= 0) { std::cout << prefix << "mesh chose triangle #" << hit_i << std::endl; }

					}

				}
				if (nearest_t != -1) {
					return nearest_t;
				}

			}

			else {
				theStack.push(currNode->left);
				theStack.push(currNode->right);
			}
		}
	}
	return nearest_t;
}

