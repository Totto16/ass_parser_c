

#include "./stdio.h"
#include "./platform.h"
#include "./stdarg.h"
#include "./stdlib.h"

#include <stb/sprintf.h>

typedef enum {
	FileTypeStdin,
	FileTypeStdout,
	FileTypeStderr,
} FileType;

struct FileImpl {
	FileType type;
};

static FILE stdin_f_l = { .type = FileTypeStdin };
static FILE stdout_f_l = { .type = FileTypeStdout };
static FILE stderr_f_l = { .type = FileTypeStderr };

FILE* stdin_f = &stdin_f_l;
FILE* stdout_f = &stdout_f_l;
FILE* stderr_f = &stderr_f_l;

static int vfprintf(FILE* restrict f, const char* restrict fmt, va_list ap) {

	va_list ap2;
	va_copy(ap2, ap);

	size_t needed_size = stbsp_vsnprintf(NULL, 0, fmt, ap2);
	va_end(ap2);

	char* result_buffer = (char*)malloc(needed_size);

	size_t result = stbsp_vsnprintf(result_buffer, needed_size, fmt, ap);

	if(result >= needed_size) {
		free(result_buffer);

		platform_error(
		    "fprintf internals did write more bytes then it had space in the buffer, available "
		    "space");
		return -1;
	}

	if(f == stdout_f) {
		platform_log(result_buffer);
	} else if(f == stderr_f) {
		platform_error(result_buffer);
	} else {
		free(result_buffer);

		platform_error("tried to log to unknown unsupported file");
		return -1;
	}

	free(result_buffer);
	return needed_size;
}

// see
// https://github.com/esmil/musl/blob/194f9cf93da8ae62491b7386edf481ea8565ae4e/src/stdio/fprintf.c
__attribute__((export_name("fprintf"))) int fprintf(FILE* restrict stream,
                                                    const char* restrict format, ...) {

	int ret;
	va_list ap;
	va_start(ap, format);
	ret = vfprintf(stream, format, ap);
	va_end(ap);
	return ret;
}

// see
// https://github.com/esmil/musl/blob/194f9cf93da8ae62491b7386edf481ea8565ae4e/src/stdio/snprintf.c
__attribute__((export_name("snprintf"))) int snprintf(char* restrict s, size_t n,
                                               const char* restrict format, ...) {

	int ret;
	va_list ap;
	va_start(ap, format);
	ret = stbsp_vsnprintf(s, n, format, ap);
	va_end(ap);
	return ret;
}
