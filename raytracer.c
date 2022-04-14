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
	{.pos={.x=3, .y=2, .z=-1}, .intensity=1},
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
	struct entity *entity;
};

/*
 * Algorithm from:
 * http://www.lighthouse3d.com/tutorials/maths/ray-sphere-intersection/
 * Note: this function assumes that the ray does not originate inside the
 * sphere.
 */
int ray_intersects_sphere(struct ray *r, struct sphere *s, struct intersection *it)
{
	/* Ray must originate outside the sphere. FIXME */
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
	if (divisor >= 0)
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

/*
 * Returns 1 if the ray intersect any scene object or 0 otherwise.  If
 * `nearest_inter` is not NULL, the data for the nearest intersection is saved
 * on it. Otherwise, the function returns early at the first intersection.
 */
int cast_ray(struct ray *r, float limit, struct intersection *nearest_inter)
{
	int ret = 0;
	if (nearest_inter)
		nearest_inter->dist = INFINITY;

	size_t nr_entities = sizeof(scene) / sizeof(scene[0]);
	for (int i = 0; i < nr_entities; i++) {
		int intersects;
		struct intersection this_inter;
		switch (scene[i].type) {
		case ENT_SPHERE:
			intersects = ray_intersects_sphere(r, &scene[i].u.s, &this_inter);
			break;
		case ENT_PLANE:
			intersects = ray_intersects_plane(r, &scene[i].u.p, &this_inter);
			break;
		default:
			die("unknown type %d", scene[i].type);
		}
		if (!intersects || this_inter.dist > limit)
			continue;
		if (!nearest_inter)
			return 1;
		if (this_inter.dist < nearest_inter->dist) {
			*nearest_inter = this_inter;
			nearest_inter->entity = &scene[i];
			ret = 1;
		}
	}
	return ret;
}

#define AMBIENT_ILLUMINATION 0.08

void color_pixel(struct color *c, struct vec3 pos, struct vec3 normal,
		 struct vec3 view_dir)
{
	float illumination = AMBIENT_ILLUMINATION;
	float specular = 0;
	normal = vec3_normalize(normal);
	size_t nr_lights = sizeof(lights) / sizeof(lights[0]);
	for (int i = 0; i < nr_lights; i++) {
		struct light *l = &lights[i];
		float light_dist = vec3_norm(vec3_sub(l->pos, pos));

		struct vec3 displaced_pos = vec3_add(pos, vec3_smul(normal, 1e-3));
		struct vec3 dir = vec3_normalize(vec3_sub(l->pos, pos));
		struct ray r = ray_new(displaced_pos.x, displaced_pos.y, displaced_pos.z, dir.x, dir.y, dir.z);

		if (cast_ray(&r, light_dist, NULL))
			continue;

		/*
		 * If the dot product is below zero it means the angle is
		 * above 90°, so the light is hitting the back of the object.
		 */
		illumination += l->intensity * max(0,
			vec3_dot(vec3_normalize(vec3_sub(l->pos, pos)), normal));

		/* Specular component */
		float spec_light = vec3_dot(
				vec3_normalize(vec3_reflect(vec3_smul(dir, -1), normal)),
				vec3_normalize(view_dir));
		spec_light = max(0, spec_light);
		specular += powf(spec_light * l->intensity, 200);
	}
	illumination = illumination > 1 ? 1 : illumination;
	set_color_intensity(c, illumination);

	specular = specular > 1 ? 1 : specular;
	struct color spec_color = {255, 255, 255};
	set_color_intensity(&spec_color, specular);
	color_add(c, spec_color);
}

void cast_ray_and_color_pixel(struct ray *r, struct color *c)
{
	struct intersection inter;
	if (cast_ray(r, INFINITY, &inter)) {
		copy_color(c, inter.entity->color);
		color_pixel(c, inter.pos, inter.normal, vec3_smul(r->dir, -1));
	} else {
		copy_color(c, background_color);
	}
}

float rand_in(float a, float b)
{
	assert(a <= b);
	return a + (((float)rand() / RAND_MAX) * (b - a));
}

/*
 * Viewport is centered at 0, 0, 1 and directed towards positive z. The left
 * top corner of the image is the origin (0, 0), and the axis grow towards
 * the right bottom corner.
 */
int main(int argc, char **argv)
{
	float aspect_ratio = 16.0 / 9.0;
	int W = 2048, H = W / aspect_ratio;

	float viewport_W = 2.0;
	float viewport_H = viewport_W / aspect_ratio;
	int NR_SAMPLES = 4;
	float pixel_sz = viewport_W / W;

	struct ppm *ppm = ppm_new(H, W);
	for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++) {
			struct color samples[NR_SAMPLES];
			float top_x = -viewport_W/2 + j * pixel_sz;
			float top_y = viewport_H/2 - i * pixel_sz;
			for (int s = 0; s < NR_SAMPLES; s++) {
				struct color *c = &samples[s];
				float x = rand_in(top_x, top_x + pixel_sz);
				float y = rand_in(top_y, top_y + pixel_sz);

#ifdef CAN_PROJ_ORTO
				struct ray r = ray_new(x, y, 0, 0, 0, 1);
#else
				struct ray r = ray_new(0, 0, 0, x, y, 1);
#endif
				r.dir = vec3_normalize(r.dir);
				cast_ray_and_color_pixel(&r, c);
			}
			copy_color(ppm_color(ppm, i, j),
				   color_average(samples, NR_SAMPLES));
		}
	}
	ppm_write(ppm, stdout);
	ppm_destroy(&ppm);
	return 0;
}
