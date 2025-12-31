#
/*
 * Polyfill for C Compiler
 *
 * Copyright 2025 Bryce Simonds
 */

int fcreat(const char *path, const char *buf)
{
    return -1;
}

int putc_old(int c, char *buf)
{
    return c;
}

void putw_old(int *p1, char *p2)
{
}

void fflush_old(char buffer[518])
{
}

