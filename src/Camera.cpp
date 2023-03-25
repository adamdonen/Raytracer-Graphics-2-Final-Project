#include "Camera.h"

const double PI = 3.1415926535897932384626433832795;

Camera::Camera(vec3 pos, vec3 upVec, vec3 lookAtVec, float fov, float aspectRatio, float aperture, float focalDist, colour background)
{
	float theta = fov * PI / 180.0f;
	float h = tan(theta / 2);
	float vpH = 2 * h;
	float vpW = aspectRatio * vpH;

	vec3 w = glm::normalize(pos - lookAtVec);
	vec3 u = glm::normalize(glm::cross(upVec, w));
	vec3 v = glm::cross(w, u);

	this->position = pos;
	this->up = vpH * v;
	this->right = vpW * u;
	this->bottomLeft = pos - (this->right/= 2.0) - (this->up/= 2) - w;
	this->background = background;

	

}

Camera::Camera(float fov, colour background) {
	this->fov = fov;
	this->background = background;

}
Camera::Camera() {
}


Ray* Camera::generateRay(float x, float y) {
	return new Ray(this->position, glm::normalize(this->bottomLeft + x*this->right + y*this->up - this->position));
}






