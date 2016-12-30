%{
#include <u.h>
#include <libc.h>
#include <avl.h>
#include "mix.h"
%}

%union {
	Sym *sym;
	int ival;
	Rune r;
}

%type	<ival>	exp aexp ipart fpart wval apart
%type	<sym>	loc reflit

%token	<sym>	LSYMDEF LSYMREF LOP LEQU LORIG LCON LALF LEND
%token	<ival>	LNUM
%token	<r>	LCHAR

%left '+' '-' '*' '/' LSS ':' ','

%%

prog:
|	prog inst

inst:
	loc LOP '\n'
	{
		defloc($loc, star);
		asm($LOP, 0, 0, -1);
	}
|	loc LOP ws apart ipart fpart
	{
		defloc($loc, star);
		asm($LOP, $apart, $ipart, $fpart);
		skipto('\n');
	}
	'\n'
|	loc LOP ws reflit ipart fpart
	{
		defloc($loc, star);
		refasm($LOP, $ipart, $fpart);
		addref($reflit, star);
		skipto('\n');
	}
	'\n'
|	loc LEQU ws wval
	{
		defloc($loc, $wval);
		skipto('\n');
	}
	'\n'
|	loc LORIG ws wval
	{
		defloc($loc, star);
		star = $wval;
		skipto('\n');
	}
	'\n'
|	loc LCON ws wval
	{
		out($wval);
		skipto('\n');
	}
	'\n'
|	loc LEND ws wval
	{
		endprog($wval);
		defloc($loc, star);
		skipto('\n');
	}
	'\n'

loc:
	ws
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
		$$ = wval(0, $exp, $fpart);
	}
|	wval ',' exp fpart
	{
		$$ = wval($wval, $exp, $fpart);
	}

ws:
	' '
|	ws ' '

%%

void
defrefs(Sym*, int)
{
}

void
defloc(Sym *sym, int val)
{
	if(sym != nil) {
		defrefs(sym, val);
		sym->lex = LSYMDEF;
		free(sym->refs);
		sym->val = val;
	}
}

void
addref(Sym *ref, int star)
{
	if(ref->refs == nil || ref->i == ref->max) {
		ref->max = ref->max == 0 ? 3 : ref->max*2;
		ref->refs = erealloc(ref->refs, ref->max);
	}
	ref->refs[ref->i++] = star;
}

void
asm(Sym *op, int apart, int ipart, int fpart)
{
	u32int inst;

	inst = op->opc & MASK1;
	inst |= (ipart&MASK1) << 6;
	if(fpart == -1)
		inst |= (op->f&MASK1) << 12;
	else
		inst |= (fpart&MASK1) << 12;
	inst |= (apart&MASK2) << 18;
	if(apart < 0)
		inst |= SIGNB
	mem[star] = inst;
}

void
refasm(Sym*, int, int)
{
}

void
out(int)
{
}

Sym*
con(int)
{
	return nil;
}

void
endprog(int)
{
}

int
wval(int, int, int)
{
	return 0;
}
