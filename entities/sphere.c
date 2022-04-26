#include <assert.h>
#include <math.h>
#include "entities.h"

/*
 * Algorithm from:
 * http://www.lighthouse3d.com/tutorials/maths/ray-sphere-intersection/
 * Note: this function assumes that the ray does not originate inside the
 * sphere.
 */
int ray_intersects_sphere(struct ray *r, struct entity *e, struct intersection *it)
{
	assert(e->type == ENT_SPHERE);
	struct sphere *s = &e->u.s;

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
		it->entity = e;
		return 1;
	} else {
		return 0;
	}
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
	int u_int = fmod(roundf(u * texture->W), texture->W);
	int v_int = fmod(roundf(v * texture->H), texture->H);

	return texture_color(texture, u_int, v_int);
}
