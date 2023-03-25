#include <glm/glm.hpp>

#include"Scene.h"


typedef glm::vec3 point3;
typedef glm::vec3 colour3;

extern double fov;
extern colour3 background_colour;


void choose_scene(char const *fn, Scene &theScene, bool useCamera);
bool trace(const point3 &e, const point3 &s, colour3 &colour, bool pick);

bool newTrace(Ray *ray, colour3& colour, bool pick);