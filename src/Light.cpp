#include "Light.h" 

PointLight::PointLight(vec3 pos, colour3 c) {
	this->position = pos;
	this->colour = c; 
	setType(POINT_LIGHT);
}

DirectionalLight::DirectionalLight(vec3 dir, colour3 c) {
	this->direction = dir;
	this->colour = c;
	setType(DIRECTIONAL);
}

AmbientLight::AmbientLight(colour3 c) {
	this->colour = c;
	setType(AMBIENT);
}

SpotLight::SpotLight(vec3 pos, vec3 dir, float cutoff, colour3 col) {
	this->position = pos;
	this->direction = dir;
	this->cutoff = cutoff;
	this->colour = col;
	setType(SPOT);
	}