#pragma once

#include <stdio.h>
#include <assert.h>

struct color {
	unsigned char R, G, B;
};

struct ppm {
	unsigned cols, rows;
	struct color *img;
};

static inline void copy_color(struct color *to, struct color *from)
{
	to->R = from->R;
	to->G = from->G;
	to->B = from->B;
}

static inline void set_color_intensity(struct color *c, float intensity)
{
	assert(intensity >= 0 && intensity <= 1);
	c->R *= intensity;
	c->G *= intensity;
	c->B *= intensity;
}

struct ppm *ppm_new(unsigned W, unsigned H);
void ppm_destroy(struct ppm **ppm_ptr);

struct color *ppm_color(struct ppm *ppm, unsigned i, unsigned j);
void ppm_write(struct ppm *ppm, FILE *f);

unsigned ppm_2d_to_1d(struct ppm *ppm, unsigned i, unsigned j);
