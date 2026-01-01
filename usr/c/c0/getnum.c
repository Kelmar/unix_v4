#include "c0.h"

#include <stdio.h>

int totdig; /* Total number of digits read */

/*
 * Translation of PDP-11 assembly getdig helper function.
 *
 * Reads a single character from the stream and adds it to the cval integer accumulator.
 * 
 * Returns true if a character was a recognized digit, false if not.
 * 
 * The last read digit (or value) is placed into the pointer chr
 */
int getdig(int base, int *chr)
{
    *chr = peekc;

    if (!*chr)
        *chr = getchar();
    else
        peekc = 0;

    if (*chr >= '0' && *chr <= '9')
    {
        ++totdig;
        *chr -= '0'; /* Convert character to value. */
        cval = (cval * base) + *chr;
        return 1;
    }

    return 0;
}

int abs(int val)
{
    return val < 0 ? -val : val;
}

/*
 * Translation of the PDP-11 getnum function.
 *
 * Reads a number from the input stream with the given base type.
 * 
 * Returns the token type
 */
int getnum(int base)
{
    int decPoint, nfract, chr;
    int neg;

    double acc, mag;

    acc = 0;

    nfract = 0;
    totdig = 0;
    cval = 0;
    decPoint = 0;
    neg = 0;

    for (;;)
    {
        while (getdig(base, &chr))
        {
            acc = acc * 10 + chr;
            ++nfract;
        }

        if (decPoint)
            break;

        nfract = 0;

        if (chr != '.')
            break;

        /* Decimal point found. */
        decPoint = 1; // Was assigned value from PC?
    }

    if (totdig != 0)
    {
        if (chr == 'e')
        {
            /* Exponent */
            neg = 0;

            cval = 0;
            decPoint = 1; // Was assigned value from PC?
            cval = 0; // Doing this twice?
            base = 10;

            chr = getchar();

            if (chr == '-')
                neg = 1;
            else if (chr != '+')
                peekc = chr; /* Character is likely a number. */

            while (getdig(base, &chr))
                ;

            if (neg)
                cval = -cval;

            nfract -= cval;
        } /* if (r0 == 'e') */
    } /* if (totdig != 0) */

    peekc = chr;

    if (totdig == 0)
        return DOT; /* DOT operator */

    if (!decPoint)
        return CON; /* Fixed point constant */

    mag = 1;

    for (int cnt = abs(nfract); cnt > 0; --cnt)
        mag *= 10;

    if (nfract > 0)
        acc /= mag;
    else
        acc *= mag;

    fcval = acc;

    // A bunch of stack like operations and tests, but no idea what we're looking for....
    //tst (r0)+
    //tst (r0)+
    //bne 1f
    //tst (r0)+
    //bne 1f
    //tst (r0)+
    //bne 1f

    // Seems like this is testing for some sort zero value in the double value of fcval

    //if (/* ???? */)
    if (fcval == 0) // Unclear if this is the intention.
    {
        cval = (int)fcval;
        return SFCON; // Float?
    }

// 1:
    
    return FCON; // Double?
}
