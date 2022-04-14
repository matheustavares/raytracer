#include <assert.h>
#include "entities.h"

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
