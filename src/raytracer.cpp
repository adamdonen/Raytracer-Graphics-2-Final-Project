// Ray Tracer Assignment Model Solution
// Winter 2021

// The JSON library allows you to reference JSON arrays like C++ vectors and JSON objects like C++ maps.

#include "raytracer.h"

#include <iostream>
#include <fstream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "json2scene.h"
#include "BVH.h"


typedef glm::vec3 colour;
typedef glm::vec3 Vertex; 
typedef glm::vec3 Vector;

const double PI = 3.1415926535897932384626433832795;
const char *PATH = "scenes/";

const float SELF_HIT = 2e-3;
const int MAX_R_DEPTH = 8;

double fov = 60;
colour background_colour(0, 0, 0);

const bool DISABLE_AMBIENT = false;
const bool DISABLE_DIFFUSE = false;
const bool DISABLE_SPECULAR = false;
const bool DISABLE_SHADOW = false;
const bool DISABLE_REFLECTION = true;
const bool DISABLE_TRANSMISSION = false;
const bool DISABLE_REFRACTION = false;
const bool DISABLE_SHADOW_TRANSPARENCY = false;



// this could happen if: e.g. we have inconsistent winding
const bool ALLOW_HIT_MESH_BACK = true;

json jscene;
Scene scene;

BVH root;

colour light(Object *obj, const Vertex &e, const Vertex &at, const Vector &snorm, int r_depth, bool pick, std::string prefix);

/****************************************************************************/

void choose_scene(char const *fn, Scene &theScene, bool useCamera) {
	if (fn == NULL) {
		std::cout << "Using default input file " << PATH << "c.json\n";
		fn = "i";
	}

	std::cout << "Loading scene " << fn << std::endl;
	
	std::string fname = PATH + std::string(fn) + ".json";
	std::fstream in(fname);
	if (!in.is_open()) {
		std::cout << "Unable to open scene file " << fname << std::endl;
		exit(EXIT_FAILURE);
	}
	
	in >> jscene;
  
  if (json_to_scene(jscene, scene, useCamera) < 0) {
		std::cout << "Error in scene file " << fname << std::endl;
		exit(EXIT_FAILURE);
  }

  Partical* p = (Partical*)scene.objects.front();
  fov = scene.camera->fov;
  background_colour = scene.camera->background;

  theScene = scene;
  

}

float ray_sphere(Sphere* obj, const Vertex& e, const Vector& d, float nearT, float farT, Vertex& hp, Vector& hp_norm, bool pick, std::string prefix) {
    float result = -1;

    Vertex& c = obj->position;
    float radius = obj->radius;
    Vector eminusc = e - c;
    float ddotd = glm::dot(d, d);
    double disc = pow(glm::dot(d, eminusc), 2) - ddotd * (glm::dot(eminusc, eminusc) - radius * radius);

    if (disc >= 0) {
        double root = sqrt(disc);
        float t = float((glm::dot(-d, eminusc) + root) / ddotd);
        // if (pick) std::cout << prefix << "checking sphere " << glm::to_string(c) << " with t=" << t << std::endl;
        if (disc > 0) {
            float t2 = float((glm::dot(-d, eminusc) - root) / ddotd);
            // if (pick) std::cout << prefix << "checking sphere " << glm::to_string(c) << " with t2=" << t2 << std::endl;
            // choose the closest t intersection that's still >= near
            if ((t2 < t && t2 >= nearT) || (t2 > t && t < nearT)) {
                t = t2;
            }
        }
        if (t >= nearT && (farT < nearT || t <= farT)) {
            if (pick) {
                std::cout << prefix << "hit sphere " << glm::to_string(c) << " at t=" << t << std::endl; }
            hp = e + t * d;
            hp_norm = glm::normalize(hp - c);
            result = t;
        }
    }

    return result;
}

float ray_plane(Plane *obj, const Vertex &e, const Vector &d, float nearT, float farT, Vertex &hp, Vector &hp_norm, bool pick, std::string prefix) {
  float result = -1;

  Vertex &a = obj->position;
  Vector n = glm::normalize(obj->normal);
  float ndotd = glm::dot(n, d);
  if (ndotd != 0) {
    float t = glm::dot(n, a - e) / ndotd;
    if (t >= nearT && (farT < nearT || t <= farT)) {
      if (pick) std::cout << prefix << "hit plane " << glm::to_string(a) << " at t=" << t << std::endl;
      hp = e + t * d;
      hp_norm = n;
      result = t;
    }
  }
  
  return result;
}

float ray_triangle(Triangle &tri, const point3 &e, const point3 &d, float nearT, float farT, Vertex &pt, Vector &n, bool pick, std::string prefix) {
  Vertex &a = tri.a;
  Vertex &b = tri.b;
  Vertex &c = tri.c;
  n = glm::normalize(glm::cross(c-b, a-b));

  float ndotd = glm::dot(n, d);
  // respect triangle winding
  if (ndotd != 0) {
    float t = glm::dot(n, a - e) / ndotd;
    if (t >= nearT && (farT < nearT || t <= farT)) {
      pt = e + t * d;
      Vertex bary = point3(
        glm::dot(glm::cross(b-a,pt-a),n),
        glm::dot(glm::cross(c-b,pt-b),n),
        glm::dot(glm::cross(a-c,pt-c),n)
      );
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

float ray_mesh(Mesh *obj, const Vertex &e, const Vector &d, float nearT, float farT, Vertex &hp, Vector &hp_norm, bool pick, std::string prefix) {
  float nearest_t = -1;
  float t;
  Vertex tri_hp;
  Vector tri_norm;

  int hit_i = -1;
  for (int i = 0; i < obj->triangles.size(); i++) {
    auto &tri = obj->triangles[i];
    t = ray_triangle(*tri, e, d, nearT, farT, tri_hp, tri_norm, pick, prefix);
		if (t >= nearT && (nearest_t < 0 || t < nearest_t)) {
			nearest_t = t;
      farT = t;
      hp = tri_hp;
      hp_norm = tri_norm;
      hit_i = i;
    }
  }
  if (pick && hit_i >= 0) std::cout << prefix << "mesh chose triangle #" << hit_i << std::endl;
  
  return nearest_t;
}

float hit(const Vertex &e, const Vector &d, float nearT, float farT, Vertex &hit_at, Vector &hit_normal, Object *&hit_object, vec3 *opacity_sum, bool pick, std::string prefix) {
  float t,t1, nearest_t = -1;

    Vertex at;
    Vector normal;

	// traverse the objects
  for (auto &&object : scene.objects) {
    if (object->getType() == SPHERE) {
      Sphere *sphere = (Sphere *)(object);
     
      //t = ray_sphere(sphere, e, d, nearT, farT, at, normal, pick, prefix);
      t = sphere->intersect(new Ray(e,d), sphere, nearT, farT, at, normal, pick, prefix);

      if (pick) { printf("Sphere %f, t1:  ", t); }
    } else if (object->getType() == PLANE) {
      Plane *plane = (Plane *)(object);
      t = ray_plane(plane, e, d, nearT, farT, at, normal, pick, prefix);
      //t = plane->intersect(new Ray(e, d), plane, nearT, farT, at, normal, pick, prefix);
      if (pick) { printf("Plane t %f", t); }
    } else if (object->getType() == MESH) {
      Mesh *mesh = (Mesh *)(object);
      t = ray_mesh(mesh, e, d, nearT, farT, at, normal, pick, prefix);
    
    }

    if (NULL != opacity_sum) {
      // if this is non-NULL, use it to send back the amount blocked by opacity (because we're in shadow), stopping at (1,1,1)
      if (t >= nearT && (farT < nearT || t <= farT)) {
        // shadow hit
        if (DISABLE_SHADOW_TRANSPARENCY) {
          *opacity_sum = vec3(1, 1, 1);
          hit_object = object;
          return 1;
        } else {
          vec3 opacity = vec3(1, 1, 1) - object->material.transmissive;
          *opacity_sum += opacity;
          *opacity_sum = glm::clamp(*opacity_sum, 0.0f, 1.0f);
          if (pick) std::cout << prefix << "shadow adding opacity " << glm::to_string(opacity) << " to get sum " << glm::to_string(*opacity_sum) << std::endl;
          if (*opacity_sum == vec3(1,1,1)) {
            return 1;
          }
        }
      }
    }

	if (t > 0 && (nearest_t < 0 || t < nearest_t)) {
      nearest_t = t;
      if (NULL == opacity_sum) {
        // if we're calculating shadow transparency, we can't skip anything
        farT = t;
      }
      hit_at = at;
      hit_normal = normal;
      hit_object = object;
    }
  }
  
  return nearest_t;
}

float newHit(Ray* ray, float nearT, float farT, vec3 &hit_at, vec3 &hit_normal, Object* &hit_object, vec3 *opacity_sum, bool pick, std::string prefix ) {
    float t, nearest_t = -1;
    vec3 at; vec3 normal;
    
    for (auto&& object : scene.objects) {
        if (object->getType() == SPHERE) {
            Sphere* sphere = (Sphere*)(object);
            t = sphere->intersect(ray, sphere, nearT, farT, at, normal, pick, prefix);
        }
        else if (object->getType() == PLANE) {
            Plane* plane = (Plane*)(object);
            t = plane->intersect(ray, plane, nearT, farT, at, normal, pick, prefix);
        }
        else if (object->getType() == MESH) {
            Mesh* mesh = (Mesh*)(object);
            t = mesh->intersect(ray, mesh, nearT, farT, at, normal, pick, prefix);
          }

        
        if (opacity_sum != NULL) {
            if (t >= nearT && (farT < nearT || t <= farT)) {
                if (DISABLE_SHADOW_TRANSPARENCY) {
                    *opacity_sum = vec3(1, 1, 1);
                    hit_object = object;
                    return 1;
                }
                else {
                    vec3 opacity = vec3(1, 1, 1) - object->material.transmissive;
                    *opacity_sum += opacity;
                    *opacity_sum = glm::clamp(*opacity_sum, 0.0f, 1.0f);
                    if (pick) std::cout << prefix << "shadow adding opacity " << glm::to_string(opacity) << " to get sum " << glm::to_string(*opacity_sum) << std::endl;
                    if (*opacity_sum == vec3(1, 1, 1)) {
                        return 1;
                    }
                }
            }
        }

        if (t > 0 && (nearest_t < 0 || t < nearest_t)) {
            nearest_t = t;
            if (NULL == opacity_sum) {
                // if we're calculating shadow transparency, we can't skip anything
                farT = t;
            }
            hit_at = at;
            hit_normal = normal;
            hit_object = object;
        }
    }

    return nearest_t;
}

vec3 reflect(Object *obj, const Vertex &e, const Vertex &at, const Vector &snorm, int r_depth, bool pick, std::string prefix) {
  vec3 reflect_colour;

  Vector v = glm::normalize(e - at);
  Vector r = glm::normalize(2 * glm::dot(snorm, v) * snorm - v);
  float t;

	Vertex hit_at;
  Vector hit_normal;
  Object *hit_object = NULL;

  if (pick) std::cout << prefix << "check reflection from " << glm::to_string(at) << " going " << glm::to_string(r) << std::endl;
  // if (pick) std::cout << prefix << "      reflection norm " << glm::to_string(snorm) << std::endl;
  //t = hit(at, r, SELF_HIT, 0, hit_at, hit_normal, hit_object, NULL, pick, prefix + " ");

  t = scene.theBVH->traverse(Ray(at, r), scene.theBVH->root, r, SELF_HIT, 0, hit_at, hit_normal, hit_object, NULL, pick, prefix + " ");

  if (t >= SELF_HIT) {
    reflect_colour = light(hit_object, at, hit_at, hit_normal, r_depth + 1, pick, prefix);
    if (pick) std::cout << prefix << " reflection on " << obj->type << " colour " << glm::to_string(reflect_colour) << std::endl;
  } else {
    reflect_colour = background_colour;
    if (pick) std::cout << prefix << " no reflection r=" << glm::to_string(r) << " t=" << t << std::endl;
  }
  
  return reflect_colour;
}

bool refract(const Vector &r, const Vector &snorm, float index_of_refraction, Vector &refracted, bool pick, std::string prefix) {
  float eta_i, eta_r;
  Vector n = snorm;
  Vector vi = glm::normalize(r);
  float vi_dot_n = glm::dot(vi, n);
  
  if (vi_dot_n < 0) {
    eta_i = 1;
    eta_r = index_of_refraction;
    if (pick) std::cout << prefix << "refraction, entering the object\n";
  } else {
    eta_i = index_of_refraction;
    eta_r = 1;
    n = -snorm;
    vi_dot_n = -vi_dot_n;
    if (pick) std::cout << prefix << "refraction, leaving the object\n";
  }
  
  float radicand = 1 - (eta_i * eta_i) * (1 - vi_dot_n * vi_dot_n) / (eta_r * eta_r);
  if (radicand >= 0) {
    refracted = eta_i * (vi - n * vi_dot_n) / eta_r - n * sqrt(radicand);
    if (pick) std::cout << prefix << " refracting path from " << glm::to_string(r) << " to " << glm::to_string(refracted) << std::endl;
    return true;
  } else {
    vi = -vi;
    refracted = 2 * glm::dot(n, vi) * n - vi;
    if (pick) std::cout << prefix << " total internal reflection from " << glm::to_string(r) << " to " << glm::to_string(refracted) << std::endl;
    return false;
  }
}

vec3 transmit(Object *obj, const Vertex &e, const Vertex &at, const Vector &snorm, int r_depth, bool pick, std::string prefix) {
  vec3 transmit_colour;
  const Material &mat = obj->material;

  Vector vi = glm::normalize(at-e), vr = vi;
  float t;

  Vertex hit_at;
  Vector hit_normal;
  Object *hit_object = NULL;

  if (mat.refraction > 0 && !DISABLE_REFRACTION) {
    refract(vi, snorm, mat.refraction, vr, pick, prefix);
  }

 // t = hit(at, vr, SELF_HIT, 0, hit_at, hit_normal, hit_object, NULL, pick, prefix);
  t = scene.theBVH->traverse(Ray(at, vr), scene.theBVH->root, vr, SELF_HIT, 0, hit_at, hit_normal, hit_object, NULL, pick, prefix);
  //t = newHit(new Ray(at, vr), SELF_HIT, 0, hit_at, hit_normal, hit_object, NULL, pick, prefix);
  if (t >= SELF_HIT) {
    transmit_colour = light(hit_object, at, hit_at, hit_normal, r_depth + 1, pick, prefix + " ");
    if (pick) std::cout << prefix << "transmission to " << obj->type << " colour " << glm::to_string(transmit_colour) << std::endl;
  } else {
    transmit_colour = background_colour;
    if (pick) std::cout << prefix << "no transmission t=" << t << std::endl;
  }

  return transmit_colour;
}

vec3 light(Object *obj, const Vertex &e, const Vertex &at, const Vector &snorm, int r_depth, bool pick, std::string prefix) {
  vec3 reflect_colour, transmit_colour, direct_colour;
  Material& mat = obj->material;
  
  prefix += "\n+";

  if (mat.reflective[0] != -1 && r_depth < MAX_R_DEPTH && !DISABLE_REFLECTION) {
    reflect_colour = reflect(obj, e, at, snorm, r_depth, pick, prefix);
  }

  if (mat.transmissive[0] != -1 && r_depth < MAX_R_DEPTH && !DISABLE_TRANSMISSION) {
    transmit_colour = transmit(obj, e, at, snorm, r_depth, pick, prefix);
  }

  for (auto &&light : scene.lights) {

	if (light->getType() == AMBIENT) {
      AmbientLight *a = (AmbientLight *)(light);
      if (!DISABLE_AMBIENT) {
  			direct_colour += a->colour * mat.ambient;
        if (pick) std::cout << prefix << "ambient lit a " << obj->type << " " << glm::to_string(a->colour * mat.ambient) << std::endl;
      }
	} 
    else {
      bool maybe_lit = false;
      float tfar = 0;
      Vector l;

  		if (light->getType() == DIRECTIONAL) {
        DirectionalLight *d = (DirectionalLight *)(light);

        l = -glm::normalize(d->direction);
        if (pick) {
            std::cout << prefix << "check directional " << glm::to_string(d->direction) << std::endl;
        }
        maybe_lit = true;

      } 
        else if (light->getType() == POINT_LIGHT) {
        PointLight *p = (PointLight *)(light);

  			l = p->position - at;
  			tfar = glm::length(l);
        if (pick) std::cout << prefix << "check point " << glm::to_string(p->position) << " going " << glm::to_string(l) << std::endl;
        l = glm::normalize(l);
        maybe_lit = true;

      } 
        else if (light->getType() == SPOT) {
        SpotLight *s = (SpotLight *)(light);
        
  			Vector l_orig = s->position - at;
  			tfar = glm::length(l_orig);
        l = glm::normalize(l_orig);
  			Vector dir = -glm::normalize(s->direction);
        if (acos(glm::dot(l, dir)) <= PI * double(s->cutoff) / 180.0) {
          if (pick) std::cout << prefix << "check spot " << glm::to_string(s->position) << " going " << glm::to_string(l_orig) << std::endl;
          maybe_lit = true;
        }
      }

      if (maybe_lit) {
        float t = -1;
        Object *shadowing_obj = NULL;
        colour shadow_opacity(0,0,0);

        if (!DISABLE_SHADOW) {
          if (pick) std::cout << prefix << " check shadow from " << glm::to_string(at) << " going " << glm::to_string(l) << " max t=" << tfar << std::endl;

          Vector unused_v;
         //t = hit(at, l, SELF_HIT, tfar, unused_v, unused_v, shadowing_obj, &shadow_opacity, pick, prefix + "  ");
         //t = newHit(new Ray(at, l), SELF_HIT, tfar, unused_v, unused_v, shadowing_obj, &shadow_opacity, pick, prefix + " ");
          t = scene.theBVH->traverse(Ray(at, l), scene.theBVH->root, l, SELF_HIT, tfar, unused_v, unused_v, shadowing_obj, &shadow_opacity, pick, prefix + "");
        }

        if (t > SELF_HIT && DISABLE_SHADOW_TRANSPARENCY) {
          if (pick) std::cout << prefix << "  shadowed by " << shadowing_obj->type << std::endl;
        } 
        else {
          if (pick && !DISABLE_SHADOW) {
            if (DISABLE_SHADOW_TRANSPARENCY) {
              std::cout << prefix << "  no shadow\n";
            }
            else {
              std::cout << prefix << "  shadow opacity amount=" << glm::to_string(shadow_opacity) << std::endl;
            }
          }

          colour c = light->colour;
          colour this_light_colour;
          Vector v = glm::normalize(e - at);
          Vector n = snorm;
          float dot = glm::dot(snorm, l);
          if (pick) {
              std::cout << prefix << "v: " << glm::to_string(v) << " norm" << glm::to_string(n) << "dot(norm, l) " << dot << std::endl;
          }
          if (dot < 0 && ALLOW_HIT_MESH_BACK && obj->getType() == MESH) {
            n = -snorm;
            dot = -dot;
          }
          
          if (dot > 0) {
          
                  if (!DISABLE_DIFFUSE) {
                      this_light_colour += glm::clamp(c * mat.diffuse * dot, 0.0f, 1.0f);
                  }
                  Vector r = 2 * dot * n - l;
                  float rdotv = glm::dot(r, v);
                  if (rdotv > 0 && !DISABLE_SPECULAR) {
                      this_light_colour += glm::clamp(c * mat.specular * float(pow(rdotv, mat.shininess)), 0.0f, 1.0f);
                  }
                  if (!DISABLE_SHADOW_TRANSPARENCY) {
                      this_light_colour *= (colour(1, 1, 1) - shadow_opacity);
                  }
                  if (pick) std::cout << prefix << light->type << " lit " << glm::to_string(this_light_colour) << " from " << glm::to_string(l) << std::endl;
                  direct_colour = glm::clamp(direct_colour + this_light_colour, 0.0f, 1.0f);
              
            
          }
        }
      }
	}
}
  
  colour colour = direct_colour;
  if (reflect_colour != vec3(0,0,0)) {
    colour += reflect_colour * mat.reflective;
  }
  if (transmit_colour != vec3(0,0,0)) {
    colour = colour * (glm::vec3(1,1,1) - mat.transmissive);
    colour += transmit_colour * mat.transmissive;
  }
  colour = glm::clamp(colour, 0.0f, 1.0f);

  if (pick) std::cout << prefix << "final colour " << glm::to_string(colour) << std::endl;
  
  return colour;
}

//bool trace(const Vertex &e, const Vertex &s, colour &colour, bool pick) {
//  Vector d = s - e;
//  float t;
//  Vertex hit_at;
//  Vector hit_normal;
//  Object *hit_object = NULL;
//
//  t = hit(e, d, 1.0f, 0.0f, hit_at, hit_normal, hit_object, NULL, pick, "");
//
//  if (t >= 1.0) {
//    // light it up if there was a hit
//    colour = light(hit_object, e, hit_at, hit_normal, 0, pick, "");
//    return true;
//  }
//
//  return false;
//}

bool newTrace(Ray *ray, colour& colour, bool pick) {
  
    Vector d = ray->direction - ray->origin;
    Vertex hit_at;
    Vector hit_normal;
    Object* hit_object = NULL;
    ray->direction = d;
        ray->t = scene.theBVH->traverse(*ray, scene.theBVH->root, d, 1.0, 0, hit_at, hit_normal, hit_object, NULL, pick, "");
        
        if (ray->getT() >= 1.0) {
            colour = light(hit_object, ray->origin, hit_at, hit_normal, 0, pick, "");
            return true;
        }
    
        return false;
    
}

