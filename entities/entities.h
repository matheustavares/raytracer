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
	struct vec3 color;
	ray_intersection_fn ray_intersects;
};

int ray_intersects_sphere(struct ray *r, struct entity *e, struct intersection *it);
int ray_intersects_plane(struct ray *r, struct entity *e, struct intersection *it);

#define ENTITY_SPHERE(center_v, radius_v, color_v) \
	{.type=ENT_SPHERE, .u={.s={.center=center_v, .radius=radius_v}}, \
	 .color=color_v, .ray_intersects=ray_intersects_sphere}

#define ENTITY_PLANE(p0_v, normal_v, color_v) \
	{.type=ENT_PLANE, .u={.p={.p0=p0_v, .normal=normal_v}}, \
	 .color=color_v, .ray_intersects=ray_intersects_plane}
