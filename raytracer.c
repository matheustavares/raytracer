#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ppm.h"
#include "vec3.h"
#include "lib/error.h"

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
	struct color color;
};

struct entity scene[] = {
	{.type=ENT_SPHERE, .u={.s={.center={.x=0, .y=0, .z=6}, .radius=1}}, .color={.R=0, .G=0, .B=255}},
	{.type=ENT_SPHERE, .u={.s={.center={.x=0.2, .y=0.2, .z=.5}, .radius=.2}}, .color={.R=0, .G=255, .B=0}},
	{.type=ENT_PLANE, .u={.p={.p0={.x=0, .y=-1, .z=0}, .normal={.x=0, .y=1, .z=0}}}, .color={.R=220, .G=160, .B=100}},
};

struct light lights[] = {
	{.pos={.x=.5, .y=.5, .z=-1}, .intensity=1},
};

struct color background_color = {60, 60, 60};

struct ray {
	struct vec3 pos, dir;
};

#define ray_new(px, py, pz, dx, dy, dz) \
	{.pos={.x=px, .y=py, .z=pz}, .dir={.x=dx, .y=dy, .z=dz}}

struct intersection {
	struct vec3 pos, normal;
	float dist;
};

/*
 * Algorithm from:
 * http://www.lighthouse3d.com/tutorials/maths/ray-sphere-intersection/
 * Note: this function assumes that the ray does not originate inside the
 * sphere.
 */
int ray_intersects_sphere(struct ray *r, struct sphere *s, struct intersection *it)
{
	/* Ray must originate outside the sphere */
	assert(vec3_norm(vec3_sub(s->center, r->pos)) > s->radius);

	float d = vec3_dot(r->dir, vec3_sub(s->center, r->pos));
	if (d > 0) {
		struct vec3 proj = vec3_add(r->pos,
				vec3_smul(r->dir, d / vec3_norm(r->dir)));
		float dist = vec3_norm(vec3_sub(proj, s->center));
		if (dist > s->radius)
			return 0;
		if (dist == s->radius) {
			it->pos = proj;
			it->dist = vec3_norm(vec3_sub(proj, r->pos));
		} else {
			float n = vec3_norm(vec3_sub(proj, s->center));
			it->dist = vec3_norm(vec3_sub(proj, r->pos)) -
					sqrt(s->radius * s->radius - n * n);
			it->pos = vec3_add(r->pos, vec3_smul(r->dir, it->dist));
		}
		it->normal = vec3_normalize(vec3_sub(it->pos, s->center));
		return 1;
	} else {
		return 0;
	}
}

int ray_intersects_plane(struct ray *r, struct plane *p, struct intersection *it)
{
	float divisor = vec3_dot(r->dir, p->normal);
	if (fabs(divisor) < 0.001)
		return 0;
	float dist = vec3_dot(vec3_sub(p->p0, r->pos), p->normal) / divisor;
	if (dist < 0)
		return 0;
	it->dist = dist;
	it->pos = vec3_add(r->pos, vec3_smul(r->dir, dist));
	it->normal = p->normal;
	return 1;
}

#define max(a, b) ({ \
		typeof(a) _a = (a); \
		typeof(b) _b = (b); \
		_a > _b ? _a : _b; \
})

float illumination_intensity(struct vec3 pos, struct vec3 normal)
{
	float ret = 0;
	normal = vec3_normalize(normal);
	size_t nr_lights = sizeof(lights) / sizeof(lights[0]);
	for (int i = 0; i < nr_lights; i++) {
		struct light *l = &lights[i];
		/*
		 * If the dot product is below zero it means the angle is
		 * above 90°, so the light is hitting the back of the object.
		 */
		float illumination = max(0,
			vec3_dot(vec3_normalize(vec3_sub(l->pos, pos)), normal));
		ret += illumination * l->intensity;
	}
	return ret > 1 ? 1 : ret;
}

void cast_ray(struct ray *r, struct color *c)
{
	struct intersection nearest = {.dist=INFINITY};
	struct entity *nearest_entity = NULL;

	size_t entities = sizeof(scene) / sizeof(scene[0]);
	for (int i = 0; i < entities; i++) {
		struct intersection it;
		int intersects;
		switch (scene[i].type) {
		case ENT_SPHERE:
			intersects = ray_intersects_sphere(r, &scene[i].u.s, &it);
			break;
		case ENT_PLANE:
			intersects = ray_intersects_plane(r, &scene[i].u.p, &it);
			break;
		default:
			die("unknown type %d", scene[i].type);
		}
		if (!intersects)
			continue;
		if (it.dist < nearest.dist) {
			nearest = it;
			nearest_entity = &scene[i];
		}
	}
	if (nearest_entity) {
		float intensity = illumination_intensity(nearest.pos, nearest.normal);
		copy_color(c, &nearest_entity->color);
		set_color_intensity(c, intensity);
	} else {
		copy_color(c, &background_color);
	}
}

/*
 * Viewport is centered at 0, 0, 1 and directed towards positive z.
 */
int main(int argc, char **argv)
{
	float aspect_ratio = 16.0 / 9.0;
	int W = 1024, H = W / aspect_ratio;

	float viewport_W = 2.0;
	float viewport_H = viewport_W / aspect_ratio;

	struct ppm *ppm = ppm_new(H, W);
	for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++) {
			float x = -viewport_W/2 + j * (viewport_W / (W-1));
			float y = viewport_H/2 - i * (viewport_H / (H-1));
#ifdef CAN_PROJ_ORTO
			struct ray r = ray_new(x, y, 0, 0, 0, 1);
#else
			struct ray r = ray_new(0, 0, 0, x, y, 1);
#endif
			r.dir = vec3_normalize(r.dir);
			struct color *c = ppm_color(ppm, i, j);
			cast_ray(&r, c);
		}
	}
	ppm_write(ppm, stdout);
	ppm_destroy(&ppm);
	return 0;
}
