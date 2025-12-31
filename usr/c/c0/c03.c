#
/*
 * C compiler
 * Copyright 1972 Bell Telephone Laboratories, Inc.
 */

#include <stdio.h>

#include "c0.h"

int putchar_old(int c);

int decref(int at)
{
    register int t;

    t = at;

    if ((t & ~07) == 0)
    {
        error("Illegal indirection");
        return (t);
    }

    return ((t >> 2) & ~07 | t & 07);
}

int incref(int t)
{
    return ((t << 2) & ~034 | (t & 07) | PTR);
}

int cbranch(struct tnode *tree, int lbl, int cond)
{
    rcexpr(block(1, CBRANCH, tree, lbl, cond, 0, 0), cctab);
}

void rcexpr(int *tree, struct tnode *table)
{
    register int c, *sp;

    if (tree == 0)
        return;

    putchar_old('#');
    c = space - treebase;
    sp = treebase;
    putw(c, binbuf);
    putw(tree, binbuf);
    putw(table, binbuf);
    putw(line, binbuf);

    while (c--)
        putw(*sp++, binbuf);
}

void branch(int lab)
{
    printf("jbr\tL%d\n", lab);
}

void label(int l)
{
    printf("L%d:", l);
}

int length(struct tnode *acs)
{
    register int t, n;
    register struct tnode *cs;

    cs = acs;
    t = cs->type;
    n = 1;

    while ((t & 030) == ARRAY)
    {
        t = decref(t);
        n = dimtab[((struct sym_list__ *)cs)->ssp & 0377];
    }

    if ((t & ~07) == FUNC)
        return (0);

    if (t >= PTR)
        return (2 * n);

    switch (t & 07)
    {
    case INT:
        return (2 * n);

    case CHAR:
        return (n);

    case FLOAT:
        return (4 * n);

    case DOUBLE:
        return (8 * n);

    case STRUCT:
        return (n * dimtab[((struct sym_list__ *)cs)->lenp & 0377]);

    case RSTRUCT:
        error("Bad structure");
        return (0);
    }

    error("Compiler error (length)");
}

int rlength(struct tnode *cs)
{
    register int l;

    if (((l = length(cs)) & 01) != 0)
        l++;

    return (l);
}

int plength(struct tnode *ap)
{
    register int t, l;
    register struct tname *p;

    p = ap;

    if (((t = p->ntype) & ~07) == 0) /* not a reference */
        return (1);

    p->ntype = decref(t);
    l = length(p);
    p->ntype = t;

    return (l);
}

int simplegoto()
{
    register struct hshtab *csp;

    if ((peeksym = symbol()) == NAME && nextchar() == ';')
    {
        csp = csym;

        if (csp->hclass == 0 && csp->htype == 0)
        {
            csp->htype = ARRAY;

            if (csp->hoffset == 0)
                csp->hoffset = isn++;
        }

        if ((csp->hclass == 0 || csp->hclass == STATIC) && csp->htype == ARRAY)
        {
            peeksym = -1;
            return (csp->hoffset);
        }
    }

    return (0);
}

int nextchar()
{
    while (ctab[peekc] == SPACE)
        peekc = getchar();

    return (peekc);
}

void chconbrk(l)
{
    if (l == 0)
        error("Break/continue error");
}

void dogoto()
{
    register struct tnode *np;

    *cp++ = tree();
    build(STAR);
    chkw(np = *--cp);

    rcexpr(block(1, JUMP, 0, 0, np, 0, 0), regtab);
}

void doret()
{
    if (nextchar() != ';')
        rcexpr(block(1, RFORCE, 0, 0, tree(), 0, 0), regtab);

    branch(retlab);
}

int putchar_old(int c)
{
    return putc_old(c, ascbuf);
}
