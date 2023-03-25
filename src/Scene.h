#pragma once

#include<vector>

#include"Camera.h"
#include "Light.h"
#include "BVH.h"


class Scene {

public:

	std::vector<Object*> objects;
	std::vector<Light*> lights;
	Camera* camera;
	BoundingBox worldBox;
	Scene(std::vector<Object*> objects, std::vector<Light*> lights, Camera* c);
	Scene();
	BVH* theBVH;
	
};