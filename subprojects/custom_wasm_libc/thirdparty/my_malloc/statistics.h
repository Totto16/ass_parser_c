#pragma once

#include <stdint.h>

typedef struct {
	uint64_t free;
	uint64_t used;
	uint64_t metadata;
	uint64_t total;
} AllocatorStatistics;

__attribute__((export_name("allocator_get_statistics"))) AllocatorStatistics allocator_get_statistics(void);
