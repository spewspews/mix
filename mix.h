typedef struct Sym Sym;
typedef struct Refinst Refinst;
typedef struct Wval Wval;
typedef struct Con Con;

struct Sym {
	Avl;
	char *name;
	long lex;
	union {
		struct {
			int opc, f;	/* LOP */
		};
		int val;	/* LSYMDEF */
		struct {
			int *refs, i, max;	/* LSYMREF */
		};
	};
	char nbuf[1];
};

struct Con {
	Sym *sym;
	int exp;
	Con *link;
};

void mixvm(void);

long yylex(void);
int yyparse(void);
void yyerror(char*);
void skipto(char);
Sym *sym(char*);
void sinit(void);

Rune mixtorune(int);
int runetomix(Rune);
void cinit(void);

void error(char*, ...);
void warn(char*, ...);
void *emalloc(ulong);
void *emallocz(ulong);
void *ecalloc(ulong, ulong);
void *erealloc(void*, ulong);
Rune *erunestrdup(Rune*);
void efmtprint(Fmt*, char*, ...);

int star;
Con *cons;
char *filename;
int line;
int vmstart;
int yydone;
extern int mask[5];
u32int mem[4000];

#define F(a, b) 8*(a)+(b)
#define UNF(a, b, f) (a) = f/8; (b) = f%8

enum {
	BITS = 6,
	MASK1 = 63,
	MASK2 = (63<<6) | MASK1,
	MASK3 = (63<<12) | MASK2,
	MASK4 = (63<<18) | MASK3,
	MASK5 = (63<<24) | MASK4,
	SIGNB = 1<<31,
};
