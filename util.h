#pragma once

#define max(a, b) ({ \
		typeof(a) _a = (a); \
		typeof(b) _b = (b); \
		_a > _b ? _a : _b; \
})

#define sign(a) ({ \
		typeof(a) _a = (a); \
		_a >= 0 ? 1 : -1; \
})

#define square(a) ({ \
		typeof(a) _a = (a); \
		_a * _a; \
})


  int rand_r(unsigned int *seedp);
       void srand(unsigned int seed);


static inline float rand_r_in(unsigned int *state, float a, float b)
{
	return a + (((float)rand_r(state) / RAND_MAX) * (b - a));
}
