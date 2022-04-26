#pragma once
#include "../vec3.h"
#include "../ppm.h"
#include "../ray.h"
#include "../texture.h"
#include "../lib/error.h"

struct sphere {
	struct vec3 center;
	float radius;
};

struct plane {
	struct vec3 p0, normal;
};

struct light {
	struct vec3 pos;
	float intensity;
};

struct material {
	struct vec3 color;
	struct texture *texture;
	float shininess, reflectiveness;
	float diffuse_constant, specular_constant;
};

#define MAT_GLOSSY(color_v) \
	{.color=color_v, .diffuse_constant=1, \
	 .specular_constant=1, .shininess=400}

#define MAT_MATTE(color_v) {.color=color_v, .diffuse_constant=1}

#define MAT_REFLECTIVE(color_v, ref) \
	{.color=color_v, .diffuse_constant=1, .reflectiveness=ref, \
	 .specular_constant=1, .shininess=800}

#define MAT_GLOSSY_T(texture_v) \
	{.texture=texture_v, .diffuse_constant=1, \
	 .specular_constant=1, .shininess=400}

#define MAT_MATTE_T(texture_v) {.texture=texture_v, .diffuse_constant=1}

#define MAT_REFLECTIVE_T(texture_v, ref) \
	{.texture=texture_v, .diffuse_constant=1, .reflectiveness=ref, \
	 .specular_constant=1, .shininess=800}

typedef int (*ray_intersection_fn)(struct ray *r, struct entity *e,
				   struct intersection *it);

typedef struct vec3 (*lookup_texture_fn)(struct entity *e,
					 struct vec3 pos);

struct entity {
	enum entity_type{
		ENT_SPHERE,
		ENT_PLANE,
	} type;
	union {
		struct sphere s;
		struct plane p;
	} u;
	struct material material;
	ray_intersection_fn ray_intersects;
	lookup_texture_fn lookup_texture;
};

int ray_intersects_sphere(struct ray *r, struct entity *e, struct intersection *it);
struct vec3 lookup_sphere_texture(struct entity *e, struct vec3 pos);

int ray_intersects_plane(struct ray *r, struct entity *e, struct intersection *it);

static struct vec3 missing_lookup_texture_fn(struct entity *e,
					     struct vec3 pos)
{
	die("Missing lookup texture function for entity %d\n", e->type);
}

#define ENTITY_SPHERE(center_v, radius_v, material_v) \
	((struct entity) {.type=ENT_SPHERE, .u={.s={.center=center_v, .radius=radius_v}}, \
	 .material=material_v, .ray_intersects=ray_intersects_sphere, \
	 .lookup_texture=lookup_sphere_texture})

#define ENTITY_PLANE(p0_v, normal_v, material_v) \
	((struct entity) {.type=ENT_PLANE, .u={.p={.p0=p0_v, .normal=normal_v}}, \
	 .material=material_v, .ray_intersects=ray_intersects_plane, \
	 .lookup_texture=missing_lookup_texture_fn})
