

#pragma once

#include "../helper/decl.h"
#include <stddef.h>
#include <stdint.h>

#ifndef NDEBUG

// expose allocator statistics

#include <my_malloc/statistics.h>

PUBLIC("allocator_get_statistics") AllocatorStatistics allocator_get_statistics(void);

PUBLIC("allocator_statistics_get_free")
uint64_t allocator_statistics_get_free(AllocatorStatistics statistics);

PUBLIC("allocator_statistics_get_total")
uint64_t allocator_statistics_get_total(AllocatorStatistics statistics);

PUBLIC("allocator_statistics_get_used")
uint64_t allocator_statistics_get_used(AllocatorStatistics statistics);

PUBLIC("allocator_statistics_get_metadata")
uint64_t allocator_statistics_get_metadata(AllocatorStatistics statistics);

#endif
