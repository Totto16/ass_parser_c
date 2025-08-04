

#include "./stdio.h"

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
