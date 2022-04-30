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
#include "texture.h"

ARRAY(struct entity) scene;
#define ADD_ENTITY(e) ARRAY_APPEND(&scene, (e));

struct light lights[] = {
	{.pos={.x=3, .y=2, .z=-1}, .intensity=1},
	/*
	 * NEEDSWORK: I only added this second light because the current shadow
	 * implementation shuts off the pixels that are not directly visible by
	 * a light source (i.e. it does not consider any reflection). This
	 * gives a weird effect on reflective materials. Instead, I think we
	 * should just darken the pixels, but considering reflectiveness.
	 */
	{.pos={.x=0, .y=0, .z=0}, .intensity=1},
};

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

	for (int i = 0; i < scene.nr; i++) {
		struct entity *e = &scene.arr[i];
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
		float displacement = sign(vec3_dot(it_to_light_dir, it->normal)) * 1e-3;
		struct vec3 displaced_it_pos = vec3_add(it->pos, vec3_smul(it->normal, displacement));
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

		diffuse_light_intensity += l->intensity * fabsf(
			vec3_dot(vec3_normalize(vec3_sub(l->pos, it->pos)),
				 it->normal));

		/* Specular component */
		/*
		 * TODO: should really use vec3_smul(ray_dir, -1)?
		 */
		float specular_light_incidence = fabsf(vec3_dot(
			vec3_normalize(vec3_reflect(vec3_smul(it_to_light_dir, -1), it->normal)),
			vec3_normalize(vec3_smul(ray_dir, -1))));

		specular_light_intensity +=
			powf(specular_light_incidence * l->intensity,
			     material->shininess);
	}

	struct vec3 base_color = material->texture ?
				it->entity->lookup_texture(it->entity, it->pos) :
				material->color;

	diffuse_light_intensity = clamp_color(diffuse_light_intensity);
	struct vec3 diffuse_color = vec3_smul(base_color,
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

struct entity background_map;

void cast_ray_and_color_pixel(struct ray *r, struct vec3 *color,
			      int recursion_limit)
{
	struct intersection it;
	if (cast_ray(r, INFINITY, &it))
		*color = intersection_color(&it, r->dir, recursion_limit);
	else
		*color = lookup_sphere_texture(&background_map, r->dir);
}

static void ensure_unit_length_in_scene_normals(void)
{
	for (int i = 0; i < scene.nr; i++) {
		struct entity *e = &scene.arr[i];
		if (e->type == ENT_PLANE)
			vec3_normalize_inplace(e->u.p.normal);
	}
}

#define ASPECT_RATIO (16.0 / 9.0)
#define OUTPUT_WIDTH 2048
#define RENDER_RESOLUTION 1 /* ]0,1] */
#define SAMPLES_PER_PIXEL 4
#define VIEWPOINT_DIST 1

void make_scene(void)
{
	struct texture_opts opts = {.rotate_X = -300};
	struct texture *env = load_texture("neon-studio.jpg", &opts);
	struct texture *tiles = load_texture("tiles.png", NULL);
	ADD_ENTITY(ENTITY_SPHERE(vec3_new(-2.5, -.5, 6), 1.2,
				 MAT_MATTE_T(tiles)));
	ADD_ENTITY(ENTITY_SPHERE(vec3_new(0, 0, 6), 1,
				 MAT_REFLECTIVE(vec3_new(0, 0, 1), 0.5)));
	ADD_ENTITY(ENTITY_SPHERE(vec3_new(0.2, 0.2, .5), .2,
				 MAT_REFLECTIVE(vec3_new(0, 1, 0), 0.5)));
	ensure_unit_length_in_scene_normals();
	background_map = ENTITY_SPHERE(vec3_new(0, 0, 0), 50, MAT_MATTE_T(env));
}

/*
 * Viewport is a 2 by 2 plane (in word coordinates), centered at
 * (0, 0, VIEWPOINT_DIST). VIEWPOINT_DIST indirectly defines the field of view.
 */
int main(int argc, char **argv)
{
	int W = OUTPUT_WIDTH * RENDER_RESOLUTION, H = W / ASPECT_RATIO;

	float viewport_W = 2.0;
	float viewport_H = viewport_W / ASPECT_RATIO;
	float pixel_sz = viewport_W / W;

	fprintf(stderr, "Loading resources...\n");
	make_scene();
	struct ppm *ppm = ppm_new(H, W);

	fprintf(stderr, "Casting rays...\n");
	#pragma omp parallel for collapse(2)
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
						       vec3_new(0, 0, VIEWPOINT_DIST));
#else
				struct ray r = ray_new(vec3_new(0, 0, 0),
						       vec3_new(x, y, VIEWPOINT_DIST));
#endif
				cast_ray_and_color_pixel(&r, color, RAY_RECUSION_LIMIT);
			}
			*ppm_color(ppm, i, j) = color_average(samples, SAMPLES_PER_PIXEL);
		}
	}
	fprintf(stderr, "Resizing...\n");
	ppm_resize(ppm, OUTPUT_WIDTH / ASPECT_RATIO, OUTPUT_WIDTH);

	fprintf(stderr, "Writing...\n");
	ppm_write(ppm, stdout);
	ppm_destroy(&ppm);
	free_textures();

	fprintf(stderr, "Done!\n");
	return 0;
}
