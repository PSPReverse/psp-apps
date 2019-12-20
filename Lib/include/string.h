#ifndef __include_string_h
#define __include_string_h

#include <types.h>

size_t strlen(const char *pszStr);

void memcpy(void *pvDst, const void *pvSrc, size_t cb);

int memcmp(const void *pv1, const void *pv2, size_t cb);

#endif
