%{
#include <u.h>
#include <libc.h>
#include <avl.h>
#include "mix.h"
%}

%union {
	Sym *sym;
	long lval;
	u32int mval;
/*	Rune r; */
}

%type	<lval>	wval apart exp aexp fpart ipart
%type	<mval>	wval1
%type	<sym>	loc reflit

%token	<sym>	LSYMDEF LSYMREF LOP LEQU LORIG LCON LALF LEND
%token	<lval>	LNUM
/* %token	<r>	LCHAR */

%left '+' '-' '*' '/' LSS ':' ','

%%

prog:
	prog1 end

prog1:
|	prog1 inst

inst:
	loc LOP eol
	{
		defloc($loc, star);
		asm($LOP, 0, 0, -1);
	}
|	loc LOP ws apart ipart fpart eol
	{
		defloc($loc, star);
		asm($LOP, $apart, $ipart, $fpart);
	}
|	loc LOP ws reflit ipart fpart eol
	{
		defloc($loc, star);
		addref($reflit, star);
		refasm($LOP, $ipart, $fpart);
	}
|	loc LEQU ws wval eol
	{
		defloc($loc, $wval);
	}
|	loc LORIG ws wval eol
	{
		defloc($loc, star);
		star = $wval;
	}
|	loc LCON ws wval1 eol
	{
		defloc($loc, star);
		cells[star++] = $wval1;
	}

end:
	loc LEND ws wval eol
	{
		endprog($wval);
		defloc($loc, star);
	}

loc:
	{
		$$ = nil;
	}
|	ws
	{
		$$ = nil;
	}
|	LSYMREF ws
	{
		$$ = $LSYMREF;
	}

apart:
	{
		$$ = 0;
	}
|	exp

reflit:
	LSYMREF
|	'=' wval1 '='
	{
		$$ = con($wval1);
	}

ipart:
	{
		$$ = 0;
	}
|	',' exp
	{
		$$ = $exp;
	}

fpart:
	{
		$$ = -1;
	}
|	'(' exp ')'
	{
		if($exp < 0)
			error("invalid fpart %d\n", $exp);
		$$ = $exp;
	}

exp:
	aexp
|	'+' aexp
	{
		$$ = $aexp;
	}
|	'-' aexp
	{
		$$ = -$aexp;
//		print("aexp unary neg %ld\n", $$);
	}
|	exp '+' aexp
	{
		$$ = $exp + $aexp;
	}
|	exp '-' aexp
	{
		$$ = $exp - $aexp;
	}
|	exp '*' aexp
	{
		$$ = $exp * $aexp;
	}
|	exp '/' aexp
	{
		$$ = ($exp) / $aexp;
	}
|	exp LSS aexp
	{
		$$ = (((vlong)$exp) << 30) / $aexp;
	}
|	exp ':' aexp
	{
		$$ = F($exp, $aexp);
	}

aexp:
	LNUM
|	LSYMDEF
	{
		u32int mval;

		mval = ($LSYMDEF)->mval;
		if(mval & SIGNB) {
			mval &= ~SIGNB;
			$$ = -((long)mval);
		} else
			$$ = mval;
//		print("aexp sym: %s, %ld\n", ($LSYMDEF)->name, $$);
	}
|	'*'
	{
		$$ = star;
	}

wval:
	wval1
	{
		if($wval1 & SIGNB)
			$$ = -(long)($wval1 & MASK5);
		else
			$$ = $wval1;
	}

wval1:
	exp fpart
	{
//		print("got wval %ld\n", $exp);
		$$ = wval(0, $exp, $fpart);
	}
|	wval ',' exp fpart
	{
		$$ = wval($wval, $exp, $fpart);
	}

eol:
	'\n'

ws:
	' '
|	ws ' '

%%

void
defrefs(Sym *sym, long apart)
{
	u32int inst, mval;
	int *ref, *ep;

//	print("defref: %p %d\n", sym->refs, sym->i);
	ep = sym->refs + sym->i;
	for(ref = sym->refs; ref < ep; ref++) {
		inst = cells[*ref];
//		print("defref on %d\n", *ref);
		inst &= ~(MASK2 << BITS*3);
		if(apart < 0) {
			mval = -apart;
			inst |= SIGNB;
		} else
			mval = apart;
		inst |= (mval&MASK2) << BITS*3;
		cells[*ref] = inst;
	}
}

void
defloc(Sym *sym, long val)
{
	if(sym == nil)
		return;
//	print("defloc %s %d\n", sym->name, val);
	defrefs(sym, val);
//	print("defloc freeing %p\n", sym->refs);
	free(sym->refs);
	sym->lex = LSYMDEF;
	sym->mval = val < 0 ? -val|SIGNB : val;
}

void
addref(Sym *ref, long star)
{
//	print("addref %p %d %d\n", ref->refs, ref->i, ref->max);
	if(ref->refs == nil || ref->i == ref->max) {
		ref->max = ref->max == 0 ? 3 : ref->max*2;
		ref->refs = erealloc(ref->refs, ref->max * sizeof(int));
	}
	ref->refs[ref->i++] = star;
//	print("addedref %p %d %d\n", ref->refs, ref->i, ref->max);
}

void
asm(Sym *op, long apart, long ipart, long fpart)
{
	u32int inst, mval;

//	print("asm %s %d %d %d\n", op->name, apart, ipart, fpart);
	inst = op->opc & MASK1;

	if(fpart == -1)
		inst |= (op->f&MASK1) << BITS;
	else
		inst |= (fpart&MASK1) << BITS;

	inst |= (ipart&MASK1) << BITS*2;

	if(apart < 0) {
		mval = -apart;
		inst |= SIGNB;
	} else
		mval = apart;
	inst |= (mval&MASK2) << BITS*3;

	cells[star++] = inst;
}

void
refasm(Sym *op, long ipart, long fpart)
{
	u32int inst;

//	print("refasm %s %d %d\n", op->name, ipart, fpart);
	inst = op->opc & MASK1;

	if(fpart == -1)
		inst |= (op->f&MASK1) << BITS;
	else
		inst |= (fpart&MASK1) << BITS;

	inst |= (ipart&MASK1) << BITS*2;

	cells[star++] = inst;
}

Sym*
con(u32int exp)
{
	Con *c;
	static int i;
	static char buf[20];

	seprint(buf, buf+20, "con%d\n", i++);
	c = emalloc(sizeof(*c));
	c->sym = sym(buf);
	c->exp = exp;
	c->link = cons;
	cons = c;
	return c->sym;
}

void
endprog(int start)
{
	Con *c;
	for(c = cons; c != nil; c = c->link) {
		defloc(c->sym, star);
		cells[star++] = c->exp;
	}
	vmstart = start;
	yydone = 1;
}

u32int
wval(u32int old, int exp, int fpart)
{
	int a, b, val, m, sign;

	if(fpart == -1) {
		if(exp < 0)
			return -exp | SIGNB;
		else
			return exp;
	}

	UNF(a, b, fpart);

	if(a > 5 || b > 5)
		error("Invalid fpart");

	if(exp < 0) {
		sign = 1;
		exp = -exp;
	} else
		sign = 0;

	if(a == 0) {
		old = sign ? old|SIGNB : old&~SIGNB;
		if(b == 0)
			return old;
		a = 1;
	}

	m = b - a;
	if(m < 0 || m > 4)
		error("Invalid fpart");
	val = exp & mask[m];
	val <<= (5-b) * BITS;
	old &= ~(mask[m] << (5-b)*BITS);
	return old | val;
}
