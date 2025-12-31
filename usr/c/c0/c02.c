#
/*
 * C compiler
 * Copyright 1972 Bell Telephone Laboratories, Inc.
 */

#include <stdlib.h>

#include "c0.h"

void chkdim();
int blkhed();

extdef()
{
    register int o, width;
    int type, elsize, nel;
    char *cs;
    register struct hshtab *ds;

    if (((o = symbol()) == OP_EOF) || o == SEMI)
        return;
    type = 0;
    xdflg++;
xxx:
    if (o == KEYW)
    {
        if (cval == EXTERN)
        {
            o = symbol();
            goto xxx;
        }

        if ((type = cval) > STRUCT)
            goto syntax; /* not type */

        elsize = 0;

        if (type == STRUCT)
        {
            elsize = strdec(&type, 0);

            if ((peeksym = symbol()) != KEYW)
                blkhed();
        }
    }
    else
    {
        if (o != NAME)
            goto syntax;
        peeksym = o;
    }

    do
    {
        defsym = 0;
        strflg = 1;
        decl1(EXTERN, type & 07, 0, elsize);

        if ((ds = defsym) == 0)
            return;

        cs = ds->name;
        funcsym = ds;
        *cs |= 0200; /* non-deletable */
        printf(".globl	_%.8s\n", cs);
        xdflg = 0;
        type = ds->htype;

        if ((type & 030) == FUNC && ((peeksym = symbol()) == LBRACE || peeksym == KEYW))
        {
            cfunc(cs);
            return;
        }

        nel = 1;

        while ((ds->htype & 030) == ARRAY)
        {
            nel = dimtab[((struct sym_list__ *)ds)->ssp & 0377];
            ds->htype = decref(ds->htype);
        }

        width = length(ds);

        if (ds->htype == STRUCT)
        {
            nel *= width / 2;
            width = 2;
        }

        ds->htype = type;
        cinit(cs, type, nel, width);
    } while ((o = symbol()) == COMMA);

    if (o == SEMI)
        return;

syntax:
    error("External definition syntax");
    errflush(o);
    statement(0);
}

void cfunc(char *cs)
{
    register int savdimp;

    strflg = 0;
    savdimp = dimp;
    printf(".text\n_%.8s:\n", cs);
    declist(ARG);
    regvar = 5;
    retlab = isn++;

    if ((peeksym = symbol()) != LBRACE)
        error("Compound statement required");

    statement(1);
    printf("L%d:jmp	rretrn\n", retlab);
    dimp = savdimp;
}

void cinit(char *cs, int type, int nel, int awidth)
{
    register int o, ninit, width;

    width = awidth;

    if ((peeksym = symbol()) == COMMA || peeksym == SEMI)
    {
        printf(".comm	_%.8s,0%o\n", cs, (nel * width + 1) & ~01);
        return;
    }

    ninit = 0;
    printf(".data\n_%.8s=.\n", cs);

    if ((o = symbol()) == LBRACE)
    {
        do
            ninit = cinit1(cs, type, width, ninit);
        while ((o = symbol()) == COMMA);
        if (o != RBRACE)
            peeksym = o;
    }
    else
    {
        peeksym = o;
        ninit = cinit1(cs, type, width, 0);
    }

    if (ninit < nel)
        printf(".=.+%d.\n", (nel - ninit) * width);
    else
        nel = ninit;

    if (nel > 1 && (type & 030) != ARRAY && (type & 07) != STRUCT)
        error("Too many initializers");

    if (((nel & width) & 01) != 0)
        printf(".even\n");
}

cinit1(cs, type, width, ninit) char *cs;
{
    float sf;
    register struct tnode *s;

    if ((peeksym = symbol()) == STRING && type == ARRAY + CHAR)
    {
        peeksym = -1;

        if (ninit)
            bxdec();

        printf(".text\n_%.8s=L%d\n", cs, cval);

        return ((nchstr + 1) & ~01);
    }

    if (peeksym == RBRACE)
        return (ninit);

    initflg++;
    s = tree();
    initflg = 0;

    switch (width)
    {
    case 1:
        printf(".byte ");
        if (s->op != CON)
            bxdec();

    case 2:
        if (s->op == CON)
        {
            printf("%d.\n", ((struct tconst *)s)->value);
            break;
        }
        rcexpr(block(1, INIT, 0, 0, s, 0, 0), regtab);
        break;

    case 4:
        sf = fcval;
        printf("0%o;0%o\n", sf);
        goto flt;

    case 8:
        printf("0%o;0%o;0%o;0%o\n", fcval);
    flt:
        if (s->op == FCON || s->op == SFCON)
            break;

    default:
        bxdec();
    }

    return (++ninit);
}

bxdec()
{
    error("Inconsistent external initialization");
}

statement(d)
{
    register int o, o2;
    register struct hshtab *o1;
    int o3, o4;
    struct tnode *np;

    const char *str;

stmt:
    switch (o = symbol())
    {
    case OP_EOF:
        error("Unexpected EOF");

    case SEMI:
    case RBRACE:
        return;

    case LBRACE:
        if (d)
        {
            if (proflg)
            {
                str = "jsr\tr5,mrsave;0f;%o\n.bss;0:.=.+2\n.text\n";
            }
            else
                str = "jsr	r5,rsave; %o\n";

            printf(str, blkhed() - 4);
        }

        while (!eof)
        {
            if ((o = symbol()) == RBRACE)
                return;
            peeksym = o;
            statement(0);
        }

        error("Missing '}'");
        return;

    case KEYW:
        switch (cval)
        {

        case GOTO:
            if (o1 = simplegoto())
                branch(o1);
            else
                dogoto();
            goto semi;

        case RETURN:
            doret();
            goto semi;

        case IF:
            np = pexpr();
            o2 = 0;
            if ((o1 = symbol()) == KEYW)
            {
                switch (cval)
                {
                case GOTO:
                    if (o2 = simplegoto())
                        goto simpif;

                    cbranch(np, o2 = isn++, 0);
                    dogoto();
                    label(o2);
                    goto hardif;

                case RETURN:
                    if (nextchar() == ';')
                    {
                        o2 = retlab;
                        goto simpif;
                    }

                    cbranch(np, o1 = isn++, 0);
                    doret();
                    label(o1);
                    o2++;
                    goto hardif;

                case BREAK:
                    o2 = brklab;
                    goto simpif;

                case CONTIN:
                    o2 = contlab;

                simpif:
                    chconbrk(o2);
                    cbranch(np, o2, 1);

                hardif:
                    if ((o = symbol()) != SEMI)
                        goto syntax;

                    if ((o1 = symbol()) == KEYW && cval == ELSE)
                        goto stmt;

                    peeksym = o1;
                    return;
                }
            }

            peeksym = o1;
            cbranch(np, o1 = isn++, 0);
            statement(0);

            if ((o = symbol()) == KEYW && cval == ELSE)
            {
                o2 = isn++;
                branch(o2);
                label(o1);
                statement(0);
                label(o2);
                return;
            }

            peeksym = o;
            label(o1);

            return;

        case WHILE:
            o1 = contlab;
            o2 = brklab;
            label(contlab = isn++);
            cbranch(pexpr(), brklab = isn++, 0);
            statement(0);
            branch(contlab);
            label(brklab);
            contlab = o1;
            brklab = o2;
            return;

        case BREAK:
            chconbrk(brklab);
            branch(brklab);
            goto semi;

        case CONTIN:
            chconbrk(contlab);
            branch(contlab);
            goto semi;

        case DO:
            o1 = contlab;
            o2 = brklab;
            contlab = isn++;
            brklab = isn++;
            label(o3 = isn++);
            statement(0);
            label(contlab);
            contlab = o1;

            if ((o = symbol()) == KEYW && cval == WHILE)
            {
                cbranch(tree(), o3, 1);
                label(brklab);
                brklab = o2;
                goto semi;
            }

            goto syntax;

        case CASE:
            o1 = conexp();
            if ((o = symbol()) != COLON)
                goto syntax;

            if (swp == 0)
            {
                error("Case not in switch");
                goto stmt;
            }

            if (swp >= swtab + swsiz)
            {
                error("Switch table overflow");
            }
            else
            {
                swp->swlab = isn;
                (swp++)->swval = o1;
                label(isn++);
            }

            goto stmt;

        case SWITCH:
            o1 = brklab;
            brklab = isn++;
            np = pexpr();
            chkw(np);
            rcexpr(block(1, RFORCE, 0, 0, np, 0, 0), regtab);
            pswitch();
            brklab = o1;

            return;

        case DEFAULT:
            if (swp == 0)
                error("Default not in switch");

            if ((o = symbol()) != COLON)
                goto syntax;

            label(deflab = isn++);
            goto stmt;

        case FOR:
            o1 = contlab;
            o2 = brklab;
            contlab = isn++;
            brklab = isn++;

            if (o = forstmt())
                goto syntax;

            label(brklab);
            contlab = o1;
            brklab = o2;

            return;
        }

        error("Unknown keyword");
        goto syntax;

    case NAME:
        if (nextchar() == ':')
        {
            peekc = 0;
            o1 = csym;

            if (o1->hclass > 0)
            {
                error("Redefinition");
                goto stmt;
            }

            o1->hclass = STATIC;
            o1->htype = ARRAY;

            if (o1->hoffset == 0)
                o1->hoffset = isn++;

            label(o1->hoffset);

            if ((peeksym = symbol()) == RBRACE)
                return;

            goto stmt;
        }
    }

    peeksym = o;
    rcexpr(tree(), efftab);

semi:
    if ((o = symbol()) == SEMI)
        return;

syntax:
    error("Statement syntax");
    errflush(o);
    goto stmt;
}

#define forsps 150

int forstmt()
{
    int l, savxpr[forsps];
    int *st, *ss;
    register int *sp1, *sp2, o;

    if ((o = symbol()) != LPARN)
        return (o);

    if ((o = symbol()) != SEMI)
    { /* init part */
        peeksym = o;
        rcexpr(tree(), efftab);
        if ((o = symbol()) != SEMI)
            return (o);
    }

    label(contlab);

    if ((o = symbol()) != SEMI)
    { /* test part */
        peeksym = o;
        rcexpr(block(1, CBRANCH, tree(), brklab, 0, 0, 0), cctab);
        if ((o = symbol()) != SEMI)
            return (o);
    }

    if ((peeksym = symbol()) == RPARN)
    { /* incr part */
        peeksym = -1;
        statement(0);
        branch(contlab);

        return (0);
    }

    l = contlab;
    contlab = isn++;
    st = tree();

    if ((o = symbol()) != RPARN)
        return (o);

    ss = space;

    if (space - treebase > forsps)
    {
        error("Expression too large");
        space = &treebase[forsps];
    }

    sp2 = savxpr;

    for (sp1 = treebase; sp1 < space;)
        *sp2++ = *sp1++;

    statement(0);
    space = ss;
    sp2 = savxpr;

    for (sp1 = treebase; sp1 < space;)
        *sp1++ = *sp2++;

    label(contlab);
    rcexpr(st, efftab);
    branch(l);

    return (0);
}

int pexpr()
{
    register int o, t;

    if ((o = symbol()) != LPARN)
        goto syntax;

    t = tree();

    if ((o = symbol()) != RPARN)
        goto syntax;

    return (t);

syntax:
    error("Statement syntax");
    errflush(o);
    return (0);
}

void pswitch()
{
    int *sswp, swlab;
    register int *swb, *wswp, dl;

    swb = sswp = swp;

    if (swp == 0)
        swb = swp = swtab;

    branch(swlab = isn++);
    dl = deflab;
    deflab = 0;
    statement(0);
    branch(brklab);
    label(swlab);
    putchar('#'); /* switch is pseudo-expression */
    label(brklab);

    if (!deflab)
    {
        deflab = isn++;
        label(deflab);
    }

    wswp = swp;
    putw(wswp - swb, binbuf);
    putw(deflab, binbuf);
    putw(4, binbuf); /* table 4 is switch */
    putw(line, binbuf);

    while (swb < wswp)
        putw(*swb++, binbuf);

    deflab = dl;
    swp = sswp;
}

int blkhed()
{
    register int pl;
    register struct hshtab *cs;

    autolen = 6;
    declist(0);
    pl = 4;

    while (paraml)
    {
        parame->hoffset = 0;
        cs = paraml;
        paraml = paraml->hoffset;

        if (cs->htype == FLOAT)
            cs->htype = DOUBLE;

        cs->hoffset = pl;
        cs->hclass = AUTO;

        if ((cs->htype & 030) == ARRAY)
        {
            cs->htype -= 020; /* set ptr */
            ((struct sym_list__ *)cs)->ssp++;        /* pop dims */
        }

        pl += rlength(cs);
    }

    for (cs = hshtab; cs < hshtab + hshsiz; cs++)
    {
        if (cs->name[0] == '\0')
            continue;

        /* check tagged structure */
        if (cs->hclass > KEYWC && (cs->htype & 07) == RSTRUCT)
        {
            struct sym_list__ *foo = (struct sym_list__ *)dimtab[((struct sym_list__ *)cs)->lenp & 0377];
            ((struct sym_list__ *)cs)->lenp = foo->lenp;
            cs->htype = cs->htype & ~07 | STRUCT;
        }

        if (cs->hclass == STRTAG && dimtab[((struct sym_list__ *)cs)->lenp & 0377] == 0)
            error("Undefined structure: %.8s", cs->name);

        if (cs->hclass == ARG)
            error("Not an argument: %.8s", cs->name);
    }

    osleft = ossiz;
    space = treebase;

    rcexpr(block(1, SETREG, regvar, 0, 0, 0, 0), regtab);

    return (autolen);
}

void blkend()
{
    register struct hshtab *cs;

    for (cs = hshtab; cs < hshtab + hshsiz; cs++)
    {
        if (cs->name[0])
        {
            if (cs->hclass == 0)
                error("%.8s undefined", cs->name);

            if ((cs->name[0] & 0200) == 0)
            { /* not top-level */
                cs->name[0] = '\0';
                hshused--;
            }
        }
    }
}

void errflush(int ao)
{
    register int o;

    o = ao;

    while (o > RBRACE) /* ; { } */
        o = symbol();

    peeksym = o;
}

int declist(int skwd)
{
    int o, elsize, ndec;
    register int offset, tkw, skw;

    offset = 0;
loop:
    ndec = 0;
    tkw = -1;
    skw = skwd;
    elsize = 0;

    while ((o = symbol()) == KEYW)
    {
        switch (cval)
        {
        case AUTO:
        case STATIC:
        case EXTERN:
        case REG:
            if (skw)
                error("Conflict in storage class");

            skw = cval;
            ndec++;

            if (tkw < 0)
                continue;

            goto list;

        case STRUCT:
            o = cval;
            elsize = strdec(&o, skw == MOS);
            cval = o;

        case INT:
        case CHAR:
        case FLOAT:
        case DOUBLE:
            ndec++;
            if (tkw >= 0)
                error("Type clash");
            tkw = cval;
            if (skw == 0)
                continue;
            goto list;

        default:
            goto brk1;
        }
    }

brk1:
    peeksym = o;

    if (ndec == 0)
        return (offset);

list:
    if (tkw < 0)
        tkw = INT;

    if (skw == 0)
        skw = AUTO;

    offset = declare(skw, tkw, offset, elsize);
    goto loop;
}

int strdec(int *tkwp, int mosf)
{
    register int elsize, o;
    register struct hshtab *ssym;
    struct hshtab *ds;

    mosflg = 1;
    ssym = 0;

    if ((o = symbol()) == NAME)
    {
        ssym = csym;

        if (ssym->hclass == 0)
        {
            ssym->hclass = STRTAG;
            ((struct sym_list__ *)ssym)->lenp = dimp;
            chkdim();
            dimtab[dimp++] = 0;
        }

        if (ssym->hclass != STRTAG)
            redec();

        mosflg = mosf;
        o = symbol();
    }

    mosflg = 0;

    if (o != LBRACE)
    {
        if (ssym == 0)
        {
        syntax:
            decsyn(o);
            return (0);
        }

        if (ssym->hclass != STRTAG)
            error("Bad structure name");

        if ((elsize = dimtab[((struct sym_list__ *)ssym)->lenp & 0377]) == 0)
        {
            *tkwp = RSTRUCT;
            elsize = ssym;
        }

        peeksym = o;
    }
    else
    {
        ds = defsym;
        mosflg = 0;
        elsize = declist(MOS);

        if (elsize & 01)
            elsize++;

        defsym = ds;

        if ((o = symbol()) != RBRACE)
            goto syntax;

        if (ssym)
        {
            if (dimtab[((struct sym_list__ *)ssym)->lenp & 0377])
                error("%.8s redeclared", ssym->name);

            dimtab[((struct sym_list__ *)ssym)->lenp & 0377] = elsize;
        }
    }

    return (elsize);
}

void chkdim()
{
    if (dimp >= dimsiz)
    {
        error("Dimension/struct table overflow");
        exit(1);
    }
}
