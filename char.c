#include <u.h>
#include <libc.h>
#include <avl.h>
#include "mix.h"

typedef
struct Mixchar {
	Avl;
	Rune r;
	int m;
} Mixchar;

static Avltree *rtomix;
static Rune mixtor[] = {
	' ',
	'A',
	'B',
	'C',
	'D',
	'E',
	'F',
	'G',
	'H',
	'I',
	L'Δ',
	'J',
	'K',
	'L',
	'M',
	'N',
	'O',
	'P',
	'Q',
	L'Σ',
	L'Π',
	'S',
	'T',
	'U',
	'V',
	'W',
	'X',
	'Y',
	'Z',
	'0',
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8',
	'9',
	'.',
	',',
	'(',
	')',
	'+',
	'-',
	'*',
	'/',
	'=',
	'$',
	'<',
	'>',
	'@',
	';',
	':',
	'\''
};

static Mixchar pool[nelem(mixtor)];

static int
runecmp(Avl *a, Avl *b)
{
	Rune ra, rb;

	ra = ((Mixchar*)a)->r;
	rb = ((Mixchar*)b)->r;

	if(ra < rb)
		return -1;
	if(ra > rb)
		return 1;
	return 0;
}

void
cinit(void)
{
	int i;
	Mixchar *a;

	rtomix = avlcreate(runecmp);
	for(i = 0; i < nelem(mixtor); i++) {
		a = pool+i;
		a->r = mixtor[i];
		a->m = i;
		avlinsert(rtomix, a);
	}
}

int
runetomix(Rune r)
{
	Mixchar *c, l;

	l.r = r;
	c = (Mixchar*)avllookup(rtomix, &l);
	if(c == nil)
		return -1;

	return c->m;
}

Rune
mixtorune(int m)
{
	if(m < nelem(mixtor))
		return mixtor[m];
	return -1;
}
