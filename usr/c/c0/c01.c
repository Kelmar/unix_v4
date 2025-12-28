#
/*
 * C compiler
 * Copyright 1972 Bell Telephone Laboratories, Inc.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "c0.h"

void chklval(struct tnode *ap);
struct tnode *chkfun(struct tnode *ap);
void chkw(struct tnode *p);
void setype(struct tnode *ap, int at, int adimptr);

void build(int op)
{
    register int t1;
    int t2, t3, t;
    struct tnode *p3, *disarray();
    register struct tnode *p1, *p2;
    struct tname *p2name;
    int d, dope, leftc, cvn, pcvn;

    if (op == LBRACK)
    {
        build(PLUS);
        op = STAR;
    }

    dope = opdope[op];

    if ((dope & BINARY) != 0)
    {
        p2 = chkfun(disarray(*--cp));
        t2 = p2->type;
    }

    p1 = *--cp;

    if (op == SIZEOF)
    {
        t1 = length(p1);
        p1->op = CON;
        p1->type = INT;
        p1->dimp = 0;
        ((struct tconst *)p1)->value = t1;
        *cp++ = p1;
        return;
    }

    if (op != AMPER)
    {
        p1 = disarray(p1);

        if (op != CALL)
            p1 = chkfun(p1);
    }

    t1 = p1->type;
    pcvn = 0;

    switch (op)
    {
    /* end of expression */
    case 0:
        *cp++ = p1;
        return;

    /* no-conversion operators */
    case COMMA:
    case LOGAND:
    case LOGOR:
        *cp++ = block(2, op, 0, 0, p1, p2, 0);
        return;

    case QUEST:
        if (p2->op != COLON)
            error("Illegal conditional");
        t = t2;
        goto nocv;

    case CALL:
        if ((t1 & 030) != FUNC)
            error("Call of non-function");
        *cp++ = block(2, CALL, decref(t1), p1->dimp, p1, p2, 0);
        return;

    case STAR:
        if (p1->op == AMPER || p1->op == AMPERA)
        {
            *cp++ = p1->tr1;
            return;
        }
        if ((t1 & 030) == FUNC)
            error("Illegal indirection");
        *cp++ = block(1, STAR, decref(t1), p1->dimp, p1, 0, 0);
        return;

    case AMPER:
        if (p1->op == STAR)
        {
            p1->tr1->dimp = p1->dimp;
            p1->tr1->type = incref(t1);
            *cp++ = p1->tr1;
            return;
        }

        if (p1->op == NAME)
        {
            if (((struct tname *)p1)->class == AUTO)
                op = AMPERA;

            *cp++ = block(1, op, incref(t1), p1->dimp, p1, 0, 0);
            return;
        }
        error("Illegal lvalue");
        break;

    case INCBEF:
    case DECBEF:
    case INCAFT:
    case DECAFT:
        chklval(p1);
        *cp++ = block(2, op, t1, p1->dimp, p1, plength(p1), 0);
        return;

    case ARROW:
        *cp++ = p1;
        chkw(p1);
        p1->type = PTR + STRUCT;
        build(STAR);
        p1 = *--cp;

    case DOT:
        /*
        if (p2->op != NAME || p2->class != MOS)
            error("Illegal structure ref");
        */

        if (p2->op != NAME)
            error("Illegal structure ref");
        
        p2name = (struct tname *)p2;

        if (p2name->class != MOS)
            error("Illegal structure ref");

        *cp++ = p1;
        t = t2;

        if ((t & 030) == ARRAY)
        {
            t = decref(t);
            p2->ssp++;
        }

        setype(p1, t, p2->dimp);
        build(AMPER);
        *cp++ = block(1, CON, 7, 0, p2name->nloc, 0, 0);
        build(PLUS);

        if ((t2 & 030) != ARRAY)
            build(STAR);

        return;
    }

    if ((dope & LVALUE) != 0)
        chklval(p1);

    if ((dope & LWORD) != 0)
        chkw(p1);

    if ((dope & RWORD) != 0)
        chkw(p2);

    if ((dope & BINARY) == 0)
    {
        if (!fold(op, p1, 0))
            *cp++ = block(1, op, t1, p1->dimp, p1, 0, 0);
        return;
    }

    if (t2 == 7)
    {
        t = t1;
        p2->type = 0; /* no int cv for struct */
        t2 = 0;
        goto nocv;
    }

    cvn = cvtab[11 * lintyp(t1) + lintyp(t2)];
    leftc = cvn & 0100;
    t = leftc ? t2 : t1;

    if (op == ASSIGN && t1 != STRUCT && t2 != STRUCT)
    {
        t = t1;

        if (leftc || cvn != 3) /* int -> float */
            goto nocv;
    }

    if (cvn &= 077)
    {
        if (op == COLON && t1 >= PTR && t1 == t2)
            goto nocv;

        if (cvn == 077)
        {
            if ((dope & RELAT) == 0 || t1 < PTR || t2 < PTR)
            illcv:
                error("Illegal conversion");
            goto nocv;
        }

        if (cvn == 2)
        {          /* ptr conv */
            t = 0; /* integer result */
            if ((dope & RELAT) != 0)
            {
                cvn = 0;
                goto nocv;
            }

            if (op != MINUS)
                goto illcv;

            pcvn = cvn;
            cvn = 0;
            goto nocv;
        }

        if (leftc)
        {
            if ((dope & ASSGOP) != 0)
            {
                if (cvn == 3)
                { /* int->float */
                    leftc = 0;
                    cvn = 4;
                    t = t1;
                    goto rcvt;
                }
                else
                    goto illcv;
            }
            p1 = convert(p1, t, cvn, plength(p2));
        }
        else
        {
        rcvt:
            p2 = convert(p2, t, cvn, plength(p1));
        }
    nocv:;
    }

    if ((dope & RELAT) != 0)
    {
        if (op > NEQUAL && (t1 >= PTR || t2 >= PTR))
            op += 4; /* ptr relation */
        t = 0;       /* relational is integer */
    }

    if (fold(op, p1, p2))
        return;

    *cp++ = block(2, op, t, (p1->dimp == 0 ? p2 : p1)->dimp, p1, p2, 0);

    if (pcvn)
    {
        p1 = *--cp;
        *cp++ = convert(p1, 0, pcvn, plength(p1->tr1));
    }
}

int convert(struct tnode *p, int t, int cvn, int len)
{
    register int n;
    int *b;

    switch (cvn)
    {
    /* int -> ptr */
    case 1:
        n = TIMES;
        goto md;

    /* ptr -> int */
    case 2:
        n = DIVIDE;
    md:
        b = block(1, CON, 0, 0, len, 0, 0);
        return (block(2, n, t, 0, p, b, 0));

    /* int -> double */
    case 3:
        n = ITOF;
        goto di;

    /* double -> int */
    case 4:
        n = FTOI;
    di:
        return (block(1, n, t, 0, p, 0, 0));
    }

    error("C error-- convert");
    
    return (p);
}

void setype(struct tnode *ap, int at, int adimptr)
{
    register struct tnode *p;
    register int t, dimptr;

    p = ap;
    t = at;
    dimptr = adimptr;
    p->type = t;

    if (dimptr != -1)
        p->dimp = dimptr;

    switch (p->op)
    {
    case AMPERA:
    case AMPER:
        setype(p->tr1, decref(t), dimptr);
        return;

    case STAR:
        setype(p->tr1, incref(t), dimptr);
        return;

    case PLUS:
    case MINUS:
        setype(p->tr1, t, dimptr);
    }
}

struct tnode *chkfun(struct tnode *ap)
{
    register struct tnode *p;
    register int t;

    p = ap;

    if (((t = p->type) & 030) == FUNC)
        return (block(1, AMPER, incref(t), p->dimp, p, 0, 0));

    return (p);
}

struct tnode *disarray(struct tnode *ap)
{
    register int t;
    register struct tnode *p;

    p = ap;

    /* check array & not MOS */
    /*
    if (((t = p->type) & 030) != ARRAY || p->op == NAME && p->class == MOS)
        return (p);
    */
    if (((t = p->type) & 030) != ARRAY)
        return (p);

    if (p->op == NAME)
    {
        if (((struct tname *)p)->class == MOS)
            return (p);
    }

    p->ssp++;
    *cp++ = p;
    setype(p, decref(t), -1);
    build(AMPER);
    return (*--cp);
}

void chkw(struct tnode *p)
{
    register int t;

    if ((t = p->type) > CHAR && t < PTR)
        error("Integer operand required");
}

int lintyp(int at)
{
    register int t;

    t = at;
    if (t < PTR)
        return (t);

    if ((t & 0170) == 0150) /* ptr to array */
        return (9);

    if ((t & 037) == t)
        return ((t & 07) + 5);

    return (10);
}

void error(char *s, ...)
{
    register char f;

    /* Not a chance this will compile on the old compiler. */
    va_list args;
    va_start(args, s);

    nerror++;
    flush_buffer(ascbuf);
    f = ascbuf[0];
    ascbuf[0] = 1;
    printf("%d: ", line);

    /* printf(s, p1, p2, p3, p4, p5, p6); */
    vprintf(s, args);
    va_end(args);

    putchar('\n');
    flush_buffer(ascbuf);
    ascbuf[0] = f;
}

int *block(int num, int op, int t, int d, int p1, int p2, int p3)
{
    register int *ap, *p;

    ap = &op;
    num += 3;
    p = space;

    while (num--)
        pblock(*ap++);

    return (p);
}

void pblock(int p)
{
    *space++ = p;
    if (--osleft <= 0)
    {
        error("Expression overflow");
        exit(1);
    }
}

/*
 * Check for an lvalue expression.
 */
void chklval(struct tnode *ap)
{
    if (ap->op != NAME && ap->op != STAR)
        error("Lvalue required");
}

/*
 * Attempt to fold a constant expression into a single value.
 */
int fold(int op, struct tnode *ap1, struct tnode *ap2)
{
    register struct tnode *p1;
    register int v1, v2;

    p1 = ap1;

    if (p1->op != CON || (ap2 != 0 && ap2->op != CON))
        return (0);

    v1 = ((struct tconst *)p1)->value;
    v2 = ((struct tconst *)ap2)->value;

    switch (op)
    {
    case PLUS:
        v1 += v2;
        break;

    case MINUS:
        v1 -= v2;
        break;

    case TIMES:
        v1 *= v2;
        break;

    case DIVIDE:
        v1 /= v2;
        break;

    case MOD:
        v1 %= v2;
        break;

    case AND:
        v1 &= v2;
        break;

    case OR:
        v1 |= v2;
        break;

    case EXOR:
        v1 ^= v2;
        break;

    case NEG:
        v1 = -v1;
        break;

    case COMPL:
        v1 = ~v1;
        break;

    case LSHIFT:
        v1 <<= v2;
        break;

    case RSHIFT:
        v1 >>= v2;
        break;

    default:
        return (0);
    }

    ((struct tconst *)p1)->value = v1;
    *cp++ = p1;
    return (1);
}

int conexp()
{
    register struct tnode *t;

    initflg++;

    if (t = tree())
    {
        if (t->op != CON)
            error("Constant required");
    }

    initflg--;
    
    return (((struct tconst *)t)->value);
}
