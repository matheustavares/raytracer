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
};

int ray_intersects_sphere(struct ray *r, struct sphere *s, struct intersection *it);
int ray_intersects_plane(struct ray *r, struct plane *p, struct intersection *it);
