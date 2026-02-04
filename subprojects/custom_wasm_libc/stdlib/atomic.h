#ifndef _INTERNAL_ATOMIC_H
#define _INTERNAL_ATOMIC_H

//From: https://github.com/esmil/musl/blob/194f9cf93da8ae62491b7386edf481ea8565ae4e/arch/sh/atomic.h
// extracted used functions

#include <stdint.h>

static inline int a_ctz_l(unsigned long x)
{
	static const char debruijn32[32] = {
		0, 1, 23, 2, 29, 24, 19, 3, 30, 27, 25, 11, 20, 8, 4, 13,
		31, 22, 28, 18, 26, 10, 7, 12, 21, 17, 9, 6, 16, 5, 15, 14
	};
	return debruijn32[(x&-x)*0x076be629 >> 27];
}

#endif
