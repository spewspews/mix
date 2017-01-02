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
		u32int mval;	/* LSYMDEF */
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
void *erealloc(void*, ulong);

int star;
Con *cons;
char *filename;
int line;
int vmstart;
int yydone;
extern int mask[5];
u32int cells[4000];

u32int ra, rx, r[7];
int ce, cl, cg, ot;

#define RJ ri[0]
#define F(a, b) 8*(a)+(b)
#define UNF(a, b, f) ((a) = f/8, (b) = f%8)
#define M(a, i) ((i) == 0 ? (a) : (a)+r[(i)])
#define FC(inst) ((inst)&MASK1)
#define FF(inst) ((inst)>>BITS & MASK1)
#define FI(inst) ((inst)>>BITS*2 & MASK1)
#define FA(inst)	\
	(int)((inst)&SIGNB ?	\
		-(int)((inst)>>BITS*3 & MASK2) :	\
		((inst)>>BITS*3 & MASK2))
#define RAVAL ra & SIGNB ? -(int)(ra&MASK5) : ra&MASK5
#define RXVAL rx & SIGNB ? -(int)(rx&MASK5) : rx&MASK5
#define RIVAL(i) r[(i)]&SIGNB ? -(int)(r[(i)]&MASK2) : r[(i)]&MASK2
#define UNPACK(inst) \
	int a, i, f, m, ival;	\
	a = FA(inst);	\
	i = FI(inst);	\
	f = FF(inst);	\
	ival = RIVAL(i);	\
	m = M(a, ival);

enum {
	BITS = 6,
	MASK1 = 63,
	MASK2 = (63<<6) | MASK1,
	MASK3 = (63<<12) | MASK2,
	MASK4 = (63<<18) | MASK3,
	MASK5 = (63<<24) | MASK4,
	OVERB = 1<<30,
	SIGNB = 1<<31,
};
