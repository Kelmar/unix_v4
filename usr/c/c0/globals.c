#

/*
 * Holds all the variables that were previously in the c0h.c file.
 */

#include "c0.h"

char symbuf[ncps]; /* Symbol name buffer */
int hshused;
struct hshtab hshtab[hshsiz];
int *space;

/* Pointer to a pointer? */
int *cp;

int cmst[cmsiz];
struct swtab swtab[swsiz];
struct swtab *swp;
int contlab;
int brklab;
int retlab;
int deflab;
int nauto;
int autolen;
int peekc; /* Look ahead character? */
int eof;
int *treebase;
struct hshtab *defsym;
struct hshtab *funcsym;
int xdflg;
int proflg;
struct hshtab *csym;
char cval;
double fcval;
int nchstr;
int nerror;
struct hshtab *paraml;
struct hshtab *parame;
int strflg;
int osleft;
int mosflg;
int initflg;
int inhdr;
int dimtab[dimsiz];
char binbuf[518];
char ascbuf[518];
int regvar;
