#pragma once

struct ray {
	struct vec3 pos, dir;
};

static inline struct ray ray_new(struct vec3 pos, struct vec3 dir)
{
	struct ray r = {.pos=pos, .dir=vec3_normalize(dir)};
	return r;
}

/* The intersection of a ray and an entity. */
struct intersection {
	struct vec3 pos, normal;
	float dist;
	struct entity *entity;
};
