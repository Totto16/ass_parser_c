

#include "./zmap.h"

ZMAP_FUN_ATTRIBUTES uint32_t zmap_default_hash(const void *key, size_t len)
{
    uint32_t hash = 2166136261u;
    const uint8_t *data = (const uint8_t *)key;
    for (size_t i = 0; i < len; i++)
    {
        hash ^= data[i];
        hash *= 16777619;
    }
    return hash;
}
