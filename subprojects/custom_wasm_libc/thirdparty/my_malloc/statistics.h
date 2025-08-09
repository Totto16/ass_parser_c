#pragma once

#include <stdint.h>

typedef struct {
	uint64_t free;
	uint64_t used;
	uint64_t metadata;
	uint64_t total;
} AllocatorStatistics;

[[nodiscard]] AllocatorStatistics my_malloc_get_statistics(void);
