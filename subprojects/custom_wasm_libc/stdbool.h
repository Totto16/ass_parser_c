

#pragma once

#if !defined(__STDC_VERSION__)

// C90

#error "bool not supported"

#elif __STDC_VERSION__ >= 202311L
// c23

// (no defines needed, all value are keywords)

#elif __STDC_VERSION__ >= 199901L

// c99

#define bool _Bool
#define true 1
#define false 0

#define __bool_true_false_are_defined 1

#endif
