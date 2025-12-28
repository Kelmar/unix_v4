#
/*
 * C compiler-- pass 1 header
 *
 *	Copyright 1973 Bell Telephone Laboratories, Inc.
 */

/*
 *  parameters
 */

#define	ncps   8
#define	hshsiz 200
#define	cmsiz  40
#define	swsiz  200
#define	ncpw   2
#define	ossiz  500
#define	dimsiz 100

#define	regtab 0
#define	efftab 1
#define	cctab  2
#define	sptab  3

struct tnode
{
    int op;
    int type;
    int dimp;
    struct tnode *tr1, *tr2;
};

/*
 * Not sure what this should be named, it was an anonymous struct previously.
 *
 *          -- B.Simonds (Dec 27, 2025)
 */
struct sym_list__
{
    int fop;
    int ftype;
    char ssp;  /* subscript list */
    char lenp; /* structure length */
};

struct tname
{
    int nop;
    int ntype;
    int ndimp;

    int class;
    int offset;
    int nloc;
};

struct tconst
{
    int cop;
    int ctype;
    int cdimp;
    int value;
};

struct hshtab
{
    int hclass;
    int htype;
    int hdimp;
    int hoffset;
    char name[ncps];
};

struct tnode_new
{
    int op;   // Same as nop, cop, hclass
    int type; // Same as ntype, ctype, htype
    int dimp; // Same as ndimp, cdimp, hdimp

    union
    {
        struct /* Reserving space? */
        {
            struct tnode *tr1;
            struct tnode *tr2;
        };
        struct /* tname */
        {
            int class;
            int offset;
            int nloc;
        };
        struct /* tconst */
        {
            int value;
        };
        struct
        {
            int hoffset;
            char name[ncps];
        };
    };
};

struct swtab
{
    int swlab;
    int swval;
};

extern char cvtab[];
extern int opdope[];
extern char ctab[];
extern char symbuf[ncps];
extern int hshused;
extern struct hshtab hshtab[hshsiz];
extern int *space;

/* Pointer to a pointer? */
extern int *cp;

extern int cmst[cmsiz];
extern int isn;
extern struct swtab swtab[swsiz];
extern struct swtab *swp;
extern int contlab;
extern int brklab;
extern int retlab;
extern int deflab;
extern int nauto;
extern int autolen;
extern int peeksym;
extern int peekc; /* Look ahead character? */
extern int eof;
extern int line;
extern int *treebase;
extern int debug;
extern struct hshtab *defsym;
extern struct hshtab *funcsym;
extern int xdflg;
extern int proflg;
extern struct hshtab *csym;
extern char cval;
extern double fcval;
extern int nchstr;
extern int nerror;
extern struct hshtab *paraml;
extern struct hshtab *parame;
extern int strflg;
extern int osleft;
extern int mosflg;
extern int initflg;
extern int inhdr;
extern int dimtab[dimsiz];
extern char binbuf[518];
extern char ascbuf[518];
extern int dimp;
extern int regvar;

/*
 * operators
 */
#define	OP_EOF  0
#define	SEMI    1
#define	LBRACE  2
#define	RBRACE  3
#define	LBRACK  4
#define	RBRACK  5
#define	LPARN   6
#define	RPARN   7
#define	COLON   8
#define	COMMA   9

#define	KEYW    19
#define	NAME    20
#define	CON     21
#define	STRING  22
#define	FCON    23
#define	SFCON   24

#define	SIZEOF  29
#define	INCBEF  30
#define	DECBEF  31
#define	INCAFT  32
#define	DECAFT  33
#define	EXCLA   34
#define	AMPERA  29
#define	AMPER   35
#define	STAR    36
#define	NEG     37
#define	COMPL   38

#define	DOT     39
#define	PLUS    40
#define	MINUS   41
#define	TIMES   42
#define	DIVIDE  43
#define	MOD     44
#define	RSHIFT  45
#define	LSHIFT  46
#define	AND     47
#define	OR      48
#define	EXOR    49
#define	ARROW   50
#define	ITOF    51
#define	FTOI    52
#define	LOGAND  53
#define	LOGOR   54

#define	EQUAL   60
#define	NEQUAL  61
#define	LESSEQ  62
#define	LESS    63
#define	GREATEQ 64
#define	GREAT   65
#define	LESSP   66
#define	LESSEQP 67
#define	GREATP  68
#define	GREATQP 69

#define	ASPLUS  70
#define	ASMINUS 71
#define	ASTIMES 72
#define	ASDIV   73
#define	ASMOD   74
#define	ASRSH   75
#define	ASLSH   76
#define	ASSAND  77
#define	ASOR    78
#define	ASXOR   79
#define	ASSIGN  80

#define	QUEST   90
#define	CALL    100
#define	MCALL   101
#define	JUMP    102
#define	CBRANCH 103
#define	INIT    104
#define	SETREG  105
#define	RFORCE  110
#define	BRANCH  111
#define	LABEL   112

/*
 * types
 */
#define	INT     0
#define	CHAR    1
#define	FLOAT   2
#define	DOUBLE  3
#define	STRUCT  4
#define	RSTRUCT 5
#define	PTR     010
#define	FUNC    020
#define	ARRAY   030

/*
 * storage classes
 */
#define	KEYWC   1
#define	MOS     4
#define	AUTO    5
#define	EXTERN  6
#define	STATIC  7
#define	REG     8
#define	STRTAG  9
#define ARG     10
#define	ARG1    11

/*
 * keywords
 */
#define	GOTO    10
#define	RETURN  11
#define	IF      12
#define	WHILE   13
#define	ELSE    14
#define	SWITCH  15
#define	CASE    16
#define	BREAK   17
#define	CONTIN  18
#define	DO      19
#define	DEFAULT 20
#define	FOR     21

/*
 * characters
 */
#define	INSERT  119
#define	PERIOD  120
#define	SQUOTE  121
#define	DQUOTE  122
#define	LETTER  123
#define	DIGIT   124
#define	NEWLN   125
#define	SPACE   126
#define	UNKN    127

/*
 * Flag bits
 */
#define	BINARY  01
#define	LVALUE  02
#define	RELAT   04
#define	ASSGOP  010
#define	LWORD   020
#define	RWORD   040
#define	COMMUTE 0100
#define	RASSOC  0200
#define	LEAF    0400


/*
 * Prototypes
 */
/* Polyfill code */
int fcreat(const char *path, const char *buf);
void putw_old(int *, char *);
void fflush_old(char buffer[518]);

/* Other functions */
void extdef();
void blkend();
int chkdim();
int length(struct hshtab *dsym);
int rlength(struct hshtab *dsym);
int plength(struct tnode *node);
int getype();
void errflush(int o);
void build(int o);
void pblock(int o);
int nextchar();
int getnum(int base);

/* Defined in c00.c */
struct tnode *tree();

/* Defined in c01.c */
void build(int op);

int *block(int num, int op, int t, int d, int p1, int p2, int p3);
void error(char *fmt, ...);

int conexp();
