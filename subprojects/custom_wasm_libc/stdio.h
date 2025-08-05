

#pragma once

#include "./stddef.h"

typedef struct FileImpl FILE;

/* Standard streams.  */
extern FILE* stdin_f;
extern FILE* stdout_f;
extern FILE* stderr_f;

/* C89/C99 say they're macros.  Make them happy.  */
#define stdin stdin_f
#define stdout stdout_f
#define stderr stderr_f

__attribute__((format(printf, 2, 3))) int fprintf(FILE* restrict stream,
                                                  const char* restrict format, ...);

__attribute__((format(printf, 3, 4))) int snprintf(char* restrict s, size_t n,
                                                   const char* restrict format, ...);
