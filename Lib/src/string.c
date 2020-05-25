#include <stddef.h>
#include <string.h>

size_t strlen(const char *pszStr)
{
	size_t cch = 0;
	while (*pszStr++ != '\0')
		cch++;

	return cch;
}

void memset(void *pvDst, uint8_t ch, size_t cb)
{
    uint8_t *pb = (uint8_t *)pvDst;
    for (unsigned i = 0; i < cb; i++)
        *pb++ = ch;
}

void memcpy(void *pvDst, const void *pvSrc, size_t cb)
{
	uint8_t *pbDst = (uint8_t *)pvDst;
	uint8_t *pbSrc = (uint8_t *)pvSrc;

	for (unsigned i = 0; i < cb; i++)
		*pbDst++ = *pbSrc++;
}

int memcmp(const void *p1, const void *p2, size_t n)
{
    size_t i;

    /**
    * p1 and p2 are the same memory? easy peasy! bail out
    */
    if (p1 == p2)
    {
        return 0;
    }

    // This for loop does the comparing and pointer moving...
    for (i = 0; (i < n) && (*(uint8_t *)p1 == *(uint8_t *)p2);
        i++, p1 = 1 + (uint8_t *)p1, p2 = 1 + (uint8_t *)p2);
        
    //if i == length, then we have passed the test
    return (i == n) ? 0 : (*(uint8_t *)p1 - *(uint8_t *)p2);
}

