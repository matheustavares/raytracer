#pragma once
#include <math.h>

struct vec3 {
	float x, y, z;
};

#define vec3_new(vx, vy, vz) ((struct vec3){.x=(vx), .y=(vy), .z=(vz)})

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

static inline struct vec3 vec3_sdiv(struct vec3 v, float s)
{
	return vec3_smul(v, 1.0/s);
}

static inline struct vec3 vec3_normalize(struct vec3 v)
{
	return vec3_sdiv(v, vec3_norm(v));
}

#define vec3_normalize_inplace(v) { \
	(v) = vec3_normalize(v); \
} while(0)

static inline struct vec3 vec3_reflect(struct vec3 d, struct vec3 n)
{
	n = vec3_normalize(n);
	return vec3_sub(d, vec3_smul(n, 2 * vec3_dot(d, n)));
}

static inline struct vec3 vec3_map(struct vec3 v, float (*map_fn)(float f))
{
	return vec3_new(map_fn(v.x), map_fn(v.y), map_fn(v.z));
}
