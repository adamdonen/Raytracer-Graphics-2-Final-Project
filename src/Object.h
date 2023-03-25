#pragma once

//#include "Ray.h"
#include "BoundingBox.h"

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include<algorithm>
#include <string>
#include <glm/glm.hpp>
#include<random>
#include <glm/gtx/string_cast.hpp>

class BVH;
typedef glm::vec3 vec3;

enum OBJECT_TYPE{SPHERE, PLANE, MESH, TRIANGLE, CYLINDER, PARTICAL };




class Material {
public:
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess; 
	vec3 reflective; 
	vec3 transmissive; 
	float refraction; 

	Material(vec3 a, vec3 d, vec3 spec, float shin, vec3 reflec, vec3 tran, float refract);
	Material() {
		this->ambient = vec3(0, 0, 0);
		this->diffuse = vec3(0, 0, 0);
		this->specular = vec3(0, 0, 0);
		this->shininess = -1.0f;
		this->reflective = vec3(0, 0, 0);
		this->transmissive = vec3(0, 0, 0);
		this->refraction = 1.0;
	}
};

class Object{

public:
	OBJECT_TYPE type; 
	Material material; 
	
	BoundingBox bbox; 
	int id;
	
	Object(OBJECT_TYPE type, Material m);
	Object();
	void setType(OBJECT_TYPE t);
	OBJECT_TYPE getType();
	BoundingBox getBounds();
	float intersect(Ray* ray, Object* obj, float nearT, float farT, vec3& hp, vec3& hp_norm, bool pick, std::string prefix);
};

class Sphere : public Object {

public:
	vec3 position; 
	float radius; 
	
	Sphere(vec3 pos, float r, Material m);
	
	vec3 getNormal(vec3 p);
	float intersect(Ray* ray, Sphere* obj, float nearT, float farT, vec3 &hp, vec3 &hp_norm, bool pick, std::string prefix);

	BoundingBox getBounds();
	void setMaterial(Material m);

};

class Plane : public Object {
public: 
	vec3 position;
	vec3 normal; 
	float size; 
	Plane(vec3 pos, vec3 nomral, Material m);
	float intersect(Ray* ray, Plane* obj, float nearT, float farT, vec3& hp, vec3& hp_norm, bool pick, std::string prefix);
	BoundingBox getBounds(); 

	bool intersectBoundingBox(Plane* p, BoundingBox box);
	Plane();
};

class Triangle : public Object {

public:
	Triangle(vec3 a, vec3 b, vec3 c);
	Triangle();
	vec3 a, b, c;
	BoundingBox box;

	bool intersectBoundingBox(Triangle tri, BoundingBox box);
	float intersect(Ray* ray, Triangle tri, float nearT, float farT, vec3& hp, vec3& hp_norm, bool pick, std::string prefix);
	bool testAxis(float p0, float p1, float p2, vec3 axis, float r);
	BoundingBox calcBounds();
};

class Mesh : public Triangle {
public:
	Mesh(std::vector<Triangle*> tris, Material m); 

	BoundingBox box;
	std::vector<Triangle*> triangles; 

	float intersect(Ray* ray, Mesh* obj, float, float, vec3&, vec3&, bool, std::string);

	BoundingBox getBounds();
	BVH *meshBVH;
};



class Partical : public Object {
public:
	std::vector<Sphere*> particals;
	int hitIndex;

	Partical();
	void generateParticals();
	float getCord(float minLimit, float maxLimit, bool isX);
	
	Material getRandomMaterial();

};

