#pragma once
#include <glm/glm.hpp>



typedef glm::vec3 vec3;

class Ray {

public:
	vec3 origin; 
	vec3 direction; 
	float t;
	vec3 invertDir; 
	Ray(vec3 origin, vec3 direction);

	float getT();
	
};