#include	"l.h"

long	OFFSET;
/*
long	BADOFFSET	=	-1;

		if(OFFSET <= BADOFFSET && OFFSET+4 > BADOFFSET)\
			abort();\
		OFFSET += 4;\

		if(OFFSET == BADOFFSET)\
			abort();\
		OFFSET++;\
*/

#define	CPUT(c)\
	{\
		cbp[0] = (c);\
		cbp++;\
		cbc--;\
		if(cbc <= 0)\
			cflush();\
	}

long
entryvalue(void)
{
	char *a;
	Sym *s;

	a = INITENTRY;
	if(*a >= '0' && *a <= '9')
		return atolwhex(a);
	s = lookup(a, 0);
	if(s->type == 0)
		return INITTEXT;
	if(s->type != STEXT && s->type != SLEAF)
		diag("entry not text: %s", s->name);
	return s->value;
}

void
asmb(void)
{
	Prog *p;
	long t;
	Optab *o;

	if(debug['v'])
		Bprint(&bso, "%5.2f asm\n", cputime());
	Bflush(&bso);
	OFFSET = HEADR;
	seek(cout, OFFSET, 0);
	pc = INITTEXT;
	for(p = firstp; p != P; p = p->link) {
		if(p->as == ATEXT) {
			curtext = p;
			autosize = p->to.u0.aoffset + 4;
		}
		if(p->pc != pc) {
			diag("phase error %lux sb %lux",
				p->pc, pc);
			if(!debug['a'])
				prasm(curp);
			pc = p->pc;
		}
		curp = p;
		o = oplook(p);	/* could probably avoid this call */
		asmout(p, o);
		pc += o->size;
	}
	while(pc-INITTEXT < textsize) {
		CPUT(0);
		pc++;
	}

	if(debug['a'])
		Bprint(&bso, "\n");
	Bflush(&bso);
	cflush();

	curtext = P;
	switch(HEADTYPE) {
	case 0:
	case 4:
		OFFSET = rnd(HEADR+textsize, 4096);
		seek(cout, OFFSET, 0);
		break;
	case 1:
	case 2:
	case 3:
	case 5:
		OFFSET = HEADR+textsize;
		seek(cout, OFFSET, 0);
		break;
	}
	for(t = 0; t < datsize; t += sizeof(buf)-100) {
		if(datsize-t > sizeof(buf)-100)
			datblk(t, sizeof(buf)-100);
		else
			datblk(t, datsize-t);
	}

	symsize = 0;
	lcsize = 0;
	if(!debug['s']) {
		if(debug['v'])
			Bprint(&bso, "%5.2f sym\n", cputime());
		Bflush(&bso);
		switch(HEADTYPE) {
		case 0:
		case 4:
			OFFSET = rnd(HEADR+textsize, 4096)+datsize;
			seek(cout, OFFSET, 0);
			break;
		case 1:
			debug['s'] = 1;
			break;
		case 3:
		case 2:
		case 5:
			OFFSET = HEADR+textsize+datsize;
			seek(cout, OFFSET, 0);
			break;
		}
		if(!debug['s'])
			asmsym();
		if(debug['v'])
			Bprint(&bso, "%5.2f pc\n", cputime());
		Bflush(&bso);
		if(!debug['s'])
			asmlc();
		cflush();
	}

	if(debug['v'])
		Bprint(&bso, "%5.2f header\n", cputime());
	Bflush(&bso);
	OFFSET = 0;
	seek(cout, OFFSET, 0);
	switch(HEADTYPE) {
	case 0:	/* unix simple */
		lput(0x160L<<16);		/* magic and sections */
		lput(0L);			/* time and date */
		lput(rnd(HEADR+textsize, 4096)+datsize);
		lput(symsize);			/* nsyms */
		lput((0x38L<<16)|7L);		/* size of optional hdr and flags */
		lput((0413<<16)|0437L);		/* magic and version */
		lput(rnd(HEADR+textsize, 4096));	/* sizes */
		lput(datsize);
		lput(bsssize);
		lputl(entryvalue());		/* va of entry */
		lput(INITTEXT-HEADR);		/* va of base of text */
		lput(INITDAT);			/* va of base of data */
		lput(INITDAT+datsize);		/* va of base of bss */
		lput(~0L);			/* gp reg mask */
		lput(0L);
		lput(0L);
		lput(0L);
		lput(0L);
		lput(~0L);			/* gp value ?? */
		break;
	case 1:	/* aif for risc os */
		lputl(0xe1a00000);		/* NOP - decompress code */
		lputl(0xe1a00000);		/* NOP - relocation code */
		lputl(0xeb000000 + 12);		/* BL - zero init code */
		lputl(0xeb000000 +
			(entryvalue()
			 - INITTEXT
			 + HEADR
			 - 12
			 - 8) / 4);		/* BL - entry code */

		lputl(0xef000011);		/* SWI - exit code */
		lputl(textsize+HEADR);		/* text size */
		lputl(datsize);			/* data size */
		lputl(0);			/* sym size */

		lputl(bsssize);			/* bss size */
		lputl(0);			/* sym type */
		lputl(INITTEXT-HEADR);		/* text addr */
		lputl(0);			/* workspace - ignored */

		lputl(32);			/* addr mode / data addr flag */
		lputl(0);			/* data addr */
		for(t=0; t<2; t++)
			lputl(0);		/* reserved */

		for(t=0; t<15; t++)
			lputl(0xe1a00000);	/* NOP - zero init code */
		lputl(0xe1a0f00e);		/* B (R14) - zero init return */
		break;
	case 2:	/* plan 9 */
		lput(0x647);			/* magic */
		lput(textsize);			/* sizes */
		lput(datsize);
		lput(bsssize);
		lput(symsize);			/* nsyms */
		lput(entryvalue());		/* va of entry */
		lput(0L);
		lput(lcsize);
		break;
	case 3:	/* boot for 4k */
		lput((0x160L<<16)|3L);		/* magic and sections */
		lput(time(0));			/* time and date */
		lput(HEADR+textsize+datsize);
		lput(symsize);			/* nsyms */
		lput((0x38L<<16)|7L);		/* size of optional hdr and flags */

		lput((0407<<16)|0437L);		/* magic and version */
		lput(textsize);			/* sizes */
		lput(datsize);
		lput(bsssize);
		lput(entryvalue());		/* va of entry */
		lput(INITTEXT);			/* va of base of text */
		lput(INITDAT);			/* va of base of data */
		lput(INITDAT+datsize);		/* va of base of bss */
		lput(~0L);			/* gp reg mask */
		lput(lcsize);
		lput(0L);
		lput(0L);
		lput(0L);
		lput(~0L);			/* gp value ?? */

		strnput(".text", 8);		/* text segment */
		lput(INITTEXT);			/* address */
		lput(INITTEXT);
		lput(textsize);
		lput(HEADR);
		lput(0L);
		lput(HEADR+textsize+datsize+symsize);
		lput(lcsize);			/* line number size */
		lput(0x20L);			/* flags */

		strnput(".data", 8);		/* data segment */
		lput(INITDAT);			/* address */
		lput(INITDAT);
		lput(datsize);
		lput(HEADR+textsize);
		lput(0L);
		lput(0L);
		lput(0L);
		lput(0x40L);			/* flags */

		strnput(".bss", 8);		/* bss segment */
		lput(INITDAT+datsize);		/* address */
		lput(INITDAT+datsize);
		lput(bsssize);
		lput(0L);
		lput(0L);
		lput(0L);
		lput(0L);
		lput(0x80L);			/* flags */
		break;
	case 4:	/* sgi unix coff executable */
		lput((0x160L<<16)|3L);		/* magic and sections */
		lput(time(0));			/* time and date */
		lput(rnd(HEADR+textsize, 4096)+datsize);
		lput(symsize);			/* nsyms */
		lput((0x38L<<16)|7L);		/* size of optional hdr and flags */

		lput((0413<<16)|01012L);	/* magic and version */
		lput(textsize);			/* sizes */
		lput(datsize);
		lput(bsssize);
		lput(entryvalue());		/* va of entry */
		lput(INITTEXT);			/* va of base of text */
		lput(INITDAT);			/* va of base of data */
		lput(INITDAT+datsize);		/* va of base of bss */
		lput(~0L);			/* gp reg mask */
		lput(lcsize);
		lput(0L);
		lput(0L);
		lput(0L);
		lput(~0L);			/* gp value ?? */

		strnput(".text", 8);		/* text segment */
		lput(INITTEXT);			/* address */
		lput(INITTEXT);
		lput(textsize);
		lput(HEADR);
		lput(0L);
		lput(HEADR+textsize+datsize+symsize);
		lput(lcsize);			/* line number size */
		lput(0x20L);			/* flags */

		strnput(".data", 8);		/* data segment */
		lput(INITDAT);			/* address */
		lput(INITDAT);
		lput(datsize);
		lput(rnd(HEADR+textsize, 4096));	/* sizes */
		lput(0L);
		lput(0L);
		lput(0L);
		lput(0x40L);			/* flags */

		strnput(".bss", 8);		/* bss segment */
		lput(INITDAT+datsize);		/* address */
		lput(INITDAT+datsize);
		lput(bsssize);
		lput(0L);
		lput(0L);
		lput(0L);
		lput(0L);
		lput(0x80L);			/* flags */
		break;
	case 5:	/* sgi unix elf executable */
		strnput("\177ELF", 4);		/* e_ident */
		CPUT(1);			/* class = 32 bit */
		CPUT(2);			/* data = MSB */
		CPUT(1);			/* version = CURRENT */
		strnput("", 9);
		lput((2L<<16)|8L);		/* type = EXEC; machine = MIPS */
		lput(1L);			/* version = CURRENT */
		lput(entryvalue());		/* entry vaddr */
		lput(52L);			/* offset to first phdr */
		lput(0L);			/* offset to first shdr */
		lput(0L);			/* flags = MIPS */
		lput((52L<<16)|32L);		/* Ehdr & Phdr sizes*/
		lput((3L<<16)|0L);		/* # Phdrs & Shdr size */
		lput((0L<<16)|0L);		/* # Shdrs & shdr string size */

		lput(1L);			/* text - type = PT_LOAD */
		lput(0L);			/* file offset */
		lput(INITTEXT-HEADR);		/* vaddr */
		lput(INITTEXT-HEADR);		/* paddr */
		lput(HEADR+textsize);		/* file size */
		lput(HEADR+textsize);		/* memory size */
		lput(0x05L);			/* protections = RWX */
		lput(0x10000L);			/* alignment code?? */

		lput(1L);			/* data - type = PT_LOAD */
		lput(HEADR+textsize);		/* file offset */
		lput(INITDAT);			/* vaddr */
		lput(INITDAT);			/* paddr */
		lput(datsize);			/* file size */
		lput(datsize+bsssize);		/* memory size */
		lput(0x06L);			/* protections = RW */
		lput(0x10000L);			/* alignment code?? */

		lput(0L);			/* data - type = PT_NULL */
		lput(HEADR+textsize+datsize);	/* file offset */
		lput(0L);
		lput(0L);
		lput(symsize);			/* symbol table size */
		lput(lcsize);			/* line number size */
		lput(0x04L);			/* protections = R */
		lput(0x04L);			/* alignment code?? */
	}
	cflush();
}

void
strnput(char *s, int n)
{
	for(; *s; s++){
		CPUT(*s);
		n--;
	}
	for(; n > 0; n--)
		CPUT(0);
}

void
lput(long l)
{

	cbp[0] = l>>24;
	cbp[1] = l>>16;
	cbp[2] = l>>8;
	cbp[3] = l;
	cbp += 4;
	cbc -= 4;
	if(cbc <= 0)
		cflush();
}

void
lputl(long l)
{

	cbp[3] = l>>24;
	cbp[2] = l>>16;
	cbp[1] = l>>8;
	cbp[0] = l;
	cbp += 4;
	cbc -= 4;
	if(cbc <= 0)
		cflush();
}

void
cflush(void)
{
	int n;

	n = sizeof(buf.s0.bcbuf) - cbc;
	if(n)
		write(cout, buf.s0.bcbuf, n);
	cbp = buf.s0.bcbuf;
	cbc = sizeof(buf.s0.bcbuf);
}

void
nopstat(char *f, Count *c)
{
	if(c->outof)
	Bprint(&bso, "%s delay %ld/%ld (%.2f)\n", f,
		c->outof - c->count, c->outof,
		(double)(c->outof - c->count)/c->outof);
}

void
asmsym(void)
{
	Prog *p;
	Auto *a;
	Sym *s;
	int h;

	s = lookup("etext", 0);
	if(s->type == STEXT)
		putsymb(s->name, 'T', s->value, s->version);

	for(h=0; h<NHASH; h++)
		for(s=hash[h]; s!=S; s=s->link)
			switch(s->type) {
			case SCONST:
				putsymb(s->name, 'D', s->value, s->version);
				continue;

			case SDATA:
				putsymb(s->name, 'D', s->value+INITDAT, s->version);
				continue;

			case SBSS:
				putsymb(s->name, 'B', s->value+INITDAT, s->version);
				continue;

			case SFILE:
				putsymb(s->name, 'f', s->value, s->version);
				continue;
			}

	for(p=textp; p!=P; p=p->cond) {
		s = p->from.u1.asym;
		if(s->type != STEXT && s->type != SLEAF)
			continue;

		/* filenames first */
		for(a=p->to.u1.aautom; a; a=a->link)
			if(a->type == D_FILE)
				putsymb(a->sym->name, 'z', a->offset, 0);
			else
			if(a->type == D_FILE1)
				putsymb(a->sym->name, 'Z', a->offset, 0);

		if(s->type == STEXT)
			putsymb(s->name, 'T', s->value, s->version);
		else
			putsymb(s->name, 'L', s->value, s->version);

		/* frame, auto and param after */
		putsymb(".frame", 'm', p->to.u0.aoffset+4, 0);
		for(a=p->to.u1.aautom; a; a=a->link)
			if(a->type == D_AUTO)
				putsymb(a->sym->name, 'a', -a->offset, 0);
			else
			if(a->type == D_PARAM)
				putsymb(a->sym->name, 'p', a->offset, 0);
	}
	if(debug['v'] || debug['n'])
		Bprint(&bso, "symsize = %lud\n", symsize);
	Bflush(&bso);
}

void
putsymb(char *s, int t, long v, int ver)
{
	int i, f;

	if(t == 'f')
		s++;
	lput(v);
	if(ver)
		t += 'a' - 'A';
	CPUT(t+0x80);			/* 0x80 is variable length */

	if(t == 'Z' || t == 'z') {
		CPUT(s[0]);
		for(i=1; s[i] != 0 || s[i+1] != 0; i += 2) {
			CPUT(s[i]);
			CPUT(s[i+1]);
		}
		CPUT(0);
		CPUT(0);
		i++;
	}
	else {
		for(i=0; s[i]; i++)
			CPUT(s[i]);
		CPUT(0);
	}
	symsize += 4 + 1 + i + 1;

	if(debug['n']) {
		if(t == 'z' || t == 'Z') {
			Bprint(&bso, "%c %.8lux ", t, v);
			for(i=1; s[i] != 0 || s[i+1] != 0; i+=2) {
				f = ((s[i]&0xff) << 8) | (s[i+1]&0xff);
				Bprint(&bso, "/%x", f);
			}
			Bprint(&bso, "\n");
			return;
		}
		if(ver)
			Bprint(&bso, "%c %.8lux %s<%d>\n", t, v, s, ver);
		else
			Bprint(&bso, "%c %.8lux %s\n", t, v, s);
	}
}

#define	MINLC	4
void
asmlc(void)
{
	long oldpc, oldlc;
	Prog *p;
	long v, s;

	oldpc = INITTEXT;
	oldlc = 0;
	for(p = firstp; p != P; p = p->link) {
		if(p->line == oldlc || p->as == ATEXT || p->as == ANOP) {
			if(p->as == ATEXT)
				curtext = p;
			if(debug['L'])
				Bprint(&bso, "%6lux %P\n",
					p->pc, p);
			continue;
		}
		if(debug['L'])
			Bprint(&bso, "\t\t%6ld", lcsize);
		v = (p->pc - oldpc) / MINLC;
		while(v) {
			s = 127;
			if(v < 127)
				s = v;
			CPUT(s+128);	/* 129-255 +pc */
			if(debug['L'])
				Bprint(&bso, " pc+%ld*%d(%ld)", s, MINLC, s+128);
			v -= s;
			lcsize++;
		}
		s = p->line - oldlc;
		oldlc = p->line;
		oldpc = p->pc + MINLC;
		if(s > 64 || s < -64) {
			CPUT(0);	/* 0 vv +lc */
			CPUT(s>>24);
			CPUT(s>>16);
			CPUT(s>>8);
			CPUT(s);
			if(debug['L']) {
				if(s > 0)
					Bprint(&bso, " lc+%ld(%d,%ld)\n",
						s, 0, s);
				else
					Bprint(&bso, " lc%ld(%d,%ld)\n",
						s, 0, s);
				Bprint(&bso, "%6lux %P\n",
					p->pc, p);
			}
			lcsize += 5;
			continue;
		}
		if(s > 0) {
			CPUT(0+s);	/* 1-64 +lc */
			if(debug['L']) {
				Bprint(&bso, " lc+%ld(%ld)\n", s, 0+s);
				Bprint(&bso, "%6lux %P\n",
					p->pc, p);
			}
		} else {
			CPUT(64-s);	/* 65-128 -lc */
			if(debug['L']) {
				Bprint(&bso, " lc%ld(%ld)\n", s, 64-s);
				Bprint(&bso, "%6lux %P\n",
					p->pc, p);
			}
		}
		lcsize++;
	}
	while(lcsize & 1) {
		s = 129;
		CPUT(s);
		lcsize++;
	}
	if(debug['v'] || debug['L'])
		Bprint(&bso, "lcsize = %ld\n", lcsize);
	Bflush(&bso);
}

void
datblk(long s, long n)
{
	Prog *p;
	char *cast;
	long l, fl, j, d;
	int i, c;

	memset(buf.dbuf, 0, n+100);
	for(p = datap; p != P; p = p->link) {
		curp = p;
		l = p->from.u1.asym->value + p->from.u0.aoffset - s;
		c = p->reg;
		i = 0;
		if(l < 0) {
			if(l+c <= 0)
				continue;
			while(l < 0) {
				l++;
				i++;
			}
		}
		if(l >= n)
			continue;
		if(p->as != AINIT && p->as != ADYNT) {
			for(j=l+(c-i)-1; j>=l; j--)
				if(buf.dbuf[j]) {
					print("%P\n", p);
					diag("multiple initialization");
					break;
				}
		}
		switch(p->to.type) {
		default:
			diag("unknown mode in initialization%P", p);
			break;

		case D_FCONST:
			switch(c) {
			default:
			case 4:
				fl = ieeedtof(p->to.u0.aieee);
				cast = (char*)&fl;
				for(; i<c; i++) {
					buf.dbuf[l] = cast[fnuxi8[i+4]];
					l++;
				}
				break;
			case 8:
				cast = (char*)p->to.u0.aieee;
				for(; i<c; i++) {
					buf.dbuf[l] = cast[fnuxi8[i]];
					l++;
				}
				break;
			}
			break;

		case D_SCONST:
			for(; i<c; i++) {
				buf.dbuf[l] = p->to.u0.asval[i];
				l++;
			}
			break;

		case D_CONST:
			d = p->to.u0.aoffset;
			if(p->to.u1.asym) {
				if(p->to.u1.asym->type == STEXT ||
				   p->to.u1.asym->type == SLEAF)
					d += p->to.u1.asym->value;
				if(p->to.u1.asym->type == SDATA)
					d += p->to.u1.asym->value + INITDAT;
				if(p->to.u1.asym->type == SBSS)
					d += p->to.u1.asym->value + INITDAT;
			}
			cast = (char*)&d;
			switch(c) {
			default:
				diag("bad nuxi %d %d%P", c, i, curp);
				break;
			case 1:
				for(; i<c; i++) {
					buf.dbuf[l] = cast[inuxi1[i]];
					l++;
				}
				break;
			case 2:
				for(; i<c; i++) {
					buf.dbuf[l] = cast[inuxi2[i]];
					l++;
				}
				break;
			case 4:
				for(; i<c; i++) {
					buf.dbuf[l] = cast[inuxi4[i]];
					l++;
				}
				break;
			}
			break;
		}
	}
	write(cout, buf.dbuf, n);
}

void
asmout(Prog *p, Optab *o)
{
	long o1, o2, o3, o4, o5, v;
	int r, rf, rt;

	o1 = 0;
	o2 = 0;
	o3 = 0;
	o4 = 0;
	o5 = 0;
	switch(o->type) {
	default:
		diag("unknown asm %d", o->type);
		prasm(p);
		break;

	case 0:		/* pseudo ops */
		break;

	case 1:		/* op R,[R],R */
		o1 = oprrr(p->as, p->scond);
		rf = p->from.reg;
		rt = p->to.reg;
		r = p->reg;
		if(p->to.type == D_NONE)
			rt = 0;
		if(r == NREG)
			r = rt;
		o1 |= rf | (r<<16) | (rt<<12);
		break;

	case 2:		/* op $I,[R],R */
		aclass(&p->from);
		o1 = oprrr(p->as, p->scond);
		o1 |= immrot(offset);
		rt = p->to.reg;
		r = p->reg;
		if(p->to.type == D_NONE)
			rt = 0;
		if(r == NREG)
			r = rt;
		o1 |= (r<<16) | (rt<<12);
		break;

	case 3:		/* add R<<[IR],[R],R */
		aclass(&p->from);
		o1 = oprrr(p->as, p->scond);
		o1 |= p->from.u0.aoffset;
		rt = p->to.reg;
		r = p->reg;
		if(p->to.type == D_NONE)
			rt = 0;
		if(r == NREG)
			r = rt;
		o1 |= (r<<16) | (rt<<12);
		break;

	case 4:		/* add $I,[R],R */
		aclass(&p->from);
		o1 = oprrr(AADD, p->scond);
		o1 |= immrot(offset);
		r = p->from.reg;
		if(r == NREG)
			r = o->param;
		o1 |= r << 16;
		o1 |= p->to.reg << 12;
		break;

	case 5:		/* bra s */
		v = -8;
		if(p->cond != P)
			v = (p->cond->pc - pc) - 8;
		o1 = opbra(p->as, p->scond);
		o1 |= (v >> 2) & 0xffffff;
		break;

	case 6:		/* b ,O(R) -> add $O,R,PC */
		aclass(&p->to);
		o1 = oprrr(AADD, p->scond);
		o1 |= immrot(offset);
		o1 |= p->to.reg << 16;
		o1 |= REGPC << 12;
		break;

	case 7:		/* bl ,O(R) -> mov PC,link; add $O,R,PC */
		aclass(&p->to);
		o1 = oprrr(AADD, p->scond);
		o1 |= immrot(0);
		o1 |= REGPC << 16;
		o1 |= REGLINK << 12;

		o2 = oprrr(AADD, p->scond);
		o2 |= immrot(offset);
		o2 |= p->to.reg << 16;
		o2 |= REGPC << 12;
		break;

	case 8:		/* sll $c,[R],R -> mov (R<<$c),R */
		aclass(&p->from);
		o1 = oprrr(p->as, p->scond);
		r = p->reg;
		if(r == NREG)
			r = p->to.reg;
		o1 |= r;
		o1 |= (offset&31) << 7;
		o1 |= p->to.reg << 12;
		break;

	case 9:		/* sll R,[R],R -> mov (R<<R),R */
		o1 = oprrr(p->as, p->scond);
		r = p->reg;
		if(r == NREG)
			r = p->to.reg;
		o1 |= r;
		o1 |= (p->from.reg << 8) | (1<<4);
		o1 |= p->to.reg << 12;
		break;

	case 10:	/* swi [$con] */
		o1 = oprrr(p->as, p->scond);
		if(p->to.type != D_NONE) {
			aclass(&p->to);
			o1 |= offset & 0xffffff;
		}
		break;

	case 11:	/* word */
		aclass(&p->to);
		o1 = offset;
		break;

	case 12:	/* movw $lcon, reg */
		if(!p->cond) {
			aclass(&p->from);
			v = immrot(~offset);
			if(v == 0) {
				diag("missing literal");
				prasm(p);
				break;
			}
			o1 = oprrr(AMVN, p->scond);
			o1 |= v;
			o1 |= p->to.reg << 12;
		} else {
			v = p->cond->pc - p->pc - 8;
			o1 = olr(v, REGPC, p->to.reg, p->scond);
		}
		break;

	case 13:	/* op $lcon, [R], R */
		if(!p->cond) {
			aclass(&p->from);
			v = immrot(~offset);
			if(v == 0) {
				diag("missing literal");
				prasm(p);
				break;
			}
			o1 = oprrr(AMVN, p->scond);
			o1 |= v;
			o1 |= REGTMP << 12;
		} else {
			v = p->cond->pc - p->pc - 8;
			o1 = olr(v, REGPC, REGTMP, p->scond);
		}

		o2 = oprrr(p->as, p->scond);
		o2 |= REGTMP;
		r = p->reg;
		if(r == NREG)
			r = p->to.reg;
		o2 |= r << 16;
		if(p->to.type != D_NONE)
			o2 |= p->to.reg << 12;
		break;

	case 14:	/* movb/movbu/movh/movhu R,R */
		o1 = oprrr(ASLL, p->scond);

		if(p->as == AMOVBU || p->as == AMOVHU)
			o2 = oprrr(ASRL, p->scond);
		else
			o2 = oprrr(ASRA, p->scond);

		r = p->to.reg;
		o1 |= (p->from.reg)|(r<<12);
		o2 |= (r)|(r<<12);
		if(p->as == AMOVB || p->as == AMOVBU) {
			o1 |= (24<<7);
			o2 |= (24<<7);
		} else {
			o1 |= (16<<7);
			o2 |= (16<<7);
		}
		break;

	case 15:	/* mul r,[r,]r */
		o1 = oprrr(p->as, p->scond);
		rf = p->from.reg;
		rt = p->to.reg;
		r = p->reg;
		if(r == NREG)
			r = rt;
		if(rt == r) {
			r = rf;
			rf = rt;
		}
		if(0)
		if(rt == r || rf == REGPC || r == REGPC || rt == REGPC) {
			diag("bad registers in MUL");
			prasm(p);
		}
		o1 |= (rf<<8) | r | (rt<<16);
		break;


	case 16:	/* div r,[r,]r */
		o1 = 0xf << 28;
		o2 = 0;
		break;

	case 20:	/* mov/movb/movbu R,O(R) */
		aclass(&p->to);
		r = p->to.reg;
		if(r == NREG)
			r = o->param;
		o1 = osr(p->from.reg, offset, r, p->scond);
		if(p->as != AMOVW)
			o1 |= 1<<22;
		break;

	case 21:	/* mov/movbu O(R),R -> lr */
		aclass(&p->from);
		r = p->from.reg;
		if(r == NREG)
			r = o->param;
		o1 = olr(offset, r, p->to.reg, p->scond);
		if(p->as != AMOVW)
			o1 |= 1<<22;
		break;

	case 22:	/* movb/movh/movhu O(R),R -> lr,shl,shr */
		aclass(&p->from);
		r = p->from.reg;
		if(r == NREG)
			r = o->param;
		o1 = olr(offset, r, p->to.reg, p->scond);

		o2 = oprrr(ASLL, p->scond);
		o3 = oprrr(ASRA, p->scond);
		r = p->to.reg;
		if(p->as == AMOVB) {
			o2 |= (24<<7)|(r)|(r<<12);
			o3 |= (24<<7)|(r)|(r<<12);
		} else {
			o2 |= (16<<7)|(r)|(r<<12);
			if(p->as == AMOVHU)
				o3 = oprrr(ASRL, p->scond);
			o3 |= (16<<7)|(r)|(r<<12);
		}
		break;

	case 23:	/* movh/movhu R,O(R) -> sb,sb */
		aclass(&p->to);
		r = p->to.reg;
		if(r == NREG)
			r = o->param;
		o1 = osr(p->from.reg, offset, r, p->scond);
		o1 |= 1<<22;

		o2 = oprrr(ASRL, p->scond);
		o2 |= (8<<7)|(p->from.reg)|(REGTMP<<12);

		o3 = osr(REGTMP, offset+1, r, p->scond);
		o3 |= 1<<22;
		break;

	case 30:	/* mov/movb/movbu R,L(R) */
		if(!p->cond) {
			aclass(&p->from);
			v = immrot(~offset);
			if(v == 0) {
				diag("missing literal");
				prasm(p);
				break;
			}
			o1 = oprrr(AMVN, p->scond);
			o1 |= v;
			o1 |= REGTMP << 12;
		} else {
			v = p->cond->pc - p->pc - 8;
			o1 = olr(v, REGPC, REGTMP, p->scond);
		}

		r = p->to.reg;
		if(r == NREG)
			r = o->param;
		o2 = osrr(p->from.reg, REGTMP,r, p->scond);
		if(p->as != AMOVW)
			o2 |= 1<<22;
		break;

	case 31:	/* mov/movbu L(R),R -> lr[b] */
	case 32:	/* movh/movb L(R),R -> lr[b] */
		if(!p->cond) {
			aclass(&p->from);
			v = immrot(~offset);
			if(v == 0) {
				diag("missing literal");
				prasm(p);
				break;
			}
			o1 = oprrr(AMVN, p->scond);
			o1 |= v;
			o1 |= REGTMP << 12;
		} else {
			v = p->cond->pc - p->pc - 8;
			o1 = olr(v, REGPC, REGTMP, p->scond);
		}

		r = p->from.reg;
		if(r == NREG)
			r = o->param;
		o2 = olrr(REGTMP,r, p->to.reg, p->scond);
		if(p->as != AMOVW)
			o2 |= 1<<22;
		if(o->type == 31)
			break;

		o3 = oprrr(ASLL, p->scond);

		if(p->as == AMOVBU || p->as == AMOVHU)
			o4 = oprrr(ASRL, p->scond);
		else
			o4 = oprrr(ASRA, p->scond);

		r = p->to.reg;
		o3 |= (r)|(r<<12);
		o4 |= (r)|(r<<12);
		if(p->as == AMOVB || p->as == AMOVBU) {
			o3 |= (24<<7);
			o4 |= (24<<7);
		} else {
			o3 |= (16<<7);
			o4 |= (16<<7);
		}
		break;

	case 34:	/* mov $lacon,R */
		if(!p->cond) {
			aclass(&p->from);
			v = immrot(~offset);
			if(v == 0) {
				diag("missing literal");
				prasm(p);
				break;
			}
			o1 = oprrr(AMVN, p->scond);
			o1 |= v;
			o1 |= REGTMP << 12;
		} else {
			v = p->cond->pc - p->pc - 8;
			o1 = olr(v, REGPC, REGTMP, p->scond);
		}

		o2 = oprrr(AADD, p->scond);
		o2 |= REGTMP;
		r = p->reg;
		if(r == NREG)
			r = o->param;
		o2 |= r << 16;
		if(p->to.type != D_NONE)
			o2 |= p->to.reg << 12;
		break;

	case 35:	/* mov PSR,R */
		o1 = (2<<23) | (0xf<<16) | (0<<0);
		o1 |= (p->scond & C_SCOND) << 28;
		o1 |= (p->from.reg & 1) << 22;
		o1 |= p->to.reg << 12;
		break;

	case 36:	/* mov R,PSR */
		o1 = (2<<23) | (0x29f<<12) | (0<<4);
		if(p->scond & C_FBIT)
			o1 ^= 0x010 << 12;
		o1 |= (p->scond & C_SCOND) << 28;
		o1 |= (p->to.reg & 1) << 22;
		o1 |= p->from.reg << 0;
		break;

	case 37:	/* mov $con,PSR */
		aclass(&p->from);
		o1 = (2<<23) | (0x29f<<12) | (0<<4);
		if(p->scond & C_FBIT)
			o1 ^= 0x010 << 12;
		o1 |= (p->scond & C_SCOND) << 28;
		o1 |= immrot(offset);
		o1 |= (p->to.reg & 1) << 22;
		o1 |= p->from.reg << 0;
		break;

	case 38:	/* movm $con,oreg -> stm */
		o1 = (0x4 << 25);
		o1 |= p->from.u0.aoffset & 0xffff;
		o1 |= p->to.reg << 16;
		aclass(&p->to);
		goto movm;

	case 39:	/* movm oreg,$con -> ldm */
		o1 = (0x4 << 25) | (1 << 20);
		o1 |= p->to.u0.aoffset & 0xffff;
		o1 |= p->from.reg << 16;
		aclass(&p->from);
	movm:
		if(offset != 0)
			diag("offset must be zero in MOVM");
		o1 |= (p->scond & C_SCOND) << 28;
		if(p->scond & C_PBIT)
			o1 |= 1 << 24;
		if(p->scond & C_UBIT)
			o1 |= 1 << 23;
		if(p->scond & C_SBIT)
			o1 |= 1 << 22;
		if(p->scond & C_WBIT)
			o1 |= 1 << 21;
		break;

	case 40:	/* swp oreg,reg,reg */
		aclass(&p->from);
		if(offset != 0)
			diag("offset must be zero in SWP");
		o1 = (0x2<<23) | (0x9<<4);
		if(p->as != ASWPW)
			o1 |= 1 << 22;
		o1 |= p->from.reg << 16;
		o1 |= p->reg << 0;
		o1 |= p->to.reg << 12;
		o1 |= (p->scond & C_SCOND) << 28;
		break;

	case 41:	/* rfe -> movm.s.w.u 0(r13),[r15] */
		o1 = 0xe8fd8000;
		break;

	case 50:	/* floating point */
		o1 = 0xe6000010;	/* undefined */
		break;
	}

	v = p->pc;
	switch(o->size) {
	default:
		if(debug['a'])
			Bprint(&bso, " %.8lux:\t\t%P\n", v, p);
		break;
	case 4:
		if(debug['a'])
			Bprint(&bso, " %.8lux: %.8lux\t%P\n", v, o1, p);
		lputl(o1);
		break;
	case 8:
		if(debug['a'])
			Bprint(&bso, " %.8lux: %.8lux %.8lux%P\n", v, o1, o2, p);
		lputl(o1);
		lputl(o2);
		break;
	case 12:
		if(debug['a'])
			Bprint(&bso, " %.8lux: %.8lux %.8lux %.8lux%P\n", v, o1, o2, o3, p);
		lputl(o1);
		lputl(o2);
		lputl(o3);
		break;
	case 16:
		if(debug['a'])
			Bprint(&bso, " %.8lux: %.8lux %.8lux %.8lux %.8lux%P\n",
				v, o1, o2, o3, o4, p);
		lputl(o1);
		lputl(o2);
		lputl(o3);
		lputl(o4);
		break;
	case 20:
		if(debug['a'])
			Bprint(&bso, " %.8lux: %.8lux %.8lux %.8lux %.8lux %.8lux%P\n",
				v, o1, o2, o3, o4, o5, p);
		lputl(o1);
		lputl(o2);
		lputl(o3);
		lputl(o4);
		lputl(o5);
		break;
	}
}

long
oprrr(int a, int sc)
{
	long o;

	o = (sc & C_SCOND) << 28;
	if(sc & C_SBIT)
		o |= 1 << 20;
	if(sc & (C_PBIT|C_WBIT))
		diag(".P/.W on dp instruction");
	switch(a) {
	case AMULU:
	case AMUL:	return o | (0x0<<21) | (0x9<<4);
	case AAND:	return o | (0x0<<21);
	case AEOR:	return o | (0x1<<21);
	case ASUB:	return o | (0x2<<21);
	case ARSB:	return o | (0x3<<21);
	case AADD:	return o | (0x4<<21);
	case AADC:	return o | (0x5<<21);
	case ASBC:	return o | (0x6<<21);
	case ARSC:	return o | (0x7<<21);
	case ATST:	return o | (0x8<<21) | (1<<20);
	case ATEQ:	return o | (0x9<<21) | (1<<20);
	case ACMP:	return o | (0xa<<21) | (1<<20);
	case ACMN:	return o | (0xb<<21);
	case AORR:	return o | (0xc<<21);
	case AMOVW:	return o | (0xd<<21);
	case ABIC:	return o | (0xe<<21);
	case AMVN:	return o | (0xf<<21);
	case ASLL:	return o | (0xd<<21) | (0<<5);
	case ASRL:	return o | (0xd<<21) | (1<<5);
	case ASRA:	return o | (0xd<<21) | (2<<5);
	case ASWI:	return o | (0xf<<24);
	}
	diag("bad rrr %d", a);
	prasm(curp);
	return 0;
}

long
opbra(int a, int sc)
{

	if(sc & (C_SBIT|C_PBIT|C_WBIT))
		diag(".S/.P/.W on bra instruction");
	sc &= C_SCOND;
	if(a == ABL)
		return (sc<<28)|(0x5<<25)|(0x1<<24);
	if(sc != 0xe)
		diag(".COND on bcond instruction");
	switch(a) {
	case ABEQ:	return (0x0<<28)|(0x5<<25);
	case ABNE:	return (0x1<<28)|(0x5<<25);
	case ABCS:	return (0x2<<28)|(0x5<<25);
	case ABHS:	return (0x2<<28)|(0x5<<25);
	case ABCC:	return (0x3<<28)|(0x5<<25);
	case ABLO:	return (0x3<<28)|(0x5<<25);
	case ABMI:	return (0x4<<28)|(0x5<<25);
	case ABPL:	return (0x5<<28)|(0x5<<25);
	case ABVS:	return (0x6<<28)|(0x5<<25);
	case ABVC:	return (0x7<<28)|(0x5<<25);
	case ABHI:	return (0x8<<28)|(0x5<<25);
	case ABLS:	return (0x9<<28)|(0x5<<25);
	case ABGE:	return (0xa<<28)|(0x5<<25);
	case ABLT:	return (0xb<<28)|(0x5<<25);
	case ABGT:	return (0xc<<28)|(0x5<<25);
	case ABLE:	return (0xd<<28)|(0x5<<25);
	case AB:	return (0xe<<28)|(0x5<<25);
	}
	diag("bad bra %A", a);
	prasm(curp);
	return 0;
}

long
olr(long v, int b, int r, int sc)
{
	long o;

	if(sc & C_SBIT)
		diag(".S on LDR/STR instruction");
	o = (sc & C_SCOND) << 28;
	if(!(sc & C_PBIT))
		o |= 1 << 24;
	if(sc & C_WBIT)
		o |= 1 << 21;
	o |= (0x1<<26) | (1<<23) | (1<<20);
	if(v < 0) {
		v = -v;
		o ^= 1 << 23;
	}
	if(v >= (1<<12))
		diag("literal span too large: %d", v);
	o |= v;
	o |= b << 16;
	o |= r << 12;
	return o;
}

long
osr(int r, long v, int b, int sc)
{

	return olr(v, b, r, sc) ^ (1<<20);
}

long
osrr(int r, int i, int b, int sc)
{

	return olr(i, b, r, sc) ^ ((1<<25) | (1<<20));
}

long
olrr(int i, int b, int r, int sc)
{

	return olr(i, b, r, sc) ^ (1<<25);
}
