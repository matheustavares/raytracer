#include <assert.h>
#include <math.h>
#include "entities.h"
#include "../util.h"

/*
 * Formula from https://www.rose-hulman.edu/class/csse/csse451/examples/notes/present7.pdf#page=2
 * The variable nomenclature is the same used in that PDF.
 * With negative distance checking idea from:
 * https://github.com/ssloy/tinyraytracer/blob/ce6b785 /tinyraytracer.cpp#L67
 */
int ray_intersects_sphere(struct ray *r, struct entity *e, struct intersection *it)
{
	assert(e->type == ENT_SPHERE);
	struct sphere *s = &e->u.s;

	struct vec3 ec = vec3_sub(s->center, r->pos);
	float ec_dot_d = vec3_dot(ec, r->dir);
	float det = square(s->radius) - vec3_square(ec) + square(ec_dot_d);
	if (det < 0)
		return 0;

	float sqrt_det = sqrt(det);
	float dist1 = ec_dot_d - sqrt_det;
	float dist2 = ec_dot_d + sqrt_det;

	/*
	 * If the distance is negative, it means the ray intersects the sphere
	 * if "going backwards". We do not want to consider such intersections. 
	 * 
	 * Furthermore, it may seem like a desirable optimization to only check
	 * the first point (... - sqrt(...)), since it should always be smaller
	 * than the second one. However, consider the case where the ray
	 * originates inside the sphere. The first point would be a "backwards"
	 * intersection, but the second will be a valid forward intersection.
	 * So we must check both.
	 */
	if (dist1 > 0)
		it->dist = dist1;
	else if (dist2 > 0)
		it->dist = dist2;
	else
		return 0;

	it->pos = vec3_add(r->pos, vec3_smul(r->dir, it->dist));
	/* Is the ray intersecting from inside or outside? */
	if (vec3_norm(ec) > s->radius)
		it->normal = vec3_normalize(vec3_sub(it->pos, s->center));
	else
		it->normal = vec3_normalize(vec3_sub(s->center, it->pos));
	it->entity = e;
	return 1;
}


struct vec3 lookup_sphere_texture(struct entity *e, struct vec3 pos)
{
	assert(e->type == ENT_SPHERE);
	struct sphere *s = &e->u.s;

	struct texture *texture = e->material.texture;
	assert(texture);

	struct vec3 normalized_pos = vec3_normalize(vec3_sub(pos, s->center));
	/*
	 * The "(atan() + 2*pi) % 2*pi" formula comes from:
	 * https://stackoverflow.com/a/25725005/11019779
	 * The idea is to map the atan() values into [0, 2*pi] radians. We need
	 * the +2*pi because fmod(x, 2*pi) returns negative values for a
	 * negative x.
	 */
	float u = fmod((atan2f(normalized_pos.z, normalized_pos.x) + 2*M_PI), 2*M_PI) / (2*M_PI);
	float v = (1 - normalized_pos.y) / 2.0;

	/* Filter method: nearest pixel. */
	int u_int = roundf(u * (texture->W - 1));
	int v_int = roundf(v * (texture->H - 1));

	return texture_color(texture, u_int, v_int);
}
