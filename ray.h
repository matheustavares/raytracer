#pragma once

struct ray {
	struct vec3 pos, dir;
};

#define ray_new(px, py, pz, dx, dy, dz) \
	{.pos={.x=px, .y=py, .z=pz}, .dir={.x=dx, .y=dy, .z=dz}}

/* The intersection of a ray and an entity. */
struct intersection {
	struct vec3 pos, normal;
	float dist;
	struct entity *entity;
};
