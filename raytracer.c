#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ppm.h"
#include "vec3.h"

struct sphere {
	struct vec3 center;
	float radius;
};

struct entity {
	enum entity_type{
		ENT_SPHERE
	} type;
	union {
		struct sphere s;
	} u;
	struct color color;
};

struct entity scene[] = {
	{.type=ENT_SPHERE, .u={.s={.center={.x=0, .y=0, .z=6}, .radius=1}}, .color={.R=0, .G=0, .B=255}},
	{.type=ENT_SPHERE, .u={.s={.center={.x=0.2, .y=0.2, .z=.3}, .radius=.2}}, .color={.R=0, .G=255, .B=0}},
};

struct color background_color = {60, 60, 60};

struct ray {
	struct vec3 pos, dir;
};

#define ray_new(px, py, pz, dx, dy, dz) \
	{.pos={.x=px, .y=py, .z=pz}, .dir={.x=dx, .y=dy, .z=dz}}

/*
 * Calculates whether the ray r intersects the sphere s and returns the
 * distance between r's origin and the intersection point or -1 they do not
 * intersect.
 * 
 * Algorithm from:
 * http://www.lighthouse3d.com/tutorials/maths/ray-sphere-intersection/
 * Note: this function assumes that the ray does not originate inside the
 * sphere.
 */
float ray_intersects_sphere(struct ray *r, struct sphere *s)
{
	/* Ray must originate outside the sphere */
	assert(vec3_norm(vec3_sub(s->center, r->pos)) > s->radius);

	float d = vec3_dot(r->dir, vec3_sub(s->center, r->pos));
	if (d > 0) {
		struct vec3 proj = vec3_add(r->pos,
				vec3_smul(r->dir, d / vec3_norm(r->dir)));
		float dist = vec3_norm(vec3_sub(proj, s->center));
		if (dist > s->radius)
			return -1;
		if (dist == s->radius)
			return vec3_norm(proj);
		float n = vec3_norm(vec3_sub(proj, s->center)); 
		return vec3_norm(vec3_sub(proj, r->pos)) -
				sqrt(s->radius * s->radius - n * n);
	} else {
		return -1;
	}
}

void cast_ray(struct ray *r, struct color *c)
{
	float min_dist = INFINITY;
	struct entity *nearest = NULL;

	size_t entities = sizeof(scene) / sizeof(scene[0]);
	for (int i = 0; i < entities; i++) {
		switch (scene[i].type) {
		case ENT_SPHERE:
			float dist = ray_intersects_sphere(r, &scene[i].u.s);
			if (dist < 0)
				continue;
			if (dist < min_dist) {
				min_dist = dist;
				nearest = &scene[i];
			}
			break;
		default:
			die("unknown type %d", scene[i].type);
		}
	}

	if (nearest)
		copy_color(c, &nearest->color);
	else
		copy_color(c, &background_color);
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
