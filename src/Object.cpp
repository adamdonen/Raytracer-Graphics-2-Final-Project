// Object class stores material and a bounding box.
// Sub classes are Sphere Plane, Mesh, Triangle, Partical
// Each sub class has ray intersection test

#include "Object.h"
#include "BVH.h"
constexpr auto NUM_PARTICALS  = 50;


const bool ALLOW_HIT_MESH_BACK = true;
typedef glm::vec3 Vertex;
typedef glm::vec3 Vector;

enum PROPERTIES{ AMBIENT, DIFFUSE, SPECULAR, SHININESS, REFLECTIVE, TRANSMISSIVE, REFRACT}; 
Material::Material(vec3 amb, vec3 dif, vec3 spec, float shin, vec3 reflec, vec3 tran, float refract) {
	this->ambient = amb; this->diffuse = dif; this->specular = spec; this->shininess = shin; this->reflective = reflec; this->transmissive = tran; this->refraction = refract;
}

void Object::setType(OBJECT_TYPE t)
{
	this->type = t;
}

OBJECT_TYPE Object::getType()
{
	return this->type;
}

Object::Object() {
	
}


Object::Object(OBJECT_TYPE type, Material m) {
	this->material = m;
	this->type = type;

}

BoundingBox Object::getBounds() {
	return this->bbox;
}

float Object::intersect(Ray*ray, Object* obj, float nearT, float farT, vec3& hp, vec3& hp_norm, bool pick, std::string prefix)
{
	Sphere* sphere;
	Plane* plane;
	Triangle* triangle;
	
	switch (obj->getType()) {

	case SPHERE:
		sphere = (Sphere*)obj;
		return sphere->intersect(ray, sphere, nearT, farT, hp, hp_norm, pick, prefix);

	case PLANE:
		plane = (Plane*)obj;
		return plane->intersect(ray, plane, nearT, farT, hp, hp_norm, pick, prefix);
	case MESH:
		triangle = (Triangle*)obj;
		
		return triangle->intersect(ray, *triangle, nearT, farT, hp, hp_norm, pick, prefix);

	}

	
}

/*****************************************SPHERE**************************************************************************************************/

Sphere::Sphere(vec3 pos, float r, Material m){
	this->position = pos;
	this->radius = r;
	this->material = m;
	setType(SPHERE);
	this->bbox = this->getBounds();
}

float Sphere::intersect(Ray* ray, Sphere* obj, float nearT, float farT, Vertex& hp, Vector& hp_norm, bool pick, std::string prefix) {
	float result = -1;
	vec3& pos = obj->position;
	float radius = obj->radius;
	vec3 eminusc = ray->origin - pos;
	float a = glm::dot(ray->direction, ray->direction);
	float b = glm::dot(ray->direction, eminusc);
	float c = glm::dot(eminusc, eminusc) - radius * radius; 
	double discrim = pow(b, 2) - a * c;
	if (discrim >= 0) {
		double root = sqrt(discrim);
		float t = float((glm::dot(-ray->direction, eminusc) + root) / a);
		// if (pick) std::cout << prefix << "checking sphere " << glm::to_string(c) << " with t=" << t << std::endl;
		if (discrim > 0) {
			float t2 = float((glm::dot(-ray->direction, eminusc) - root) / a);
			// if (pick) std::cout << prefix << "checking sphere " << glm::to_string(c) << " with t2=" << t2 << std::endl;
			// choose the closest t intersection that's still >= near
			if ((t2 < t && t2 >= nearT) || (t2 > t && t < nearT)) {
				t = t2;
			}
		}
		if (t >= nearT && (farT < nearT || t <= farT)) {
			if (pick) {
				std::cout << prefix << "hit sphere " << glm::to_string(pos) << " at t=" << t << std::endl; }
			hp = ray->origin + t * ray->direction;
			hp_norm = glm::normalize(hp - pos);
			result = t;
		}
	}

	return result;
	

}

BoundingBox Sphere::getBounds() {
	BoundingBox b;
	float minX, minY, minZ, maxX, maxY, maxZ; 

	minX = this->position.x - this->radius; 
	minY = this->position.y - this->radius;
	minZ = this->position.z - this->radius;

	maxX = this->position.x + this->radius;
	maxY = this->position.y + this->radius;
	maxZ = this->position.z + this->radius;

	//b.min = vec3(minX, minY, minZ); b.max = vec3(maxX, maxY, maxZ);
	return BoundingBox(vec3(minX,minY,minZ), vec3(maxX,maxY,maxZ));
}
void Sphere::setMaterial(Material m) {
	this->material = m;
}
/*****************************************PLANE**************************************************************************************************/
Plane::Plane() {

}
Plane::Plane(vec3 pos, vec3 normal, Material m) {
	this->position = pos;
	this->normal = normal;
	this->material = m;
	setType(PLANE);
	this->bbox = this->getBounds();
}
float Plane::intersect(Ray* ray, Plane* obj, float nearT, float farT, vec3& hp, vec3& hp_norm, bool pick, std::string prefix) {
	float ret = -1;

	Vertex& pos = obj->position;
	Vector normal = glm::normalize(obj->normal);
	float angle = glm::dot(normal, ray->direction);
	if (angle != 0) {
		float t = glm::dot(normal, pos - ray->origin) / angle;
		if (t >= nearT && (farT < nearT || t <= farT)) {
			if (pick) std::cout << prefix << "hit plane " << glm::to_string(pos) << " at t=" << t << std::endl;
			hp = ray->origin + t * ray->direction;
			hp_norm = normal;
			ret = t;
		}
	}
	return ret;
}

BoundingBox Plane::getBounds() {
	float val = 50.0f;

	return BoundingBox(vec3(this->position.x-val, this->position.y-val, this->position.z-val),vec3(this->position.x+val , this->position.y+val , this->position.z+val));
}

bool Plane::intersectBoundingBox(Plane* p, BoundingBox box) {
	vec3 centre = box.calcCentre();
	vec3 ex = box.max - centre; 

	float r = ex.x * abs(p->normal.x) + ex.y * abs(p->normal.y) + ex.z * abs(p->normal.z);
	float s = glm::dot(p->normal, centre) - p->size; 

	return abs(s) <= r;


}
/*****************************************MESH**************************************************************************************************/
Mesh::Mesh(std::vector<Triangle*> tris, Material m){
	this->triangles = tris; 
	this->material = m;
	setType(MESH);
	this->bbox = this->getBounds();
	this->meshBVH = new BVH(triangles);

}

Triangle::Triangle(){}

BoundingBox Mesh::getBounds() {
	BoundingBox box;
	float minX = 1000, minY= 1000, minZ=1000, maxX=-1000, maxY=-10000, maxZ=-1000.0f;

	for (auto& tri : this->triangles) {
		vec3 a = tri->a;
		vec3 b = tri->b;
		vec3 c = tri->c;

		minX = glm::min(glm::min(a.x, b.x), c.x);
		minY = glm::min(glm::min(a.y, b.y), c.y);
		minZ = glm::min(glm::min(a.z, b.z), c.z);

		maxX = glm::max(glm::max(a.x, b.x), c.x);
		maxY = glm::max(glm::max(a.y, b.y), c.y);
		maxZ = glm::max(glm::max(a.z, b.z), c.z);

		tri->box.min = vec3(minX, minY, minZ);
		tri->box.max = vec3(maxX, maxY, maxZ);
	
	}
	for (auto& tri : this->triangles) {
		box.join(tri->box);
		//this->box = this->box.join(tri.box);
	}
	box.centre = box.calcCentre();
	return box;
}


Triangle::Triangle(vec3 a, vec3 b, vec3 c) {
	this->a = a;
	this->b = b;
	this->c = c;
	this->box = this->calcBounds();
	this->setType(TRIANGLE);
}

BoundingBox Triangle::calcBounds() {
	float minX = std::min({ this->a.x,this->b.x,this->c.x });
	float minY = std::min({ this->a.y,this->b.y,this->c.y });
	float minZ = std::min({ this->a.z,this->b.z,this->c.z });

	float maxX = std::max({ this->a.x,this->b.x,this->c.x });
	float maxY = std::max({ this->a.y,this->b.y, this->c.y });
	float maxZ = std::max({ this->a.z, this->b.z,this->c.z });

	vec3 min = vec3(minX, minY, minZ);
	vec3 max = vec3(maxX, maxY, maxZ);
	return BoundingBox(min, max);
}
float Triangle::intersect(Ray* ray, Triangle tri, float nearT, float farT, vec3& hp, vec3& norm, bool pick, std::string prefix) {
	vec3 a = tri.a; vec3 b = tri.b; vec3 c = tri.c;
	norm = glm::normalize(glm::cross(c - b, a - b));

	float angle = glm::dot(norm, ray->direction);
	if (angle != 0) {
		float t = glm::dot(norm, (a - ray->origin)) / angle; 
		if (t >= nearT && (farT < nearT || t <= farT)) {
			hp = ray->origin + t * ray->direction;
			vec3 bary = vec3(glm::dot(glm::cross(b - a, hp - a), norm), glm::dot(glm::cross(c-b, hp-b),norm), glm::dot(glm::cross(a-c,hp-c),norm));
			if (bary.x > 0 && bary.y > 0 && bary.z > 0) {
				if (pick) std::cout << prefix << "hit triangle " << glm::to_string(a) << " at t=" << t << std::endl;
				return t;
			}
			if (ALLOW_HIT_MESH_BACK && bary.x < 0 && bary.y < 0 && bary.z < 0) {
				if (pick) std::cout << prefix << "hit back of triangle " << glm::to_string(a) << " at t=" << t << std::endl;
				return t;
			}
		}
	}
	return -1;
}

bool Triangle::intersectBoundingBox(Triangle tri, BoundingBox box) {

	// SAT Test
	vec3 v0 = tri.a;
	vec3 v1 = tri.b;
	vec3 v2 = tri.c;

	vec3 boxCentre = box.calcCentre();
	

	float e0 = (box.max.x - box.min.x) * 0.5f;
	float e1 = (box.max.y - box.min.y) * 0.5f;
	float e2 = (box.max.z - box.min.z) * 0.5f;
	vec3 e = vec3(e0, e1, e2);

	v0 = v0- boxCentre; 
	v1 = v1- boxCentre;
	v2 = v2 -boxCentre;

	vec3 ab = v1 - v0;
	vec3 bc = v2 - v1;
	vec3 ca = v0 - v2;

	
	vec3 a00 = vec3(0, -ab.z, ab.y);
	vec3 a01 = vec3(0, -bc.z, bc.y);
	vec3 a02 = vec3(0, -ca.z, ca.y);
	vec3 a10 = vec3(ab.z, 0, -ab.x);
	vec3 a11 = vec3(bc.z, 0, -bc.x);
	vec3 a12 = vec3(ca.z, 0, -ca.x);
	vec3 a20 = vec3(-ab.y, ab.x, 0);
	vec3 a21 = vec3(-bc.y, bc.x, 0);
	vec3 a22 = vec3(-ca.y, ca.x, 0);

	//// test axis 00
	float p0 = glm::dot(v0, a00);
	float p1 = glm::dot(v1, a00);
	float p2 = glm::dot(v2, a00);
	float r = e.y * abs(ab.z) + e.z * abs(ab.y);
	if (std::max(-std::max({p0,p1,p2 }), std::min({p0,p1,p2})) > r) {
		return false;
	}

	////axis 01
	p0 = glm::dot(v0, a01);
	p1 = glm::dot(v1, a01);
	p2 = glm::dot(v2, a01);
	r = e.y * abs(bc.z) + e.z * abs(bc.y);
	if (std::max(-std::max({ p0,p1,p2 }), std::min({ p0,p1,p2 })) > r) {
		return false;
	}
	
	// axis 02
	p0 = glm::dot(v0, a02);
	p1 = glm::dot(v1, a02);
	p2 = glm::dot(v2, a02);
	r = e.y * abs(ca.z) + e.z * abs(ca.y);
	if (std::max(-std::max({ p0,p1,p2 }), std::min({ p0,p1,p2 })) > r) {
		return false;
	}
	
	//axis 10
	p0 = glm::dot(v0, a10);
	p1 = glm::dot(v1, a10);
	p2 = glm::dot(v2, a10);
	r = e.x * abs(ab.z) + e.z * abs(ab.x);
	if (std::max(-std::max({ p0,p1,p2 }), std::min({ p0,p1,p2 })) > r) {
		return false;
	}

	//axis 11
	p0 = glm::dot(v0, a11);
	p1 = glm::dot(v1, a11);
	p2 = glm::dot(v2, a11);
	r = e.x * abs(bc.z) + e.z * abs(bc.x);
	if (std::max(-std::max({ p0,p1,p2 }), std::min({ p0,p1,p2 })) > r) {
		return false;
	}
	//axis 12
	p0 = glm::dot(v0, a12);
	p1 = glm::dot(v1, a12);
	p2 = glm::dot(v2, a12);
	r = e.x * abs(ca.z) + e.z * abs(ca.x);
	if (std::max(-std::max({ p0,p1,p2 }), std::min({ p0,p1,p2 })) > r) {
		return false;
	}

	//axis 20
	p0 = glm::dot(v0, a20);
	p1 = glm::dot(v1, a20);
	p2 = glm::dot(v2, a20);
	r = e.x * abs(ab.y) + e.y * abs(ab.x);
	if (std::max(-std::max({ p0,p1,p2 }), std::min({ p0,p1,p2 })) > r) {
		return false;
	}

	//axis 21
	p0 = glm::dot(v0, a21);
	p1 = glm::dot(v1, a21);
	p2 = glm::dot(v2, a21);
	r = e.x * abs(bc.y) + e.y * abs(bc.x);
	if (std::max(-std::max({ p0,p1,p2 }), std::min({ p0,p1,p2 })) > r) {
		return false;
	}

	//axis 22
	p0 = glm::dot(v0, a22);
	p1 = glm::dot(v1, a22);
	p2 = glm::dot(v2, a22);
	r = e.x * abs(ca.y) + e.y * abs(ca.x);
	if (std::max(-std::max({ p0,p1,p2 }), std::min({ p0,p1,p2 })) > r) {
		return false;
	}

	if (std::max({ v0.x, v1.x,v2.x }) < -e.x || std::min({ v0.x, v1.x, v2.x }) > e.x) {
		return false;
	}
	if (std::max({ v0.y,v1.y,v2.y }) < -e.y || std::min({ v0.y,v1.y,v2.y }) > e.y) {
		return false;
	}
	if (std::max({ v0.z,v1.z,v2.z }) < -e.z || std::min({ v0.z,v1.z,v2.z }) > e.z) {
		return false;
	}

	Plane *p = new Plane(); 
	p->normal = glm::normalize(glm::cross(bc, ab));
	p->size = dot(p->normal, tri.a);

	return p->intersectBoundingBox(p, box);
}

bool Triangle::testAxis(float p0, float p1, float p2, vec3 axis, float r) {
	return std::max(-std::max({ p0,p1,p2 }), std::min({ p0,p1,p2 })) > r;
}

float Mesh::intersect(Ray* ray, Mesh* obj, float nearT, float farT, vec3& hp, vec3& hp_norm, bool pick, std::string prefix) {
	float nearest_t = -1;
	float t;

	vec3 tri_hp, tri_norm;

	int hit_i = -1; 
	for (int i = 0; i < obj->triangles.size(); i++) {
		auto& tri = obj->triangles[i];
		t = tri->intersect(ray, *tri, nearT, farT, tri_hp, tri_norm, pick, prefix);
		if (t >= nearT && (nearest_t < 0 || t < nearest_t)) {
			nearest_t = t;
			farT = t;
			hp = tri_hp;
			hp_norm = tri_norm;
			hit_i = i;
		}
	}
	if (pick && hit_i >= 0) { std::cout << prefix << "mesh chose triangle #" << hit_i << std::endl; }
	return nearest_t;

}





/*****************************************Partical**************************************************************************************************/

Partical::Partical()
{
	this->setType(PARTICAL);
	this->generateParticals();
	
}

void Partical::generateParticals()
{
	Sphere* tempSphere;
	std::vector<Sphere*> spheres;
	float MAX_X = 4.5f;
	float MAX_Z = 35.0f;
	float MIN_Y = -4.5f;
	float MAX_Y = 7.0f;
	float MIN_X = -5.5f;
	float MIN_Z = 5.0f;

	for (int i = 0; i < NUM_PARTICALS; i++) {
		float randX = getCord(MIN_X, MAX_X, true);
		float randY = getCord(MIN_Y, MAX_Y, true);
		float randZ = getCord(MIN_Z, MAX_Z, false);

		float randR = getCord(0.15, 0.75, true);

		
		tempSphere = new Sphere(vec3(randX, randY, randZ), randR, this->material);
		tempSphere->setMaterial(getRandomMaterial());
		tempSphere->bbox = tempSphere->getBounds();
		spheres.push_back(tempSphere);
	}
	this->particals = spheres;
}

float Partical::getCord(float minLimit, float maxLimit, bool isX) {
	std::random_device rd; 
	std::mt19937 gen(rd());

	std::uniform_real_distribution<> distr(minLimit, maxLimit);
	float ret = distr(gen);

	//float ret = minLimit + (rand() % (int)(maxLimit - minLimit + 1));
	if (!isX) {
		ret *= -1;
	}
	return ret;

}

Material Partical::getRandomMaterial() {
	Material m;
	
	float randX, randY, randZ;
	bool isReflect, isTransmis, isRefract;

	//srand((unsigned)time(NULL));
	for (int i = 0; i < 7; i++) {
		int a = (rand() % 100 + 1);
		int b = (rand() % 100 + 1);
		int c = (rand() % 100 + 1);
		int d = (rand() % 300 + 1);
		
		randX = (float)rand() / RAND_MAX;
		randY = (float)rand() / RAND_MAX;
		randZ = (float)rand() / RAND_MAX;

		// 50% chance 
		isReflect = a < 50;
		// 40% chance
		isTransmis = b < 40;
		//15% chance 
		isRefract = c < 15;

		switch (i) {
		case AMBIENT:
			m.ambient = vec3(randX, randY, randZ);
			break;
		case DIFFUSE:
			m.diffuse = vec3(randX, randY, randZ);
			break;
		case SPECULAR: 
			m.specular = vec3(randX, randY, randZ);
			break;
		case SHININESS:
			m.shininess = (float)d;
			break;
		case REFLECTIVE:
			if (isReflect) {
				m.reflective = vec3(randX, randX, randX);
				break;
			}
			else { m.reflective = vec3(-1, -1, -1); break; }
		case TRANSMISSIVE:
			if (isTransmis) {
				m.transmissive = vec3(randY, randY, randY);
				break;
			}
			else { m.transmissive = vec3(-1, -1, -1); break; }
		case REFRACT:
			if (isRefract && isTransmis) {
				m.refraction = 1.33;
				break;
			}
			else { m.refraction = 1.0; break; }
		}

	}
	return m;

	
}