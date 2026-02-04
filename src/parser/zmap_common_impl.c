

#include <tmap.h>

#include "./types.h"

TMAP_IMPLEMENT_MAP_TYPE(char*, CHAR_PTR_KEYNAME, FinalStr, SectionFieldEntry)

TMAP_IMPLEMENT_MAP_TYPE(char*, CHAR_PTR_KEYNAME, ExtraSectionEntry, ExtraSectionHashMapEntry)

TMAP_HASH_FUNC_SIG(char*, CHAR_PTR_KEYNAME) {
	return TMAP_HASH_STR(key);
}

TMAP_COMPARE_FUNC_SIG(char*, CHAR_PTR_KEYNAME) {
	return strcmp(key1, key2);
}
