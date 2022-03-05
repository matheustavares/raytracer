#include <stdio.h>
#include "ppm.h"

int main(int argc, char **argv)
{
	int N = 256;
	struct ppm *ppm = ppm_new(N, N);
	unsigned max = ppm_2d_to_1d(ppm, N - 1, N - 1);
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			struct color *c = ppm_color(ppm, i, j);
			c->R = ((double)ppm_2d_to_1d(ppm, i, j) / max) * 255;
			c->B = ((double)ppm_2d_to_1d(ppm, i, j) / max) * 255;
			c->G = 20;
		}
	}
	ppm_write(ppm, stdout);
	ppm_destroy(&ppm);
	return 0;
}
