#include <u.h>
#include <libc.h>
#include <ctype.h>
#include <bio.h>
#include <avl.h>
#include "mix.h"

int
getf(char *line)
{
	long a, b;

//	print("getf: %s\n", line);
	if(*line == '\0')
		return 5;
	if(*line != '(') 
		return -1;
	a = strtol(line+1, &line, 10);
	if(*line != ':')
		return -1;
	b = strtol(line+1, &line, 10);
	if(*line != ')')
		return -1;
	return F(a, b);
}	

void
disp(char *line)
{
	int f;
	long m;

	if(setjmp(errjmp) == 1)
		goto Err;
	m = strtol(line, &line, 10);
	if((f = getf(line)) == -1)
		goto Err;
	print("%d\n", V(cells[m], f));
	return;

Err:
	print("?\n");
}

void
dispreg(char *line)
{
	vlong rax;
	char c;
	int i, f;
	u32int reg;

	if(setjmp(errjmp) == 1)
		goto Err;

	switch(c = *line++) {
	case 'a':
		if(*line == 'x') {
			rax = ra & MASK5;
			rax <<= 5 * BITS;
			rax |= rx & MASK5;
			if(ra >> 31)
				rax = -rax;
			print("%lld\n", rax);
			return;
		} else
			reg = ra;
		break;
	case 'x':
		reg = rx;
		break;
	case 'j':
		reg = ri[0];
		break;
	default:
		if(!isdigit(c))
			goto Err;
		i = c - '0';
		if(i < 1 || i > 6)
			goto Err;
		reg = ri[i];
	}

	if((f = getf(line)) == -1)
		goto Err;

	print("%d\n", V(reg, f));
	return;

Err:
	print("?\n");
}

void
breakp(char *line)
{
	long l;

	if(!isdigit(*line)) {
		goto Err;
	}
	l = strtol(line, nil, 10);
	if(l < 0 || l > 4000)
		goto Err;
	bp[l] ^= 1;
	return;

Err:
	print("?\n");
	return;
}

void
asm(char *l)
{
	l = skip(l, ' ');
	if(*l++ == '<') {
		Bterm(&bin);
		print("asm: %s\n", l);
		if(asmfile(skip(l, ' ')) == -1)
			goto Err;
		Binit(&bin, 0, OREAD);
		return;
	}

	line = 1;
	filename = "<stdin>";
	if(setjmp(errjmp) == 0)
		yyparse();
	Bterm(&bin);
	Binit(&bin, 0, OREAD);
	return;

Err:
	print("?\n");
}

void
disasm(char *line)
{
	long l;

	if(!isdigit(*line)) {
		print("?\n");
		return;
	}

	l = strtol(line, nil, 10);
	prinst(l);
}

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
		print("%S", buf);
	}
	print("\n");
}

void
out(char *line)
{
	long l;

	if(!isdigit(*line)) {
		print("?\n");
		return;
	}

	l = strtol(line, nil, 10);
	mixprint(l, 1);
}

void
repl(void)
{
	char *line, c;
	int len, once;

	Binit(&bin, 0, OREAD);

	for(;;) {
		print("MIX ");

		if((line = Brdline(&bin, '\n')) == nil)
			return;

		if((len = Blinelen(&bin)) == 1)
			continue;

		line[len-1] = '\0';

		once = 0;
		switch(c = line[0]) {
		default:
			if(isdigit(c))
				disp(line);
			break;
		case 'r':
			dispreg(line+1);
			break;
		case 'b':
			breakp(line+1);
			break;
		case 'a':
			asm(line+1);
			break;
		case 'd':
			disasm(line+1);
			break;
		case 'o':
			out(line+1);
			break;
		case 's':
			once = 1;
		case 'g':
			if(vmstart == -1) {
				print("?\n");
				break;
			}
			vmstart = mixvm(vmstart, once);
			if(vmstart == -1)
				print("halted\n");
			else {
				print("at %d:\t", vmstart);
				prinst(vmstart);
			}
			break;
		case 'x':
			return;
		}
	}
}
