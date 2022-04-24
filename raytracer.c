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
	ENTITY_SPHERE(vec3_new(-3, 0, 6), 1, MAT_REFLECTIVE(vec3_new(0, .5, .5), 0.5)),
	ENTITY_SPHERE(vec3_new(0, 0, 6), 1, MAT_REFLECTIVE(vec3_new(0, 0, 1), 0.5)),
	ENTITY_SPHERE(vec3_new(0.2, 0.2, .5), .2, MAT_REFLECTIVE(vec3_new(0, 1, 0), 0.5)),
	ENTITY_PLANE(vec3_new(0, -1, 0), vec3_new(0, 1, 0), MAT_REFLECTIVE(vec3_new(0.977, 0.627, 0.392), 0.5)),
};

struct light lights[] = {
	{.pos={.x=3, .y=2, .z=-1}, .intensity=1},
	/*
	 * NEEDSWORK: I only added this second light because the current shadow
	 * implementation shuts off the pixels that are not directly visible by
	 * a light source (i.e. it does not consider any reflection). This
	 * gives a weird effect on reflective materials. Instead, I think we
	 * should just darken the pixels, but considering reflectiveness.
	 */
	{.pos={.x=0, .y=0, .z=0}, .intensity=.001},
};

struct vec3 background_color = {.24, .24, .24};

#define RAY_RECUSION_LIMIT 4

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

void cast_ray_and_color_pixel(struct ray *r, struct vec3 *color,
			      int recursion_limit);

#define AMBIENT_LIGHT_INTENSITY 0.08

struct vec3 intersection_color(struct intersection *it, struct vec3 ray_dir,
			       int recursion_limit)
{
	float diffuse_light_intensity = AMBIENT_LIGHT_INTENSITY;
	float specular_light_intensity = 0;
	struct material *material = &it->entity->material;
	struct vec3 reflect_color;
	int reflected = 0;

	for (int i = 0; i < ARRAY_SIZE(lights); i++) {
		struct light *l = &lights[i];
		float light_dist = vec3_norm(vec3_sub(l->pos, it->pos));
		struct vec3 it_to_light_dir = vec3_normalize(vec3_sub(l->pos, it->pos));

		/* Shadow */
		struct vec3 displaced_it_pos = vec3_add(it->pos, vec3_smul(it->normal, 1e-3));
		struct ray shadow_ray = ray_new(displaced_it_pos, it_to_light_dir);
		if (cast_ray(&shadow_ray, light_dist, NULL))
			continue;

		/* Reflection */
		if (recursion_limit && material->reflectiveness) {
			struct vec3 reflect_dir = vec3_reflect(ray_dir, it->normal);
			struct ray reflect_ray = ray_new(displaced_it_pos, reflect_dir);
			cast_ray_and_color_pixel(&reflect_ray, &reflect_color,
						 recursion_limit - 1);
			reflected = 1;
		}

		/*
		 * If the dot product is below zero it means the angle is
		 * above 90°, so the light is hitting the back of the object.
		 */
		diffuse_light_intensity += l->intensity * max(0,
			vec3_dot(vec3_normalize(vec3_sub(l->pos, it->pos)),
				 it->normal));

		/* Specular component */
		/*
		 * TODO: should really use vec3_smul(ray_dir, -1)?
		 */
		float specular_light_incidence = max(0, vec3_dot(
			vec3_normalize(vec3_reflect(vec3_smul(it_to_light_dir, -1), it->normal)),
			vec3_normalize(vec3_smul(ray_dir, -1))));

		specular_light_intensity +=
			powf(specular_light_incidence * l->intensity,
			     material->shininess);
	}

	diffuse_light_intensity = clamp_color(diffuse_light_intensity);
	struct vec3 diffuse_color = vec3_smul(material->color,
					      diffuse_light_intensity *
					      material->diffuse_constant);

	specular_light_intensity = clamp_color(specular_light_intensity);
	struct vec3 specular_color =
		vec3_smul((struct vec3){1, 1, 1},
			  specular_light_intensity * material->specular_constant);

	struct vec3 this_color = vec3_add(diffuse_color, specular_color);

	if (reflected)
		return vec3_add(vec3_smul(this_color, 1 - material->reflectiveness),
				vec3_smul(reflect_color, material->reflectiveness));
	return this_color;
}

void cast_ray_and_color_pixel(struct ray *r, struct vec3 *color,
			      int recursion_limit)
{
	struct intersection it;
	if (cast_ray(r, INFINITY, &it))
		*color = intersection_color(&it, r->dir, recursion_limit);
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

#define ASPECT_RATIO (16.0 / 9.0)
#define OUTPUT_WIDTH 2048
#define RENDER_RESOLUTION 0.5 /* ]0,1] */
#define SAMPLES_PER_PIXEL 4

/*
 * Viewport is centered at 0, 0, 1 and directed towards positive z. The left
 * top corner of the image is the origin (0, 0), and the axis grow towards
 * the right bottom corner.
 */
int main(int argc, char **argv)
{
	int W = OUTPUT_WIDTH * RENDER_RESOLUTION, H = W / ASPECT_RATIO;

	float viewport_W = 2.0;
	float viewport_H = viewport_W / ASPECT_RATIO;
	float pixel_sz = viewport_W / W;

	ensure_unit_length_in_scene_normals();

	struct ppm *ppm = ppm_new(H, W);
	for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++) {
			struct vec3 samples[SAMPLES_PER_PIXEL];
			float top_x = -viewport_W/2 + j * pixel_sz;
			float top_y = viewport_H/2 - i * pixel_sz;
			for (int s = 0; s < SAMPLES_PER_PIXEL; s++) {
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
				cast_ray_and_color_pixel(&r, color, RAY_RECUSION_LIMIT);
			}
			*ppm_color(ppm, i, j) = color_average(samples, SAMPLES_PER_PIXEL);
		}
	}
	ppm_resize(ppm, OUTPUT_WIDTH / ASPECT_RATIO, OUTPUT_WIDTH);
	ppm_write(ppm, stdout);
	ppm_destroy(&ppm);
	return 0;
}
