

#include "./platform.h"

void platform_log_single(bool error, const char* message) {

	platform_log_start(error);
	platform_log_add(message);
	platform_log_end();
}
