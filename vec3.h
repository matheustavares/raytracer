#pragma once
#include <math.h>

struct vec3 {
	float x, y, z;
};

#define vec3_new(vx, vy, vz) (struct vec3){.x=(vx), .y=(vy), .z=(vz)}

static inline struct vec3 vec3_add(struct vec3 v, struct vec3 u)
{
	return vec3_new(v.x + u.x, v.y + u.y, v.z + u.z);
}

static inline struct vec3 vec3_sub(struct vec3 v, struct vec3 u)
{
	return vec3_new(v.x - u.x, v.y - u.y, v.z - u.z);
}

static inline float vec3_dot(struct vec3 v, struct vec3 u)
{
	return v.x * u.x + v.y * u.y + v.z * u.z;
}

static inline float vec3_norm(struct vec3 v)
{
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

static inline struct vec3 vec3_smul(struct vec3 v, float s)
{
	return vec3_new(v.x * s, v.y * s, v.z * s);
}

