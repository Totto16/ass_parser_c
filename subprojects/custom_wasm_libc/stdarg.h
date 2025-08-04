

#pragma once

#ifndef _VA_LIST
#define _VA_LIST
typedef __builtin_va_list va_list;
#endif

#ifndef va_arg

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
/* C23 does not require the second parameter for va_start. */
#define va_start(ap, ...) __builtin_va_start(ap, 0)
#else
/* Versions before C23 do require the second parameter. */
#define va_start(ap, param) __builtin_va_start(ap, param)
#endif
#define va_end(ap) __builtin_va_end(ap)
#define va_arg(ap, type) __builtin_va_arg(ap, type)

#endif
