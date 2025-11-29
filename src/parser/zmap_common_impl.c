

#include <zmap/zmap.h>

#include "./types.h"

ZMAP_IMPLEMENT_MAP_TYPE(char*, CHAR_PTR_KEYNAME, FinalStr, SectionFieldEntry)

ZMAP_IMPLEMENT_MAP_TYPE(char*, CHAR_PTR_KEYNAME, ExtraSectionEntry, ExtraSectionHashMapEntry)
