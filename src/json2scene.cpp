// Ray Tracer Assignment JSON->CPP utility
// Winter 2021

#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <glm/glm.hpp>

#include "Camera.h"
#include "Object.h"
#include "json2scene.h"
#include "Light.h"
#include "Scene.h"

typedef glm::vec3 Vector; 
typedef glm::vec3 vec3;
typedef colour3 RGB;

using json = nlohmann::json;

std::vector<Object*> sceneObjects;
std::vector<Light*> sceneLights;

// camera 
Camera* camera; 
Object *currObject;
float field;
colour background;

// material properties
vec3 amb; 
vec3 diff; 
vec3 spec; 
float shin; 
vec3 reflect; 
vec3 transm; 
float refract; 

/****************************************************************************/

static glm::vec3 vector_to_vec3(const std::vector<float> &v) {
	return glm::vec3(v[0], v[1], v[2]);
}

int json_to_scene(json &jscene, Scene &s, bool useCamera) {
  json cam = jscene["camera"];
  if (cam.find("field") != cam.end()) {
    //s.cam.fov = cam["field"];
    field = cam["field"];
  }
  if (cam.find("background") != cam.end()) {
    //s.cam.background = vector_to_vec3(cam["background"]);
    background = vector_to_vec3(cam["background"]);
  }
  if (useCamera) {
      point3 pos = vec3(0, 3, 2);
      vec3 up = vec3(0, 1, 0);
      vec3 lookAt = vec3(2, 3, -1);
      camera = new Camera(pos, up, lookAt, 60, 1.0f, 2.0f, 10.0, background);
  }
  else
  {   camera = new Camera(field, background);
  }
   // Traverse the objects
  json &objects = jscene["objects"];
  for (json::iterator it = objects.begin(); it != objects.end(); ++it) {
    json &object = *it;
    // Every object will have a material, but all parameters are optional
    json &material = object["material"];
    if (material.find("ambient") != material.end()) {
      amb = vector_to_vec3(material["ambient"]);
    }
    else {
        amb = glm::vec3(0, 0, 0);
    }
    if (material.find("diffuse") != material.end()) {
      diff = vector_to_vec3(material["diffuse"]);
    }
    else {
       diff = glm::vec3(0, 0, 0);
    }
    if (material.find("specular") != material.end()) {
       spec = vector_to_vec3(material["specular"]);
    }
    else {
        spec = glm::vec3(0, 0, 0);
    }
    if (material.find("shininess") != material.end()) {
      shin = material["shininess"];
    }
    else {
       shin = -1.0;
    }
    if (material.find("reflective") != material.end()) {
      reflect = vector_to_vec3(material["reflective"]);
    }
    else {
        reflect = glm::vec3(-1, -1, -1);
    }
    if (material.find("transmissive") != material.end()) {
     transm = vector_to_vec3(material["transmissive"]);
    }
    else {
        transm = glm::vec3(-1, -1, -1);
    }
    if (material.find("refraction") != material.end()) {
      refract = material["refraction"];
    }
    else {
        refract = 1.0f;
    }

    Material m = Material::Material(amb, diff, spec, shin, reflect, transm, refract);

    // Every object in the scene will have a type
    if (object["type"] == "sphere") {
      // Every sphere has a position and a radius
      vec3 pos = vector_to_vec3(object["position"]);
      float radius = object["radius"];
      //s.objects.push_back(new Sphere(m, radius, pos));
      //currObject = new Sphere(pos, radius, m);
      sceneObjects.push_back(new Sphere(pos, radius, m));
    } else if (object["type"] == "plane") {
      // obType = PLANE;
      // Every plane has a position (point of intersection) and a normal
      vec3 pos = vector_to_vec3(object["position"]);
      Vector normal = vector_to_vec3(object["normal"]);
      //s.objects.push_back(new Plane(m, pos, normal));
      sceneObjects.push_back(new Plane(pos, normal, m));
    } else if (object["type"] == "mesh") {
      // obType = MESH;
      // Every mesh has a list of triangles 
      std::vector<Triangle*> tris;
      json &ts = object["triangles"];
      for (json::iterator ti = ts.begin(); ti != ts.end(); ++ti) {
        json &t = *ti;
        vec3 a = vector_to_vec3(t[0]);
        vec3 b = vector_to_vec3(t[1]);
        vec3 c = vector_to_vec3(t[2]);
   
        tris.push_back(new Triangle(a,b,c));
      }
      sceneObjects.push_back(new Mesh(tris, m));
    }
    
    else if (object["type"] == "partical") {
        sceneObjects.push_back(new Partical());
    }
    
    else {
      std::cout << "*** unrecognized object type " << object["type"] << "\n";
      return -1;
    }
    
  }
  Partical* p = (Partical*)sceneObjects.front();
  // Traverse the lights
  json &lights = jscene["lights"];
  for (json::iterator it = lights.begin(); it != lights.end(); ++it) {
    json &light = *it;
    
    // Every light in the scene will have a colour (ired, igreen, iblue)
    RGB colour = vector_to_vec3(light["color"]);
    
    if (light["type"] == "ambient") {
      // There should only be one ambient light
     /* for (Light *l : s.lights) {
        if (l->type == "ambient") {
          std::cout << "*** there should only be one ambient light!\n";
          return -1;
        }
      }*/
        sceneLights.push_back(new AmbientLight(colour));
      //s.lights.push_back(new AmbientLight(colour));
    } else if (light["type"] == "directional") {
      // Every directional light has a direction
      Vector direction = vector_to_vec3(light["direction"]);
     // s.lights.push_back(new DirectionalLight(colour, direction));
      sceneLights.push_back(new DirectionalLight(direction, colour));
    } else if (light["type"] == "point") {
      // Every point light has a position
      vec3 pos = vector_to_vec3(light["position"]);
     // s.lights.push_back(new PointLight(colour, pos));
      sceneLights.push_back(new PointLight(pos, colour));
    } else if (light["type"] == "spot") {
      // Every spot light has a position, direction, and cutoff
      vec3 pos = vector_to_vec3(light["position"]);
      Vector direction = vector_to_vec3(light["direction"]);
      float cutoff = light["cutoff"];
    //  s.lights.push_back(new SpotLight(colour, pos, direction, cutoff));
      sceneLights.push_back(new SpotLight(pos, direction, cutoff, colour));
    } else {
      std::cout << "*** unrecognized light type " << light["type"] << "\n";
      return -1;
    }
  }
  
  Scene *scene = new Scene(sceneObjects, sceneLights, camera);
  s = *scene;
  return 0;
}

/****************************************************************************/

