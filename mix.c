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
	{ "1H",	LHERE,	1,	-1 },
	{ "2H",	LHERE,	2,	-1 },
	{ "3H",	LHERE,	3,	-1 },
	{ "4H",	LHERE,	4,	-1 },
	{ "5H",	LHERE,	5,	-1 },
	{ "6H",	LHERE,	6,	-1 },
	{ "7H",	LHERE,	7,	-1 },
	{ "8H",	LHERE,	8,	-1 },
	{ "9H",	LHERE,	9,	-1 },
	{ "1B",	LBACK,	1,	-1 },
	{ "2B",	LBACK,	2,	-1 },
	{ "3B",	LBACK,	3,	-1 },
	{ "4B",	LBACK,	4,	-1 },
	{ "5B",	LBACK,	5,	-1 },
	{ "6B",	LBACK,	6,	-1 },
	{ "7B",	LBACK,	7,	-1 },
	{ "8B",	LBACK,	8,	-1 },
	{ "9B",	LBACK,	9,	-1 },
	{ "1F",	LFORW,	1,	-1 },
	{ "2F",	LFORW,	2,	-1 },
	{ "3F",	LFORW,	3,	-1 },
	{ "4F",	LFORW,	4,	-1 },
	{ "5F",	LFORW,	5,	-1 },
	{ "6F",	LFORW,	6,	-1 },
	{ "7F",	LFORW,	7,	-1 },
	{ "8F",	LFORW,	8,	-1 },
	{ "9F",	LFORW,	9,	-1 },
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
	mixvm(vmstart);
	mixquery();
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
	*sign = inst>>31;
	opc = inst & MASK1;
	inst >>= BITS;
	*fpart = inst & MASK1;
	inst >>= BITS;
	*ipart = inst & MASK1;
	inst >>= BITS;
	*apart = inst & MASK2;

	return opc;
}

int
mval(u32int a, int s, u32int m)
{
	int sign, val;

	sign = a >> 31;
	val = a>>s*BITS & m;
	if(sign)
		return -val;
	return val;
}

void
mixquery(void)
{
	char buf[512];
	long l, r;
	vlong rax;
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
				if(buf[2] == 'x') {
					rax = ra & MASK5;
					rax <<= 5 * BITS;
					rax |= rx & MASK5;
					if(ra >> 31)
						rax = -rax;
					print("%lld\n", rax);
				} else
					print("%d\n", mval(ra, 0, MASK5));
				break;
			case 'x':
				print("%d\n", mval(rx, 0, MASK5));
				break;
			case 'j':
				print("%d\n", mval(ri[0], 0, MASK2));
				break;
			default:
				if(!isdigit(buf[1]))
					break;
				print("%d\n", mval(ri[buf[1]-'0'], 0, MASK2));
			}
			continue;
		}
		if(isdigit(buf[0])) {
			l = strtol(buf, nil, 10);
			opc = disasm(l, &sign, &apart, &ipart, &fpart);
			print("%d\n", mval(cells[l], 0, MASK5));
			print("%d\t%d,%d(%d)\n", opc, apart, ipart, fpart);
		}
	}
}

int
M(int a, int i)
{
	int off, r;

	r = ri[i] & ~(MASK3<<2*BITS);
	off = i == 0 ? 0 : mval(r, 0, MASK2);
	return a + off;
}

int
V(u32int w, int f)
{
	int a, b, d;

	if(f == 0)
		return 0;

	UNF(a, b, f);
	if(a > 0)
		w &= ~SIGNB;
	else
		a++;

	d = b - a;
	if(a > 5 || b > 5 || d < 0 || d > 4)
		error("Invalid fpart");

	return mval(w, 5-b, mask[d]);
}

void mixfadd(int){}

void
mixadd(int m, int f)
{
	int rval;
	
	rval = mval(ra, 0, MASK5);
	rval += V(cells[m], f);
	ra = rval < 0 ? -rval|SIGNB : rval;
	if(ra & OVERB) {
		ra &= ~OVERB;
		ot = 1;
	}
}

void mixfsub(int){}

void
mixsub(int m, int f)
{
	int rval;

	rval = mval(ra, 0, MASK5);
	rval -= V(cells[m], f);
	ra = rval < 0 ? -rval|SIGNB : rval;
	if(ra & OVERB) {
		ra &= ~OVERB;
		ot = 1;
	}
}

void mixfmul(int){}

void
mixmul(int m, int f)
{
	vlong rval;
	int signb;

	rval = mval(ra, 0, MASK5);
	rval *= V(cells[m], f);

	if(rval < 0) {
		rval = -rval;
		signb = SIGNB;
	} else
		signb = 0;

	ra = rval>>5*BITS & MASK5 & signb;
	rx = rval & MASK5 & signb;
}

void mixfdiv(int){}

void mixdiv(int m, int f)
{
	vlong rax, quot;
	u32int xsignb, asignb;
	int rem, v;

	v = V(cells[m], f);
	if(v == 0) {
		ot = 1;
		return;
	}
	rax = ra & MASK5;
	rax <<= 5 * BITS;
	rax |= rx & MASK5;
	if(ra >> 31)
		rax = -rax;

	quot = rax / v;
	rem = rax % v;

	if(quot < 0) {
		quot = -quot;
		asignb = SIGNB;
	} else
		asignb = 0;

	if(rem < 0) {
		rem = -rem;
		xsignb = SIGNB;
	} else
		xsignb = 0;

	if(quot & ~MASK5)
		ot = 1;

	ra = quot & MASK5 & asignb;
	rx = rem & MASK5 & xsignb;
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

void
mixslra(int m, int left)
{
	u32int val;

	if(m < 0)
		error("Bad shift");
	if(m > 4) {
		ra &= ~MASK5;
		return;
	}
	val = ra & MASK5;
	ra &= ~MASK5;
	if(left)
		val <<= m * BITS;
	else
		val >>= m * BITS;
	ra |= val & MASK5;
}

void
mixslrax(int m, int left)
{
	u64int rax;

	if(m < 0)
		error("Bad shift");
	if(m > 9) {
		ra &= ~MASK5;
		rx &= ~MASK5;
		return;
	}
	rax = ra & MASK5;
	ra &= ~MASK5;
	rax <<= 5 * BITS;
	rax |= rx & MASK5;
	rx &= ~MASK5;
	if(left)
		rax <<= m;
	else
		rax >>= m;
	rx |= rax & MASK5;
	ra |= rax>>5*BITS & MASK5;
}

void
mixslc(int m)
{
	u64int rax, s;

	if(m < 0)
		error("Bad shift");

	m %= 10;

	rax = ra & MASK5;
	ra &= ~MASK5;
	rax <<= 5 * BITS;
	rax |= rx & MASK5;
	rx &= ~MASK5;

	s = rax & mask[m]<<10-m;
	rax <<= m;
	rax &= ~mask[m];
	rax |= s;

	rx |= rax & MASK5;
	ra |= rax>>5*BITS & MASK5;
}

void
mixsrc(int m)
{
	u64int rax, s;

	if(m < 0)
		error("Bad shift");

	m %= 10;

	rax = ra & MASK5;
	ra &= ~MASK5;
	rax <<= 5 * BITS;
	rax |= rx & MASK5;
	rx &= ~MASK5;

	s = rax & mask[m];
	rax >>= m;
	rax &= ~mask[m] << 10-m;
	rax |= s<<10-m;

	rx |= rax & MASK5;
	ra |= rax>>5*BITS & MASK5;
}

void
mixmove(int s, int f)
{
	int d;

	if(f == 0)
		return;

	d = mval(ri[1], 0, MASK2);
	if(d < 0 || d > 4000)
		error("Bad address");
	memcpy(cells+d, cells+s, f*sizeof(u32int));
	d += f;
	d &= MASK2;
	ri[1] = d < 0 ? -d|SIGNB : d;
}

void
mixld(int m, int f, u32int *reg)
{
	int v;

//	print("mixld: m is %d\n", m);
	v = V(cells[m], f);
//	print("loading %d with %d\n", mval(*reg, 0, MASK5), v);
	*reg = v < 0 ? -v|SIGNB : v;
//	print("now is %d\n", *reg);
}

void
mixldn(int m, int f, u32int *reg)
{
	int v;

	v = -V(cells[m], f);
	*reg = v < 0 ? -v|SIGNB : v;
}

u32int
fset(u32int w, u32int v, int f, int sign)
{
	int a, b, d;

	if(f == 5)
		return v;

	UNF(a, b, f);
	if(a == 0) {
		w = sign ? w|SIGNB : w&~SIGNB;
		if(b == 0)
			return w;
		a++;
	}

	d = b - a;
	if(a > 5 || b > 5 || d < 0 || d > 4)
		error("Bad fpart");
	v &= mask[d];
	v <<= (5-b) * BITS;
	w &= ~(mask[d] << (5-b)*BITS);
	return w | v;
}

void
mixst(int m, int f, u32int reg, u32int msk)
{
	cells[m] = fset(cells[m], reg&msk, f, reg>>31);
}

int
mixjbus(int /*m*/, int /*f*/, int ip)
{
	return ip+1;
}

void
mixioc(int, int f)
{
	switch(f) {
	case 18:
	case 19:
		print("\n");
		break;
	}
}

void mixin(int, int){}

void
mixprint(int m, int words)
{
	int i;
	u32int *wp, w;
	Rune buf[6], *rp;

	wp = cells+m;
	while(words-- > 0) {
		rp = buf;
		w = *wp++;
		for(i = 4; i > -1; i--)
			*rp++ = mixtorune(w>>i*BITS & MASK1);
		*rp = '\0';
		print("%S\n", buf);
	}
}

void
mixout(int m, int f)
{
	switch(f) {
	case 18:
		mixprint(m, 24);
		break;
	case 19:
		mixprint(m, 14);
		break;
	}
}

int
mixjred(int m, int /*f*/, int /*ip*/)
{
	return m;
}

int
mixjmp(int m, int ip)
{
//	print("mixjmp: m %d, ip %d\n", m, ip);
	ri[0] = ip+1 & MASK2;
	return m;
}

int
mixjov(int m, int ip)
{
	if(ot) {
		ot = 0;
		ri[0] = ip+1 & MASK2;
		return m;
	}
	return ip + 1;
}

int
mixjnov(int m, int ip)
{
	if(ot) {
		ot = 0;
		return ip + 1;
	}
	ri[0] = ip+1 & MASK2;
	return m;
}

int
mixjc(int m, int ip, int c1, int c2)
{
	if(c1 || c2) {
		ri[0] = ip+1 & MASK2;
		return m;
	}
	return ip + 1;
}

int
mixjaxic(int m, int ip, u32int r, u32int msk, int f)
{
	int v, c;

	v = mval(r, 0, msk);
	switch(f) {
	default:	error("Bad instruction");
	case 0:	c = v < 0;	break;
	case 1:	c = v == 0;	break;
	case 2:	c = v > 0;	break;
	case 3:	c = v >= 0;	break;
	case 4:	c = v != 0;	break;
	case 5:	c = v <= 0;	break;
	}

	if(c) {
		ri[0] = ip+1 & MASK2;
		return m;
	}
	return ip + 1;
}

void
mixinc(int m, u32int *r)
{
	int v;

	v = mval(*r, 0, MASK5);
	v += m;
	*r = v < 0 ? -v|SIGNB : v;
}

void mixfcmp(void){}

void
mixcmp(int m, int f, u32int r)
{
	int v1, v2;

	ce = cg = cl = 0;

	v1 = V(r, f);
	v2 = V(cells[m], f);
	if(v1 < v2)
		cl = 1;
	else if(v1 > v2)
		cg = 1;
	else
		ce = 1;
}

void
mixvm(int ip)
{
	int a, i, f, c, m, inst;

Top:
	for (;;) {
//		print("dovm: ip is %d\n", ip);
		if(ip < 0 || ip > 4000)
			error("Bad memory access %d\n", ip);
		inst = cells[ip];
//		print("dovm: inst is %ud\n", inst);
		a = V(inst, F(0, 2));
		i = V(inst, F(3, 3));
		f = V(inst, F(4, 4));
		c = V(inst, F(5, 5));
		m = M(a, i);
//		print("dovm: a is %d; i is %d; m is %d\n", a, i, m);
		switch(c) {
		default:
			fprint(2, "Bad op!\n");
			exits("error");
		case 0:
			break;
		case 1:
			if(f == 6)
				mixfadd(inst);
			else
				mixadd(m, f);
			break;
		case 2:
			if(f == 6)
				mixfsub(inst);
			else
				mixsub(m, f);
			break;
		case 3:
			if(f == 6)
				mixfmul(inst);
			else
				mixmul(m, f);
			break;
		case 4:
			if(f == 6)
				mixfdiv(inst);
			else
				mixdiv(m, f);
			break;
		case 5:
			switch(f) {
			default:
				error("Bad instruction");
			case 0:
				mixnum();
				break;
			case 1:
				mixchar();
				break;
			case 2:
				return;	/* HLT */
			}
			break;
		case 6:
			switch(f) {
			default: error("Bad instruction");
			case 0: mixslra(m, 1);	break;
			case 1: mixslra(m, 0);	break;
			case 2: mixslrax(m, 1);	break;
			case 4: mixslrax(m, 0);	break;
			case 5: mixslc(m);	break;
			case 6: mixsrc(m);	break;
			}
			break;
		case 7:
			mixmove(m, f);
			break;
		case 8:
			mixld(m, f, &ra);
			break;
		case 9: case 10: case 11:
		case 12: case 13: case 14:
			mixld(m, f, ri + (c-8));
			break;
		case 15:
			mixld(m, f, &rx);
			break;
		case 16:
			mixldn(m, f, &ra);
			break;
		case 17: case 18: case 19:
		case 20: case 21: case 22:
			mixldn(m, f, ri + (c-16));
			break;
		case 23:
			mixldn(m, f, &rx);
			break;
		case 24:
			mixst(m, f, ra, MASK5);
			break;
		case 25: case 26: case 27:
		case 28: case 29: case 30:
			mixst(m, f, ri[c-24], MASK2);
			break;
		case 31:
			mixst(m, f, rx, MASK5);
			break;
		case 32:
			mixst(m, f, ri[0], MASK2);
			break;
		case 33:
			cells[m] = 0; /* STZ */
			break;
		case 34:
			ip = mixjbus(m, f, ip);
			goto Top;
		case 35:
			mixioc(m, f);
			break;
		case 36:
			mixin(m, f);
			break;
		case 37:
			mixout(m, f);
			break;
		case 38:
			ip = mixjred(m, f, ip);
			break;
		case 39:
			switch(f) {
			default: error("Bad instruction");
			case 0: ip = mixjmp(m, ip);	break;
			case 1: ip = m;	break;
			case 2: ip = mixjov(m, ip);	break;
			case 3: ip = mixjnov(m, ip);	break;
			case 4: ip = mixjc(m, ip, cl, 0);	break;
			case 5: ip = mixjc(m, ip, ce, 0);	break;
			case 6: ip = mixjc(m, ip, cg, 0);	break;
			case 7: ip = mixjc(m, ip, cg, ce);	break;
			case 8: ip = mixjc(m, ip, cl, cg);	break;
			case 9: ip = mixjc(m, ip, cl, ce);	break;
			}
			goto Top;
		case 40:
			ip = mixjaxic(m, ip, ra, MASK5, f);
			goto Top;
		case 41: case 42: case 43:
		case 44: case 45: case 46:
			ip = mixjaxic(m, ip, ri[c-40], MASK2, f);
			goto Top;
		case 47:
			ip = mixjaxic(m, ip, rx, MASK5, f);
			goto Top;
		case 48:
			switch(f) {
			case 0:	mixinc(m, &ra);	break;
			case 1: mixinc(-m, &ra);	break;
			case 2:	ra = m < 0 ? -m|SIGNB : m;	break;
			case 3:	ra = m > 0 ? m|SIGNB : -m;	break;
			}
			break;
		case 49: case 50: case 51:
		case 52: case 53: case 54:
			switch(f) {
			case 0:	mixinc(m, ri+(c-48));	break;
			case 1:	mixinc(-m, ri+(c-48));	break;
			case 2:	ri[c-48] = m < 0 ? -m|SIGNB : m;	break;
			case 3:	ri[c-48] = m > 0 ? m|SIGNB : -m;	break;
			}
			break;
		case 55:
			switch(f) {
			case 0:	mixinc(m, &rx);	break;
			case 1: mixinc(-m, &rx);	break;
			case 2:	rx = m < 0 ? -m|SIGNB : m;	break;
			case 3:	rx = m > 0 ? m|SIGNB : -m;	break;
			}
			break;
		case 56:
			if(f == 6)
				mixfcmp();
			else
				mixcmp(m, f, ra);
			break;
		case 57: case 58: case 59:
		case 60: case 61: case 62:
			mixcmp(m, f, ri[c-56] & ~(MASK3<<2*BITS));
			break;
		case 63:
			mixcmp(m, f, rx);
			break;
		}
		ip++;
	}
}
