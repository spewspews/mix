%{
#include <u.h>
#include <libc.h>
#include <avl.h>
#include "mix.h"
%}

%union {
	Sym *sym;
	long lval;
/*	Rune r; */
}

%type	<lval>	exp aexp ipart fpart wval apart
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
|	loc LCON ws wval eol
	{
		mem[star++] = $wval;
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
|	'=' wval '='
	{
		$$ = con($wval);
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
		$$ = ($LSYMDEF)->val;
	}
|	'*'
	{
		$$ = star;
	}

wval:
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
defrefs(Sym *sym, int apart)
{
	u32int inst;
	int *ref, *ep;

//	print("defref: %p %d\n", sym->refs, sym->i);
	ep = sym->refs + sym->i;
	for(ref = sym->refs; ref < ep; ref++) {
		inst = mem[*ref];
//		print("defref on %d\n", *ref);
		inst &= ~MASK2;
		inst |= apart&MASK2;
		if(apart < 0)
			inst |= SIGNB;
		mem[*ref] = inst;
	}
}

void
defloc(Sym *sym, int val)
{
	if(sym == nil)
		return;
//	print("defloc %s %d\n", sym->name, val);
	defrefs(sym, val);
//	print("defloc freeing %p\n", sym->refs);
	free(sym->refs);
	sym->lex = LSYMDEF;
	sym->val = val;
}

void
addref(Sym *ref, int star)
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
asm(Sym *op, int apart, int ipart, int fpart)
{
	u32int inst;

	print("asm %s %d %d %d\n", op->name, apart, ipart, fpart);
	inst = apart & MASK2;

	inst |= (ipart&MASK1) << BITS*2;

	if(fpart == -1)
		inst |= (op->f&MASK1) << BITS*3;
	else
		inst |= (fpart&MASK1) << BITS*3;

	inst |= (op->opc&MASK1) << BITS*4;

	if(apart < 0)
		inst |= SIGNB;

	mem[star++] = inst;
}

void
refasm(Sym *op, int ipart, int fpart)
{
	u32int inst;

//	print("refasm %s %d %d\n", op->name, ipart, fpart);
	inst = (ipart&MASK1) << BITS*2;

	if(fpart == -1)
		inst |= (op->f&MASK1) << BITS*3;
	else
		inst |= (fpart&MASK1) << BITS*3;

	inst |= (op->opc&MASK1) << BITS*4;

	mem[star++] = inst;
}

Sym*
con(int exp)
{
	Con *c;
	static int i;
	static char buf[20];

	seprint(buf, buf+20, "_con_%d\n", i++);
	c = emalloc(sizeof(*c));
	c->sym = sym(buf);
	c->exp = exp;
	c->link = cons;
	return c->sym;
}

void
endprog(int start)
{
	Con *c;
	for(c = cons; c != nil; c = c->link) {
		defloc(c->sym, star);
		mem[star++] = c->exp;
	}
	vmstart = start;
	yydone = 1;
}

int
wval(int old, int exp, int fpart)
{
	int a, b, val, m;

	if(fpart == -1) {
		a = 0;
		b = 5;
	} else {
		UNF(a, b, fpart);
	}
	if(a == 0) {
		if(exp < 0)
			old |= SIGNB;
		else
			old &= ~SIGNB;
		if(b == 0)
			return old;
		a = 1;
	}
	m = b-a;
	if(a > b || a > 5 || b > 5 || m > 4)
		error("Invalid fpart");
	val = exp & mask[m];
	val <<= (5-b) * BITS;
	old &= ~(mask[m] << (5-b)*BITS);
	old |= val;
	return old;
}
