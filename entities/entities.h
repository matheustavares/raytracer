#pragma once
#include "../vec3.h"
#include "../ppm.h"
#include "../ray.h"

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

typedef int (*ray_intersection_fn)(struct ray *r, struct entity *e,
				   struct intersection *it);

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
};

int ray_intersects_sphere(struct ray *r, struct entity *e, struct intersection *it);
int ray_intersects_plane(struct ray *r, struct entity *e, struct intersection *it);

#define ENTITY_SPHERE(center_v, radius_v, material_v) \
	{.type=ENT_SPHERE, .u={.s={.center=center_v, .radius=radius_v}}, \
	 .material=material_v, .ray_intersects=ray_intersects_sphere}

#define ENTITY_PLANE(p0_v, normal_v, material_v) \
	{.type=ENT_PLANE, .u={.p={.p0=p0_v, .normal=normal_v}}, \
	 .material=material_v, .ray_intersects=ray_intersects_plane}
