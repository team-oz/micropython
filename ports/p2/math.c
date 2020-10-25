// This file includes missing math functions not present in the Propeller2 math library but required by MicroPython code

#include "../../lib/libm/libm.h"
#include <complex.h>
#include <string.h>

// this intermediate function is required to trick optimizer to avoid recursion in cexpf
float sinecos(float x, float y, int sine)
{
    if (sine)
        return y*sinf(x);
    else
        return y*cosf(x);
}


float complex cexpf(float complex z)
{
    float a[2], b;
    float complex c;

    a[1] = 0.0; // Needed to avoid compiler warning message
    memcpy(a, &z, 8);
    if (a[0] == 0)
        b = 1.0;
    else
        b = expf(a[0]);
    a[0] = sinecos(a[1], b, 0);
    a[1] = sinecos(a[1], b, 1);
    memcpy(&c, a, 8);

    return c;
}
