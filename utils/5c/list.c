#include "gc.h"

void
listinit(void)
{

	fmtinstall('A', Aconv);
	fmtinstall('P', Pconv);
	fmtinstall('S', Sconv);
	fmtinstall('N', Nconv);
	fmtinstall('B', Bconv);
	fmtinstall('D', Dconv);
}

int
Pconv(va_list *arg, Fconv *fp)
{
	char str[STRINGSZ];
	Prog *p;
	int a;

	p = va_arg(*arg, Prog*);
	a = p->as;
	if(a == ADATA)
		sprint(str, "	%A	%D/%d,%D", a, &p->from, p->reg, &p->to);
	else
	if(p->reg == NREG)
		sprint(str, "	%A	%D,%D", a, &p->from, &p->to);
	else
	if(p->from.type != D_FREG)
		sprint(str, "	%A	%D,R%d,%D", a, &p->from, p->reg, &p->to);
	else
		sprint(str, "	%A	%D,F%d,%D", a, &p->from, p->reg, &p->to);
	strconv(str, fp);
	return 0;
}

int
Aconv(va_list *arg, Fconv *fp)
{
	char *s;
	int a;

	a = va_arg(*arg, int);
	s = "???";
	if(a >= AXXX && a < ALAST)
		s = anames[a];
	strconv(s, fp);
	return 0;
}

int
Dconv(va_list *arg, Fconv *fp)
{
	char str[STRINGSZ];
	Adr *a;

	a = va_arg(*arg, Adr*);
	switch(a->type) {

	default:
		sprint(str, "GOK-type(%d)", a->type);
		break;

	case D_NONE:
		str[0] = 0;
		if(a->name != D_NONE || a->reg != NREG || a->sym != S)
			sprint(str, "%N(R%d)(NONE)", a, a->reg);
		break;

	case D_CONST:
		if(a->reg != NREG)
			sprint(str, "$%N(R%d)", a, a->reg);
		else
			sprint(str, "$%N", a);
		break;

	case D_OREG:
		if(a->reg != NREG)
			sprint(str, "%N(R%d)", a, a->reg);
		else
			sprint(str, "%N", a);
		break;

	case D_REG:
		sprint(str, "R%d", a->reg);
		if(a->name != D_NONE || a->sym != S)
			sprint(str, "%N(R%d)(REG)", a, a->reg);
		break;

	case D_FREG:
		sprint(str, "F%d", a->reg);
		if(a->name != D_NONE || a->sym != S)
			sprint(str, "%N(R%d)(REG)", a, a->reg);
		break;

	case D_PSR:
		sprint(str, "PSR", a->reg);
		if(a->name != D_NONE || a->sym != S)
			sprint(str, "%N(PSR)(REG)", a, a->reg);
		break;

	case D_BRANCH:
		sprint(str, "%ld(PC)", a->u0.aoffset-pc);
		break;

	case D_FCONST:
		sprint(str, "$%.17e", a->u0.adval);
		break;

	case D_SCONST:
		sprint(str, "$\"%S\"", a->u0.asval);
		break;
	}
	strconv(str, fp);
	return 0;
}

int
Sconv(va_list *arg, Fconv *fp)
{
	int i, c;
	char str[STRINGSZ], *p, *a;

	a = va_arg(*arg, char*);
	p = str;
	for(i=0; i<NSNAME; i++) {
		c = a[i] & 0xff;
		if(c >= 'a' && c <= 'z' ||
		   c >= 'A' && c <= 'Z' ||
		   c >= '0' && c <= '9' ||
		   c == ' ' || c == '%') {
			*p++ = c;
			continue;
		}
		*p++ = '\\';
		switch(c) {
		case 0:
			*p++ = 'z';
			continue;
		case '\\':
		case '"':
			*p++ = c;
			continue;
		case '\n':
			*p++ = 'n';
			continue;
		case '\t':
			*p++ = 't';
			continue;
		case '\r':
			*p++ = 'r';
			continue;
		case '\f':
			*p++ = 'f';
			continue;
		}
		*p++ = (c>>6) + '0';
		*p++ = ((c>>3) & 7) + '0';
		*p++ = (c & 7) + '0';
	}
	*p = 0;
	strconv(str, fp);
	return 0;
}

int
Nconv(va_list *arg, Fconv *fp)
{
	char str[STRINGSZ];
	Adr *a;
	Sym *s;

	a = va_arg(*arg, Adr*);
	s = a->sym;
	if(s == S) {
		sprint(str, "%ld", a->u0.aoffset);
		goto out;
	}
	switch(a->name) {
	default:
		sprint(str, "GOK-name(%d)", a->name);
		break;

	case D_NONE:
		sprint(str, "%ld", a->u0.aoffset);
		break;

	case D_EXTERN:
		sprint(str, "%s+%ld(SB)", s->name, a->u0.aoffset);
		break;

	case D_STATIC:
		sprint(str, "%s<>+%ld(SB)", s->name, a->u0.aoffset);
		break;

	case D_AUTO:
		sprint(str, "%s-%ld(SP)", s->name, -a->u0.aoffset);
		break;

	case D_PARAM:
		sprint(str, "%s+%ld(FP)", s->name, a->u0.aoffset);
		break;
	}
out:
	strconv(str, fp);
	return 0;
}
