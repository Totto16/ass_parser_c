
#include "./assert.h"

#include "./platform.h"

void custom_assert(const char* file, int line, bool cond, const char* message) {
	if(!cond) {
		platform_panic(file, line, message);
	}
}
