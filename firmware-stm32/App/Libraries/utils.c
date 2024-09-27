#include "utils.h"

float map(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
/*
void *volatile_memcpy(volatile void *dest, const volatile void *src, size_t n)
{
    volatile unsigned char *d = (volatile unsigned char *) dest;
    const volatile unsigned char *s = (const volatile unsigned char *) src;

    for (size_t i = 0; i < n; ++i) {
        d[i] = s[i];
    }

    return (void *) dest;
}
*/