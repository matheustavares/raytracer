#include "ppm.h"
#include "lib/wrappers.h"
#include "lib/error.h"

static struct vec3 color_float_to_byte(struct vec3 color)
{
	return vec3_map(vec3_smul(clamp_color_vec(color), 255), roundf);
}

void ppm_write(struct ppm *ppm, FILE *f)
{
	fprintf(f, "P3\n");
	fprintf(f, "%u %u\n", ppm->cols, ppm->rows);
	fprintf(f, "255\n");
	for (unsigned i = 0; i < ppm->rows; i++) {
		for (unsigned j = 0; j < ppm->cols; j++) {
			char sep = j == ppm->cols - 1 ? '\n' : '\t';
			struct vec3 color = color_float_to_byte(*ppm_color(ppm, i, j));
			fprintf(f, "%d %d %d%c", (int)color.x,
				(int)color.y, (int)color.z, sep);
		}
	}
}

struct ppm *ppm_new(unsigned rows, unsigned cols)
{
	struct ppm *ppm = xmalloc(sizeof(*ppm));
	ppm->rows = rows;
	ppm->cols = cols;
	ppm->img = xcalloc(rows * cols, sizeof(*(ppm->img)));
	return ppm;
}

unsigned ppm_2d_to_1d(struct ppm *ppm, unsigned i, unsigned j)
{
	if (i >= ppm->rows || j >= ppm->cols)
	die("invalid index (%u, %u) for img of size (%u, %u)",
	    i, j, ppm->rows, ppm->cols);
	return i * ppm->cols + j;
}

struct vec3 *ppm_color(struct ppm *ppm, unsigned i, unsigned j)
{
	return &(ppm->img[ppm_2d_to_1d(ppm, i, j)]);
}

void ppm_destroy(struct ppm **ppm_ptr)
{
	struct ppm *ppm = *ppm_ptr;
	free(ppm->img);
	free(ppm);
	*ppm_ptr = NULL;
}
