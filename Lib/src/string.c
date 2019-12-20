#include <string.h>

size_t strlen(const char *pszStr)
{
	size_t cch = 0;
	while (*pszStr++ != '\0')
		cch++;

	return cch;
}

void memcpy(void *pvDst, const void *pvSrc, size_t cb)
{
	uint8_t *pbDst = (uint8_t *)pvDst;
	uint8_t *pbSrc = (uint8_t *)pvSrc;

	for (unsigned i = 0; i < cb; i++)
		*pbDst++ = *pbSrc++;
}
