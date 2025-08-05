#pragma once

#include <stdint.h>

#ifndef NDEBUG

typedef struct {
	uint64_t total;
	uint64_t free;
	uint64_t used;
	uint64_t metadata;
} AllocatorStatistics;

AllocatorStatistics allocator_get_statistics(void);

#endif
