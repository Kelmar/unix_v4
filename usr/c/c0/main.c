
#
/*
 * C compiler
 * Copyright 1972 Bell Telephone Laboratories, Inc.
 * 
 * Stage 1
 */

#include <fcntl.h>

#include "c0.h"

/*************************************************************************/
/*
 * Main entry point for the Stage 1 portion of the compiler.
 */

void main(int argc, char* argv[])
{
    /*
     * In the Unix V4 world the external fin variable is effectively the
     * variable that holds the stdin file descriptor.
     */
    //extern int fin = 0;
    int fin = 0;

    int treespace[ossiz];
    register char *sp, *np;
    register struct kwtab *ip;
    struct hshtab *sym;

    if (argc < 4)
    {
        error("Arg count");
        exit(1);
    }

    if ((fin = open(argv[1], 0)) < 0)
    {
        error("Can't find %s", argv[1]);
        exit(1);
    }

    if (fcreat(argv[2], ascbuf) < 0 || fcreat(argv[3], binbuf) < 0)
    {
        error("Can't create temp");
        exit(1);
    }

    if (argc > 4)
        proflg++;
        
    xdflg++;

    /*
     * Initialize the keywords into the hash table.
     */
    for (ip = kwtab; (np = ip->kwname); ip++)
    {
        for (sp = symbuf; sp < symbuf + ncps;)
        {
            /* 
             * This odd code blanks out the entirity of symbuf 
             * so the hash always comes out the same.
             */
            if ((*sp++ = *np++) == '\0')
                np--;
        }

        sym = lookup();
        sym->hclass = KEYWC;
        sym->htype = ip->kwval;
    }

    xdflg = 0;
    treebase = treespace + 10;
    putw_old(treebase, binbuf);

    while (!eof)
    {
        extdef();
        blkend();
    }

    fflush_old(ascbuf);
    fflush_old(binbuf);

    exit(nerror != 0);
}
