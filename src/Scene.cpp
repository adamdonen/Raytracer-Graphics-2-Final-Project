#include "Scene.h"


Scene::Scene() {

}

Scene::Scene(std::vector<Object*> objects, std::vector<Light*> lights, Camera* c) {
	this->camera = c;
	this->objects = objects;
	this->lights = lights; 

	//getting world boundary box
	for (auto& ob : objects) {
		this->worldBox.join(ob->bbox);
	}
	this->theBVH = new BVH(this->objects);

}
