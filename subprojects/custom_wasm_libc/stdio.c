

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

static void stbsp_init() {
	stbsp_set_separators(',', '.');
}

static char* vfprintf_callback(const char* buf, void* user, int len) {

	char* buf_start = (char*)user;

	if(len > STB_SPRINTF_MIN) {
		platform_log_single(true, "formatting returned a too big chunk");
		return NULL;
	}

	buf_start[len] = '\0';

	platform_log_add(buf);

	return buf_start;
}

static int vfprintf(FILE* restrict f, const char* restrict fmt, va_list ap) {

	static char static_vfprintf_buf[STB_SPRINTF_MIN + 1] = { 0 };

	bool error;

	if(f == stdout_f) {
		error = false;
	} else if(f == stderr_f) {
		error = true;
	} else {
		platform_log_single(true, "tried to log to unknown unsupported file");
		return -1;
	}

	platform_log_start(error);

	size_t result =
	    stbsp_vsprintfcb(vfprintf_callback, static_vfprintf_buf, static_vfprintf_buf, fmt, ap);

	platform_log_end();

	return result;
}

// see
// https://github.com/esmil/musl/blob/194f9cf93da8ae62491b7386edf481ea8565ae4e/src/stdio/fprintf.c
int fprintf(FILE* restrict stream, const char* restrict format, ...) {

	int ret;
	va_list ap;
	va_start(ap, format);
	ret = vfprintf(stream, format, ap);
	va_end(ap);
	return ret;
}

// see
// https://github.com/esmil/musl/blob/194f9cf93da8ae62491b7386edf481ea8565ae4e/src/stdio/snprintf.c
int snprintf(char* restrict s, size_t n, const char* restrict format, ...) {

	int ret;
	va_list ap;
	va_start(ap, format);
	ret = stbsp_vsnprintf(s, n, format, ap);
	va_end(ap);
	return ret;
}

__attribute__((constructor)) void stdio_constructor(void) {
	stbsp_init();
}
