#pragma once

#include <stdio.h>
#include <assert.h>
#include "vec3.h"

struct ppm {
	unsigned cols, rows;
	/*
	 * Each pixel is a vec3, where x, y, and z respectively corresponds to
	 * the R, G, and B components. The components are encoded in floats
	 * from 0.0 to 1.0 (although the values are allowed to under/overflow
	 * when manipulating them). When writing out the image, the values are
	 * clamped and converted to the [0, 255] range.
	 */
	struct vec3 *img;
};

#define clamp_color(a) ((a) > 1 ? 1 : (a) < 0 ? 0 : (a))
#define clamp_color_vec(c) \
	((struct vec3){clamp_color((c).x), clamp_color((c).y), clamp_color((c).z)})

static struct vec3 color_average(struct vec3 *colors, int size)
{
	struct vec3 average = {0};
	for (int i = 0; i < size; i++)
		average = vec3_add(average, colors[i]);
	average = vec3_smul(average, 1.0 / size);
	return clamp_color_vec(average);
}

struct ppm *ppm_new(unsigned W, unsigned H);
void ppm_destroy(struct ppm **ppm_ptr);

struct vec3 *ppm_color(struct ppm *ppm, unsigned i, unsigned j);
void ppm_write(struct ppm *ppm, FILE *f);

unsigned ppm_2d_to_1d(struct ppm *ppm, unsigned i, unsigned j);

void ppm_resize(struct ppm *ppm, unsigned rows, unsigned cols);
