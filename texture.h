#pragma once

#include "lib/error.h"
#include "vec3.h"

struct texture_opts {
	int invert_X:1,
	    invert_Y:1;
	int rotate_X; /* in pixels */
};

struct texture {
	struct vec3 *data;
	int W, H;
	struct texture_opts opts;
};

struct texture *load_texture(const char *name, struct texture_opts *opts);
void free_textures(void);

static inline unsigned texture_2d_to_1d(struct texture *t, unsigned u, unsigned v)
{
	if (u >= t->W || v >= t->H)
		die("invalid index (%u, %u) for texture of size (%u, %u)",
		    u, v, t->W, t->H);

	u = (u + t->opts.rotate_X) % t->W;

	u = t->opts.invert_X ? t->W - 1 - u : u;
	v = t->opts.invert_Y ? t->H - 1 - v : v;

	return v * t->W + u;
}

static inline struct vec3 texture_color(struct texture *t, unsigned u, unsigned v)
{
	return t->data[texture_2d_to_1d(t, u, v)];
}
