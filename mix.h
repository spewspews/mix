typedef struct Sym Sym;
typedef struct Refinst Refinst;
typedef struct Wval Wval;
typedef struct Con Con;

struct Sym {
	Avl;
	char *name;
	int lex;
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
	Sym *s;
	int exp;
	Con *link;
};

long yylex(void);
int yyparse(void);
void yyerror(char*);
int skipto(char);

Rune mixtorune(int);
int runetomix(Rune);

void error(char*, ...);
void warn(char*, ...);
void *emalloc(ulong);
void *emallocz(ulong);
void *ecalloc(ulong, ulong);
void *erealloc(void*, ulong);
Rune *erunestrdup(Rune*);
void efmtprint(Fmt*, char*, ...);

int star;
char *filename;
int line;

int mem[4000];

#define F(a, b) 8*(a)+(b)

enum {
	MASK1 = 0x3f,
	MASK2 = 0xfff,
	MASK3 = 0x3fff,
	MASK4 = 0xffffff,
	MASK5 = 0x3fffffff,
	SIGNB = 1<<31,
};
