#pragma once
#include <glm/glm.hpp>

typedef glm::vec3 vec3;
typedef glm::vec3 colour3;

enum LIGHTS { POINT_LIGHT, DIRECTIONAL, SPOT, AMBIENT };


 

class Light {
public: 
	LIGHTS type;
	colour3 colour; 

	void setType(LIGHTS l) {
		this->type = l;
	}
	LIGHTS getType() {
		return this->type;
	}

};

class PointLight : public Light {
public:
	vec3 position; 
	PointLight(vec3 pos, colour3 col);

};

class DirectionalLight : public Light {

public: 
	vec3 direction;
	DirectionalLight(vec3 dir, colour3 col);
};

class AmbientLight : public Light {

public:
	AmbientLight(colour3 col);
};

class SpotLight : public Light {
public:
	vec3 position;
	vec3 direction;
	float cutoff; 

	SpotLight(vec3 pos, vec3 dir, float cutoff, colour3 col); 
};