#pragma once

#include <stdio.h>
#include <assert.h>
#include "vec3.h"

struct color {
	unsigned char R, G, B;
};

struct ppm {
	unsigned cols, rows;
	struct color *img;
};

static inline void copy_color(struct color *to, struct color from)
{
	to->R = from.R;
	to->G = from.G;
	to->B = from.B;
}

static inline void set_color_intensity(struct color *c, float intensity)
{
	assert(intensity >= 0 && intensity <= 1);
	c->R *= intensity;
	c->G *= intensity;
	c->B *= intensity;
}

#define clamp(a) ((a) > 255 ? 255 : (a))
#define add_and_clamp(a, b) clamp((int)(a) + (int)(b))

static inline void color_add(struct color *c1, struct color c2)
{
	c1->R = add_and_clamp(c1->R, c2.R);
	c1->G = add_and_clamp(c1->G, c2.G);
	c1->B = add_and_clamp(c1->B, c2.B);
}

static struct color color_average(struct color *colors, int size)
{
	struct vec3 float_average = {0};
	for (int i = 0; i < size; i++) {
		float_average.x += colors[i].R;
		float_average.y += colors[i].G;
		float_average.z += colors[i].B;
	}
	float_average.x /= size;
	float_average.y /= size;
	float_average.z /= size;
	struct color average_color = {
		.R = clamp(float_average.x),
		.G = clamp(float_average.y),
		.B = clamp(float_average.z) };
	return average_color;
}

struct ppm *ppm_new(unsigned W, unsigned H);
void ppm_destroy(struct ppm **ppm_ptr);

struct color *ppm_color(struct ppm *ppm, unsigned i, unsigned j);
void ppm_write(struct ppm *ppm, FILE *f);

unsigned ppm_2d_to_1d(struct ppm *ppm, unsigned i, unsigned j);
