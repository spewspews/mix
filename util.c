#include <u.h>
#include <libc.h>
#include <avl.h>
#include "mix.h"

static char buf[1024];

char*
skip(char *s, int c) {
	while(*s == c)
		s++;
	return s;
}

void
error(char *s, ...)
{
	char *bp;
	va_list a;

	bp = seprint(buf, buf+1024, "Error: %s:%d: ", filename, line);
	va_start(a, s);
	bp = vseprint(bp, buf+1024, s, a);
	va_end(a);
	*bp++ = '\n';
	write(2, buf, bp - buf);
	longjmp(errjmp, 1);
}

void*
emalloc(ulong s)
{
	void *v;

	v = malloc(s);
	if(v == nil)
		error("Error allocating %lud: %r\n", s);
	setmalloctag(v, getcallerpc(&s));
	return v;
}

void*
emallocz(ulong s)
{
	void *v;

	v = malloc(s);
	if(v == nil)
		error("Error allocating %lud: %r", s);
	memset(v, 0, s);
	return v;
}

void*
ecalloc(ulong n, ulong s)
{
	void *v;

	v = calloc(n, s);
	if(v == nil)
		error("Error allocating %lud: %r", s);
	setmalloctag(v, getcallerpc(&s));
	return v;
}

void*
erealloc(void *p, ulong s)
{
	void *v;

	v = realloc(p, s);
	if(v == nil)
		error("Error re-allocating %lud: %r", s);
	setrealloctag(v, getcallerpc(&s));
	return v;
}

Rune*
erunestrdup(Rune *s)
{
	Rune *n;

	n = runestrdup(s);
	if(n == nil)
		error("Error duplicating string %s: %r", s);
	setmalloctag(n, getcallerpc(&s));
	return n;
}

void
efmtprint(Fmt *f, char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	if(fmtvprint(f, fmt, va) < 0)
		error("Printing error.");
	va_end(va);
}
