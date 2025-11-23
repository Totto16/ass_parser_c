
#include "./ctype.h"

// from musl: https://github.com/esmil/musl/blob/master/src/ctype/isupper.c

static int isupper(int c) {
	return (unsigned)c - 'A' < 26;
}

// from musl: https://github.com/esmil/musl/blob/master/src/ctype/tolower.c

int tolower(int c) {
	if(isupper(c)) {
		return c | 32;
	}

	return c;
}
