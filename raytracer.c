#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ppm.h"
#include "vec3.h"
#include "lib/error.h"
#include "util.h"
#include "entities/entities.h"
#include "ray.h"
#include "lib/array.h"

struct entity scene[] = {
	ENTITY_SPHERE(vec3_new(0, 0, 6), 1, vec3_new(0, 0, 1)),
	ENTITY_SPHERE(vec3_new(0.2, 0.2, .5), .2, vec3_new(0, 1, 0)),
	ENTITY_PLANE(vec3_new(0, -1, 0), vec3_new(0, 1, 0), vec3_new(0.977, 0.627, 0.392)),
};

struct light lights[] = {
	{.pos={.x=3, .y=2, .z=-1}, .intensity=1},
};

struct vec3 background_color = {.24, .24, .24};

/*
 * Returns 1 if the ray intersect any scene object or 0 otherwise.  If
 * `nearest_it` is not NULL, the data for the nearest intersection is saved
 * on it. Otherwise, the function returns early at the first intersection.
 */
int cast_ray(struct ray *r, float limit, struct intersection *nearest_it)
{
	int ret = 0;
	if (nearest_it)
		nearest_it->dist = INFINITY;

	for (int i = 0; i < ARRAY_SIZE(scene); i++) {
		struct entity *e = &scene[i];
		struct intersection this_it;
		if (!e->ray_intersects(r, e, &this_it) || this_it.dist > limit)
			continue;
		if (!nearest_it)
			return 1;
		if (this_it.dist < nearest_it->dist) {
			*nearest_it = this_it;
			ret = 1;
		}
	}
	return ret;
}

#define AMBIENT_ILLUMINATION 0.08

struct vec3 intersection_color(struct intersection *it, struct vec3 cam_dir)
{
	struct vec3 color = it->entity->color;
	float illumination = AMBIENT_ILLUMINATION;
	float specular = 0;

	for (int i = 0; i < ARRAY_SIZE(lights); i++) {
		struct light *l = &lights[i];
		float light_dist = vec3_norm(vec3_sub(l->pos, it->pos));
		struct vec3 it_to_light_dir = vec3_normalize(vec3_sub(l->pos, it->pos));

		/* Shadow */
		struct vec3 displaced_it_pos = vec3_add(it->pos, vec3_smul(it->normal, 1e-3));
		struct ray shadow_ray = ray_new(displaced_it_pos, it_to_light_dir);
		if (cast_ray(&shadow_ray, light_dist, NULL))
			continue;

		/*
		 * If the dot product is below zero it means the angle is
		 * above 90°, so the light is hitting the back of the object.
		 */
		illumination += l->intensity * max(0,
			vec3_dot(vec3_normalize(vec3_sub(l->pos, it->pos)),
				 it->normal));

		/* Specular component */
		float spec_light = vec3_dot(
			vec3_normalize(vec3_reflect(vec3_smul(it_to_light_dir, -1), it->normal)),
			vec3_normalize(cam_dir));

		spec_light = max(0, spec_light);
		specular += powf(spec_light * l->intensity, 200);
	}
	illumination = illumination > 1 ? 1 : illumination;
	color = vec3_smul(color, illumination);

	specular = specular > 1 ? 1 : specular;
	struct vec3 spec_color = {specular, specular, specular};
	color = vec3_add(color, spec_color);
	return color;
}

void cast_ray_and_color_pixel(struct ray *r, struct vec3 *color)
{
	struct intersection it;
	if (cast_ray(r, INFINITY, &it))
		*color = intersection_color(&it, vec3_smul(r->dir, -1));
	else
		*color = background_color;
}

static void ensure_unit_length_in_scene_normals(void)
{
	for (int i = 0; i < ARRAY_SIZE(scene); i++) {
		struct entity *e = &scene[i];
		if (e->type == ENT_PLANE)
			vec3_normalize_inplace(e->u.p.normal);
	}
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

	ensure_unit_length_in_scene_normals();

	struct ppm *ppm = ppm_new(H, W);
	for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++) {
			struct vec3 samples[NR_SAMPLES];
			float top_x = -viewport_W/2 + j * pixel_sz;
			float top_y = viewport_H/2 - i * pixel_sz;
			for (int s = 0; s < NR_SAMPLES; s++) {
				struct vec3 *color = &samples[s];
				float x = rand_in(top_x, top_x + pixel_sz);
				float y = rand_in(top_y, top_y + pixel_sz);

#ifdef CAN_PROJ_ORTO
				struct ray r = ray_new(vec3_new(x, y, 0),
						       vec3_new(0, 0, 1));
#else
				struct ray r = ray_new(vec3_new(0, 0, 0),
						       vec3_new(x, y, 1));
#endif
				cast_ray_and_color_pixel(&r, color);
			}
			*ppm_color(ppm, i, j) = color_average(samples, NR_SAMPLES);
		}
	}
	ppm_write(ppm, stdout);
	ppm_destroy(&ppm);
	return 0;
}
