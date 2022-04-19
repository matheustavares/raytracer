#include "entities.h"

int ray_intersects_plane(struct ray *r, struct entity *e, struct intersection *it)
{
	assert(e->type == ENT_PLANE);
	struct plane *p = &e->u.p;

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
