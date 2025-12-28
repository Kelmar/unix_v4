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

void putw_old(int *p1, char *p2)
{
}

