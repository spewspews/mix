#include <u.h>
#include <libc.h>
#include <ctype.h>
#include <bio.h>
#include <avl.h>
#include "mix.h"
#include "y.tab.h"

Avltree *syms;

struct Resvd {
	char *name;
	long lex;
	int c;
	int f;
} res[] = {
	{ "NOP",	LOP,	0,	F(0, 5) },
	{ "ADD",	LOP,	1,	F(0, 5) },
	{ "FADD",	LOP,	1,	6 },
	{ "SUB",	LOP,	2,	F(0, 5) },
	{ "FSUB",	LOP,	2,	6 },
	{ "MUL",	LOP,	3,	F(0, 5) },
	{ "FMUL",	LOP,	3,	6 },
	{ "DIV",	LOP,	4,	F(0, 5) },
	{ "FDIV",	LOP,	4,	6 },
	{ "NUM",	LOP,	5,	0 },
	{ "CHAR",	LOP,	5,	1 },
	{ "HLT",	LOP,	5,	2 },
	{ "SLA",	LOP,	6,	0 },
	{ "SRA",	LOP,	6,	1 },
	{ "SLAX",	LOP,	6,	2 },
	{ "SRAX",	LOP,	6,	3 },
	{ "SLC",	LOP,	6,	4 },
	{ "SRC",	LOP,	6,	5 },
	{ "MOVE",	LOP,	7,	1 },
	{ "LDA",	LOP,	8,	F(0, 5) },
	{ "LD1",	LOP,	9,	F(0, 5) },
	{ "LD2",	LOP,	10,	F(0, 5) },
	{ "LD3",	LOP,	11,	F(0, 5) },
	{ "LD4",	LOP,	12,	F(0, 5) },
	{ "LD5",	LOP,	13,	F(0, 5) },
	{ "LD6",	LOP,	14,	F(0, 5) },
	{ "LDX",	LOP,	15,	F(0, 5) },
	{ "LDAN",	LOP,	16,	F(0, 5) },
	{ "LD1N",	LOP,	17,	F(0, 5) },
	{ "LD2N",	LOP,	18,	F(0, 5) },
	{ "LD3N",	LOP,	19,	F(0, 5) },
	{ "LD4N",	LOP,	20,	F(0, 5) },
	{ "LD5N",	LOP,	21,	F(0, 5) },
	{ "LD6N",	LOP,	22,	F(0, 5) },
	{ "LDXN",	LOP,	23,	F(0, 5) },
	{ "STA",	LOP,	24,	F(0, 5) },
	{ "ST1",	LOP,	25,	F(0, 5) },
	{ "ST2",	LOP,	26,	F(0, 5) },
	{ "ST3",	LOP,	27,	F(0, 5) },
	{ "ST4",	LOP,	28,	F(0, 5) },
	{ "ST5",	LOP,	29,	F(0, 5) },
	{ "ST6",	LOP,	30,	F(0, 5) },
	{ "STX",	LOP,	31,	F(0, 5) },
	{ "STJ",	LOP,	32,	F(0, 2) },
	{ "STZ",	LOP,	33,	F(0, 5) },
	{ "JBUS",	LOP,	34, 0 },
	{ "IOC",	LOP,	35, 0 },
	{ "IN",	LOP,	36,	0 },
	{ "OUT",	LOP,	37,	0 },
	{ "JRED",	LOP,	38,	0 },
	{ "JMP",	LOP,	39, 0 },
	{ "JSJ",	LOP,	39, 1 },
	{ "JOV",	LOP,	39, 2 },
	{ "JNOV",	LOP,	39, 3 },
	{ "JL",	LOP,	39,	4 },
	{ "JE",	LOP,	39,	5 },
	{ "JG",	LOP,	39,	6 },
	{ "JGE",	LOP,	39,	7 },
	{ "JNE",	LOP,	39,	8 },
	{ "JLE",	LOP,	39,	9 },
	{ "JAN",	LOP,	40,	0 },
	{ "JAZ",	LOP,	40,	1 },
	{ "JAP",	LOP,	40,	2 },
	{ "JANN",	LOP,	40,	3 },
	{ "JANZ",	LOP,	40,	4 },
	{ "JANP",	LOP,	40,	5 },
	{ "J1N",	LOP,	41,	0 },
	{ "J1Z",	LOP,	41,	1 },
	{ "J1P",	LOP,	41,	2 },
	{ "J1NN",	LOP,	41,	3 },
	{ "J1NZ",	LOP,	41,	4 },
	{ "J1NP",	LOP,	41,	5 },
	{ "J2N",	LOP,	42,	0 },
	{ "J2Z",	LOP,	42,	1 },
	{ "J2P",	LOP,	42,	2 },
	{ "J2NN",	LOP,	42,	3 },
	{ "J2NZ",	LOP,	42,	4 },
	{ "J2NP",	LOP,	42,	5 },
	{ "J3N",	LOP,	43,	0 },
	{ "J3Z",	LOP,	43,	1 },
	{ "J3P",	LOP,	43,	2 },
	{ "J3NN",	LOP,	43,	3 },
	{ "J3NZ",	LOP,	43,	4 },
	{ "J3NP",	LOP,	43,	5 },
	{ "J4N",	LOP,	44,	0 },
	{ "J4Z",	LOP,	44,	1 },
	{ "J4P",	LOP,	44,	2 },
	{ "J4NN",	LOP,	44,	3 },
	{ "J4NZ",	LOP,	44,	4 },
	{ "J4NP",	LOP,	44,	5 },
	{ "J5N",	LOP,	45,	0 },
	{ "J5Z",	LOP,	45,	1 },
	{ "J5P",	LOP,	45,	2 },
	{ "J5NN",	LOP,	45,	3 },
	{ "J5NZ",	LOP,	45,	4 },
	{ "J5NP",	LOP,	45,	5 },
	{ "J6N",	LOP,	46,	0 },
	{ "J6Z",	LOP,	46,	1 },
	{ "J6P",	LOP,	46,	2 },
	{ "J6NN",	LOP,	46,	3 },
	{ "J6NZ",	LOP,	46,	4 },
	{ "J6NP",	LOP,	46,	5 },
	{ "JXN",	LOP,	47,	0 },
	{ "JXZ",	LOP,	47,	1 },
	{ "JXP",	LOP,	47,	2 },
	{ "JXNN",	LOP,	47,	3 },
	{ "JXNZ",	LOP,	47,	4 },
	{ "JXNP",	LOP,	47,	5 },
	{ "INCA",	LOP,	48,	0 },
	{ "DECA",	LOP,	48,	1 },
	{ "ENTA",	LOP,	48,	2 },
	{ "ENNA",	LOP,	48,	3 },
	{ "INC1",	LOP,	49,	0 },
	{ "DEC1",	LOP,	49,	1 },
	{ "ENT1",	LOP,	49,	2 },
	{ "ENN1",	LOP,	49,	3 },
	{ "INC2",	LOP,	50,	0 },
	{ "DEC2",	LOP,	50,	1 },
	{ "ENT2",	LOP,	50,	2 },
	{ "ENN2",	LOP,	50,	3 },
	{ "INC3",	LOP,	51,	0 },
	{ "DEC3",	LOP,	51,	1 },
	{ "ENT3",	LOP,	51,	2 },
	{ "ENN3",	LOP,	51,	3 },
	{ "INC4",	LOP,	52,	0 },
	{ "DEC4",	LOP,	52,	1 },
	{ "ENT4",	LOP,	52,	2 },
	{ "ENN4",	LOP,	52,	3 },
	{ "INC5",	LOP,	53,	0 },
	{ "DEC5",	LOP,	53,	1 },
	{ "ENT5",	LOP,	53,	2 },
	{ "ENN5",	LOP,	53,	3 },
	{ "INC6",	LOP,	54,	0 },
	{ "DEC6",	LOP,	54,	1 },
	{ "ENT6",	LOP,	54,	2 },
	{ "ENN6",	LOP,	54,	3 },
	{ "INCX",	LOP,	55,	0 },
	{ "DECX",	LOP,	55,	1 },
	{ "ENTX",	LOP,	55,	2 },
	{ "ENNX",	LOP,	55,	3 },
	{ "CMPA",	LOP,	56,	F(0, 5) },
	{ "FCMP",	LOP,	56,	6 },
	{ "CMP1",	LOP,	57,	F(0, 5) },
	{ "CMP2",	LOP,	58,	F(0, 5) },
	{ "CMP3",	LOP,	59,	F(0, 5) },
	{ "CMP4",	LOP,	60,	F(0, 5) },
	{ "CMP5",	LOP,	61,	F(0, 5) },
	{ "CMP6",	LOP,	62,	F(0, 5) },
	{ "CMPX",	LOP,	63,	F(0, 5) },
	{ "EQU",	LEQU,	-1,	-1 },
	{ "ORIG",	LORIG,	-1,	-1 },
	{ "CON",	LCON,	-1,	-1 },
	{ "ALF",	LALF,	-1,	-1 },
	{ "END",	LEND,	-1,	-1 },
};

int mask[] = {
	MASK1,
	MASK2,
	MASK3,
	MASK4,
	MASK5
};

int symcmp(Avl*, Avl*);
Sym *sym(char*);
Biobuf bin;

void
main(int argc, char **argv)
{
	ARGBEGIN {
	} ARGEND

	Binit(&bin, 0, OREAD);
	line = 0;
	cinit();
	sinit();
	if(yyparse() != 0) {
		fprint(2, "Parsing failed\n");
		exits("Parsing");
	}
	mixvm();
}

void
sinit(void)
{
	struct Resvd *r;
	Sym *s;

	syms = avlcreate(symcmp);
	for(r = res; r < res + nelem(res); r++) {
		s = sym(r->name);
		s->lex = r->lex;
		s->opc = r->c;
		s->f = r->f;
		avlinsert(syms, s);
	}
}

long
yylex(void)
{
	static Rune buf[11];
	Rune r, *bp, *ep;
	static char cbuf[100];
	int isnum;

	if(yydone)
		return -1;
	r = Bgetrune(&bin);
	switch(r) {
	case Beof:
		return -1;
	case '\t':
		return ' ';
	case '\n':
		line++;
	case '+':
	case '-':
	case '*':
	case ':':
	case ',':
	case '(':
	case ')':
	case '=':
	case ' ':
		return r;
	case '/':
		r = Bgetrune(&bin);
		if(r == '/')
			return LSS;
		else
			Bungetrune(&bin);
		return '/';
	case '#':
		skipto('\n');
		return '\n';
	}
	bp = buf;
	ep = buf+nelem(buf)-1;
	isnum = 1;
	for(;;) {
//		print("forming sym %C\n", r);
		if(runetomix(r) == -1) 
			error("Invalid character");
		if(bp == ep)
			error("Symbol or number too long");
		*bp++ = r;
		if(isnum && (r >= Runeself || !isdigit(r)))
			isnum = 0;
		r = Bgetrune(&bin);
		switch(r) {
		case Beof:
		case '\t':
		case '\n':
		case '+':
		case '-':
		case '*':
		case ':':
		case ',':
		case '(':
		case ')':
		case '=':
		case ' ':
		case '/':
			Bungetrune(&bin);
			*bp = '\0';
			goto End;
		}
	}
End:
	seprint(cbuf, cbuf+100, "%S", buf);
	if(isnum) {
		yylval.lval = strtol(cbuf, nil, 10);
		return LNUM;
	}
	yylval.sym = sym(cbuf);
//	print("yylex %s %ld\n", yylval.sym->name, yylval.sym->lex);
	return yylval.sym->lex;
}

void
yyerror(char *e)
{
	error(e);
}

Sym*
sym(char *name)
{
	Sym *s, l;

	l.name = name;
	s = (Sym*)avllookup(syms, &l);
	if(s != nil)
		return s;

	s = emallocz(sizeof(*s) + strlen(name));
	strcpy(s->nbuf, name);
	s->name = s->nbuf;
	s->lex = LSYMREF;
	avlinsert(syms, s);
	return s;
}

int
symcmp(Avl *a, Avl *b)
{
	Sym *sa, *sb;

	sa = (Sym*)a;
	sb = (Sym*)b;
	return strcmp(sa->name, sb->name);
}

void
skipto(char c)
{
	Rune r;

	for(;;) {
		r = Bgetrune(&bin);
		if(r != c)
			continue;
		return;
	}
}

int
disasm(int l, int *sign, int *apart, int *ipart, int *fpart)
{
	int inst, opc;

	inst = cells[l];
	print("%d\n", inst);
	*sign = inst&SIGNB;
	opc = inst & MASK1;
	inst >>= BITS;
	*fpart = inst & MASK1;
	inst >>= BITS;
	*ipart = inst & MASK1;
	inst >>= BITS;
	*apart = inst & MASK2;

	return opc;
}

void dovm(int);

void
mixvm(void)
{
	dovm(vmstart);
}

void
query(void)
{
	char buf[512];
	long l, r;
	int sign, apart, ipart, fpart, opc;

	for(;;) {
		print("μ ");
		r = read(0, buf, sizeof(buf));
		if(r <= 0)
			exits(nil);
		if(r == 1)
			continue;
		buf[r] = '\0';
		if(buf[0] == 'r') {
			switch(buf[1]) {
			case 'a':
				print("%d\n", RAVAL);
				break;
			case 'x':
				print("%d\n", RXVAL);
				break;
			}
			continue;
		}
		l = strtol(buf, nil, 10);
		opc = disasm(l, &sign, &apart, &ipart, &fpart);
		print("%d\t%d,%d(%d)\n", opc, apart, ipart, fpart);
	}
}

int
V(int c, int f)
{
	u32int word;
	int sign, a, b, m;

	UNF(a, b, f);

	if(a > 5 || b > 5)
		error("Invalid fpart");

	word = cells[c];

	if(a == 0) {
		sign = word&SIGNB;
		a = 1;
	} else
		sign = 0;

	m = b - a;
	if(m < 0 || m > 4)
		error("Invalid fpart");

	word >>= ((5-b)*BITS);
	word &= mask[m];
	return sign ? -(int)word : word;
}

void mixfadd(int){}

void
mixadd(int inst)
{
	int rval;
	UNPACK(inst);

//	print("mixadd ra: %d\n", ra);
	rval = RAVAL;
//	print("mixadd raval: %d\n", rval);
	rval += V(m, f);
//	print("mixadd result: %d\n", rval);
	ra = rval < 0 ? -rval|SIGNB : rval;
	if(ra & OVERB) {
		ra &= ~OVERB;
		ot = 1;
	}
}

void mixfsub(int){}

void
mixsub(int inst)
{
	int rval;
	UNPACK(inst);

	rval = RAVAL;
	rval += V(m, f);
	ra = rval < 0 ? -rval|SIGNB : rval;
	if(ra & OVERB) {
		ra &= ~OVERB;
		ot = 1;
	}
}

void mixfmul(int){}

void
mixmul(int inst)
{
	vlong rval;
	u32int signb;
	UNPACK(inst);

	rval = RAVAL;
	rval *= V(m, f);

	if(rval < 0) {
		rval = -rval;
		signb = SIGNB;
	} else
		signb = ~SIGNB;

	ra = rval>>5 & MASK5 & signb;
	rx = rval & MASK5 & signb;
}

void mixfdiv(int){}

void mixdiv(int inst)
{
	vlong divend;
	u32int xsignb, signb;
	int quot, rem, v;

	UNPACK(inst);

	v = V(m, f);
	if(v == 0) {
		ot = 1;
		return;
	}
	divend = ra & MASK5;
	divend <<= BITS * 5;
	divend |= rx & MASK5;
	if(ra & SIGNB)
		divend = -divend;

	quot = divend / v;
	rem = divend % v;

	if(quot < 0) {
		quot = -quot;
		signb = SIGNB;
	} else
		signb = ~SIGNB;

	if(rem < 0) {
		rem = -rem;
		xsignb = SIGNB;
	} else
		xsignb = ~SIGNB;

	ra = quot & signb;
	rx = rem & xsignb;
	if(ra & OVERB) {
		ra &= ~OVERB;
		ot = 1;
	}
}

void
mixnum(void)
{
	int i, b;
	u32int n;

	n = 0;
	for(i = 0; i < 5; i++) {
		b = ra>>(4-i)*BITS & MASK1;
		b %= 10;
		n = 10*n + b;
	}
	for(i = 0; i < 5; i++) {
		b = rx>>(4-i)*BITS & MASK1;
		b %= 10;
		n = 10*n + b;
	}
	ra &= ~MASK5;
	ra |= n & MASK5;
}

void
mixchar(void)
{
	int i;
	u32int a, val;

	val = ra;
	for(i = 0; i < 5; i++) {
		a = val % 10;
		a += 30;
		rx &= ~(MASK1<<i);
		rx |= a<<i;
		val /= 10;
	}
	for(i = 0; i < 5; i++) {
		a = val % 10;
		a += 30;
		ra &= ~(MASK1<<i);
		ra |= a<<i;
		val /= 10;
	}
}

void mixhalt(void)
{
	query();
}

void mixsla(int){}
void mixsra(int){}
void mixslax(int){}
void mixsrax(int){}
void mixslc(int){}
void mixsrc(int){}
void mixmove(int){}
void mixlda(int){}
void mixldi(int){}
void mixldx(int){}
void mixldan(int){}
void mixldin(int){}
void mixldxn(int){}
void mixsta(int){}
void mixsti(int){}
void mixstx(int){}
int mixjbus(int){ return 0; }
void mixioc(int){}
void mixin(int){}
void mixout(int){}

void
dovm(int ip)
{
	int inst;

Top:
	for (;;) {
		inst = cells[ip];
		switch(FC(inst)) {
		default:
			fprint(2, "Bad op!\n");
			exits("error");
		case 0:
			break;
		case 1:
			if(FF(inst) == 6)
				mixfadd(inst);
			else
				mixadd(inst);
			break;
		case 2:
			if(FF(inst) == 6)
				mixfsub(inst);
			else
				mixsub(inst);
			break;
		case 3:
			if(FF(inst) == 6)
				mixfmul(inst);
			else
				mixmul(inst);
			break;
		case 4:
			if(FF(inst) == 6)
				mixfdiv(inst);
			else
				mixdiv(inst);
			break;
		case 5:
			switch(FF(inst)) {
			default:
				error("Bad instruction");
			case 0:
				mixnum();
				break;
			case 1:
				mixchar();
				break;
			case 2:
				mixhalt();
				break;
			}
			break;
		case 6:
			switch(FF(inst)) {
			default:
				error("Bad instruction");
			case 0:
				mixsla(inst);
				break;
			case 1:
				mixsra(inst);
				break;
			case 2:
				mixslax(inst);
				break;
			case 4:
				mixsrax(inst);
				break;
			case 5:
				mixslc(inst);
				break;
			case 6:
				mixsrc(inst);
				break;
			}
			break;
		case 7:
			mixmove(inst);
			break;
		case 8:
			mixlda(inst);
			break;
		case 9: case 10: case 11:
		case 12: case 13: case 14:
			mixldi(inst);
			break;
		case 15:
			mixldx(inst);
			break;
		case 16:
			mixldan(inst);
			break;
		case 17: case 18: case 19:
		case 20: case 21: case 22:
			mixldin(inst);
			break;
		case 23:
			mixldxn(inst);
			break;
		case 24:
			mixsta(inst);
			break;
		case 25: case 26: case 27:
		case 28: case 29: case 30:
			mixsti(inst);
			break;
		case 31:
			mixstx(inst);
			break;
		case 32:
			ip = mixjbus(inst);
			goto Top;
		case 33:
			mixioc(inst);
			break;
		case 34:
			mixin(inst);
			break;
		case 35:
			mixout(inst);
			break;
		}
		ip++;
	}
}
