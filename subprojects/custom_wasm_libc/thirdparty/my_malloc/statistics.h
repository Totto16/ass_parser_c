#pragma once

#include <stdint.h>

typedef struct {
	uint64_t total;
	uint64_t free;
	uint64_t used;
	uint64_t metadata;
} AllocatorStatistics;

AllocatorStatistics my_malloc_get_statistics(void);
