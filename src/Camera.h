#pragma once

#include "Ray.h"

typedef glm::vec3 colour; 
typedef glm::vec3 vec3; 

class Camera {

public:

	float swidth, sheight; 
	float fov;
	colour background;
	float aspectRatio;
	vec3 position;

	Camera();
	Camera(vec3 pos, vec3 upVec, vec3 lookAtVec, float fov, float aspectRatio,float aperture, float focalDist, colour);
	Camera(float fov, colour backgroud);

	Ray* generateRay(float x, float y);


private: 


	vec3 lookAt;

	vec3 up;
	vec3 right;
	vec3 bottomLeft;

	vec3 u, v, w;

	float lensR;

};
