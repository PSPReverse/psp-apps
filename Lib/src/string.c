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
    if (   ((uintptr_t)pvDst & 3)
        || cb < sizeof(uint32_t))
    {
        uint8_t *pb = (uint8_t *)pvDst;
        for (unsigned i = 0; i < cb; i++)
            *pb++ = ch;
    }
    else
    {
        /* A bit more optimized */
        uint32_t *pu32Dst = (uint32_t *)pvDst;
        uint32_t dw = (ch << 24) | (ch << 16) | (ch << 8) | ch;

        while (cb >= sizeof(uint32_t))
        {
            *pu32Dst++ = dw;
            cb -= sizeof(uint32_t);
        }

        uint8_t *pbDst = (uint8_t *)pu32Dst;
        switch (cb)
        {
            default:
            case 0:
                break;
            case 1:
                *pbDst = ch;
                break;
            case 2:
                *pbDst++ = ch;
                *pbDst   = ch;
                break;
            case 3:
                *pbDst++ = ch;
                *pbDst++ = ch;
                *pbDst   = ch;
                break;
        }
    }
}

void memcpy(void *pvDst, const void *pvSrc, size_t cb)
{
    if (   cb < sizeof(uint32_t)
        || ((uintptr_t)pvDst & 3)
        || ((uintptr_t)pvSrc & 3))
    {
        uint8_t *pbDst = (uint8_t *)pvDst;
        const uint8_t *pbSrc = (const uint8_t *)pvSrc;

        for (unsigned i = 0; i < cb; i++)
            *pbDst++ = *pbSrc++;
    }
    else
    {
        /* A bit more optimized */
        uint32_t *pu32Dst = (uint32_t *)pvDst;
        const uint32_t *pu32Src = (const uint32_t *)pvSrc;

        while (cb >= sizeof(uint32_t))
        {
            *pu32Dst++ = *pu32Src++;
            cb -= sizeof(uint32_t);
        }

        uint8_t *pbDst = (uint8_t *)pu32Dst;
        const uint8_t *pbSrc = (uint8_t *)pu32Src;
        switch (cb)
        {
            default:
            case 0:
                break;
            case 1:
                *pbDst = *pbSrc;
                break;
            case 2:
                *pbDst++ = *pbSrc++;
                *pbDst = *pbSrc;
                break;
            case 3:
                *pbDst++ = *pbSrc++;
                *pbDst++ = *pbSrc++;
                *pbDst = *pbSrc;
                break;
        }
    }
}

