



#ifndef STBDS_ONLY_MACROS


#ifndef _HAVE_STBDS_DECL
#define _HAVE_STBDS_DECL

#define STBDS_NO_SHORT_NAMES
#include "./stb_ds.h"

// remove macros, as we don#t use stbds arrays (we use zvec instead), just hashmaps
#undef stbds_arrlen
#undef stbds_arrlenu
#undef stbds_arrput
#undef stbds_arrput
#undef stbds_arrpop
#undef stbds_arrfree
#undef stbds_arraddn
#undef stbds_arraddnptr
#undef stbds_arraddnindex
#undef stbds_arrsetlen
#undef stbds_arrlast
#undef stbds_arrins
#undef stbds_arrinsn
#undef stbds_arrdel
#undef stbds_arrdeln
#undef stbds_arrdelswap
#undef stbds_arrcap
#undef stbds_arrsetcap
#endif

#endif


#ifndef _HAVE_STBDS_MACROS
#define _HAVE_STBDS_MACROS

#define STBDS_HASH_MAP_TYPE(KeyType, ValueType, TypeName) \
	typedef struct { \
		KeyType key; \
		ValueType value; \
	} TypeName

#define STBDS_HASH_MAP(TypeName) TypeName*

#define STBDS_HASH_MAP_EMPTY NULL


#endif
