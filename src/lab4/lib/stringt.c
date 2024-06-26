#include "stringt.h"

void *memset(void *dst, int c, uint64 n) {
    char *cdst = (char *)dst;
    for (uint64 i = 0; i < n; ++i)
        cdst[i] = c;

    return dst;
}

void*memcpyt(void * dst, void * src, uint64 size)
{
    char *cdst = (char *)dst;
    char *csrc = (char *)src;

    for(uint64 i=0; i<size; i++)
    {
        cdst[i] = csrc[i];
    }

    return dst;
}