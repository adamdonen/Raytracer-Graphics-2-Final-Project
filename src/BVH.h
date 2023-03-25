#pragma once
#include "Node.h"
#include <algorithm>
#include <vector>
#include <stack>

class BVH {
public:
	BVH();
	BVH(std::vector<Object*> objects);
	BVH(std::vector<Triangle*> triangles);

	Node* root; 
	BoundingBox worldBox;
	bool isMesh;


	void setWorldBound(BoundingBox b);
	void build();
	void recursive_build(Node* &node, int depth, std::vector<Object*> obs);
	void recursive_mesh_build(Node*& node, int depth, std::vector<Triangle*> tris);
	float traverse(Ray& ray, Node *node, vec3 d, float nearT, float farT, vec3 &hit_at, vec3 &hit_normal, Object* &hit_object, vec3* opacity_sum, bool pick, std::string prefix );
	float traverseMeshBVH(Ray& ray, BVH* meshBVH, float nearT, float farT, vec3& hp, vec3& norm, bool pick, std::string prefix);
protected:
	
	std::vector<Object*> objects;
	std::vector<Triangle*> triangles;

};