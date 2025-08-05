#include "./helper.h"

AllocatorStatistics allocator_get_statistics(void) {
	return my_malloc_get_statistics();
}

uint64_t allocator_statistics_get_free(AllocatorStatistics statistics) {
	return statistics.free;
}

uint64_t allocator_statistics_get_total(AllocatorStatistics statistics) {
	return statistics.total;
}

uint64_t allocator_statistics_get_used(AllocatorStatistics statistics) {
	return statistics.used;
}

uint64_t allocator_statistics_get_metadata(AllocatorStatistics statistics) {
	return statistics.metadata;
}
