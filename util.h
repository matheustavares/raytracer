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

static inline float rand_in(float a, float b)
{
	return a + (((float)rand() / RAND_MAX) * (b - a));
}
