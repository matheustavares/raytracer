#pragma once

#define max(a, b) ({ \
		typeof(a) _a = (a); \
		typeof(b) _b = (b); \
		_a > _b ? _a : _b; \
})

static inline float rand_in(float a, float b)
{
	return a + (((float)rand() / RAND_MAX) * (b - a));
}
