#include "gc.h"

void
listinit(void)
{

	fmtinstall('R', Rconv);
	fmtinstall('A', Aconv);
	fmtinstall('D', Dconv);
	fmtinstall('P', Pconv);
	fmtinstall('S', Sconv);
	fmtinstall('X', Xconv);
	fmtinstall('B', Bconv);
}

int
Pconv(va_list *arg, Fconv *fp)
{
	char str[STRINGSZ], s[20];
	Prog *p;

	p = va_arg(*arg, Prog*);
	sprint(str, "	%A	%D,%D", p->as, &p->from, &p->to);
	if(p->from.field) {
		sprint(s, ",%d,%d", p->to.field, p->from.field);
		strcat(str, s);
	}
	strconv(str, fp);
	return 0;
}

int
Aconv(va_list *arg, Fconv *fp)
{
	int r;

	r = va_arg(*arg, int);
	strconv(anames[r], fp);
	return 0;
}

int
Xconv(va_list *arg, Fconv *fp)
{
	char str[20], s[10];
	int i, o0, o1;

	o0 = va_arg(*arg, int);
	o1 = va_arg(*arg, int);
	str[0] = 0;
	i = o0 & D_MASK;
	if(i != D_NONE) {
		sprint(str, "(%R.", i);
		i = o1;
		sprint(s, "%c*%c)",
			"WWWWLLLL"[i],
			"12481248"[i]);
		strcat(str, s);
	}
	strconv(str, fp);
	return 0;
}

int
Dconv(va_list *arg, Fconv *fp)
{
	char str[40], s[20];
	Adr *a;
	int i, j;
	long d;

	a = va_arg(*arg, Adr*);
	i = a->index;
	if(i != D_NONE) {
		a->index = D_NONE;
		d = a->u0.s0.displace;
		j = a->scale;
		a->u0.s0.displace = 0;
		switch(i & I_MASK) {
		default:
			sprint(str, "???%ld(%D%X)", d, a, i, j);
			break;

		case I_INDEX1:
			sprint(str, "%D%X", a, i, a->scale);
			break;

		case I_INDEX2:
			if(d)
				sprint(str, "%ld(%D)%X", d, a, i, j);
			else
				sprint(str, "(%D)%X", a, i, j);
			break;

		case I_INDEX3:
			if(d)
				sprint(str, "%ld(%D%X)", d, a, i, j);
			else
				sprint(str, "(%D%X)", a, i, j);
			break;
		}
		a->u0.s0.displace = d;
		a->index = i;
		goto out;
	}
	i = a->type;
	j = i & I_MASK;
	if(j) {
		a->type = i & D_MASK;
		d = a->u0.s0.offset;
		a->u0.s0.offset = 0;
		switch(j) {
		case I_INDINC:
			sprint(str, "(%D)+", a);
			break;

		case I_INDDEC:
			sprint(str, "-(%D)", a);
			break;

		case I_INDIR:
			if(a->type == D_CONST)
				sprint(str, "%ld", d);
			else
			if(d)
				sprint(str, "%ld(%D)", d, a);
			else
				sprint(str, "(%D)", a);
			break;

		case I_ADDR:
			a->u0.s0.offset = d;
			sprint(str, "$%D", a);
			break;
		}
		a->type = i;
		a->u0.s0.offset = d;
		goto out;
	}
	switch(i) {

	default:
		sprint(str, "%R", i);
		break;

	case D_NONE:
		str[0] = 0;
		break;

	case D_BRANCH:
		sprint(str, "%ld(PC)", a->u0.s0.offset-pc);
		break;

	case D_EXTERN:
		sprint(str, "%s+%ld(SB)", a->sym->name, a->u0.s0.offset);
		break;

	case D_STATIC:
		sprint(str, "%s<>+%ld(SB)", a->sym->name, a->u0.s0.offset);
		break;

	case D_AUTO:
		sprint(str, "%s-%ld(SP)", a->sym->name, -a->u0.s0.offset);
		break;

	case D_PARAM:
		sprint(str, "%s+%ld(FP)", a->sym->name, a->u0.s0.offset);
		break;

	case D_CONST:
		sprint(str, "$%ld", a->u0.s0.offset);
		break;

	case D_STACK:
		sprint(str, "TOS+%ld", a->u0.s0.offset);
		break;

	case D_FCONST:
		sprint(str, "$%.17e", a->u0.dval);
		goto out;

	case D_SCONST:
		sprint(str, "$\"%S\"", a->u0.sval);
		goto out;
	}
	if(a->u0.s0.displace) {
		sprint(s, "/%ld", a->u0.s0.displace);
		strcat(str, s);
	}
out:
	strconv(str, fp);
	return 0;
}

int
Rconv(va_list *arg, Fconv *fp)
{
	char str[20];
	int r;

	r = va_arg(*arg, int);
	if(r >= D_R0 && r < D_R0+NREG)
		sprint(str, "R%d", r-D_R0);
	else
	if(r >= D_A0 && r < D_A0+NREG)
		sprint(str, "A%d", r-D_A0);
	else
	if(r >= D_F0 && r < D_F0+NREG)
		sprint(str, "F%d", r-D_F0);
	else
	switch(r) {

	default:
		sprint(str, "gok(%d)", r);
		break;

	case D_NONE:
		sprint(str, "NONE");
		break;

	case D_TOS:
		sprint(str, "TOS");
		break;

	case D_CCR:
		sprint(str, "CCR");
		break;

	case D_SR:
		sprint(str, "SR");
		break;

	case D_SFC:
		sprint(str, "SFC");
		break;

	case D_DFC:
		sprint(str, "DFC");
		break;

	case D_CACR:
		sprint(str, "CACR");
		break;

	case D_USP:
		sprint(str, "USP");
		break;

	case D_VBR:
		sprint(str, "VBR");
		break;

	case D_CAAR:
		sprint(str, "CAAR");
		break;

	case D_MSP:
		sprint(str, "MSP");
		break;

	case D_ISP:
		sprint(str, "ISP");
		break;

	case D_TREE:
		sprint(str, "TREE");
		break;

	case D_FPCR:
		sprint(str, "FPCR");
		break;

	case D_FPSR:
		sprint(str, "FPSR");
		break;

	case D_FPIAR:
		sprint(str, "FPIAR");
		break;

	case D_TC:
		sprint(str, "TC");
		break;

	case D_ITT0:
		sprint(str, "ITT0");
		break;

	case D_ITT1:
		sprint(str, "ITT1");
		break;

	case D_DTT0:
		sprint(str, "DTT0");
		break;

	case D_DTT1:
		sprint(str, "DTT1");
		break;

	case D_MMUSR:
		sprint(str, "MMUSR");
		break;
	case D_URP:
		sprint(str, "URP");
		break;

	case D_SRP:
		sprint(str, "SRP");
		break;
	}
	strconv(str, fp);
	return 0;
}

int
Sconv(va_list *arg, Fconv *fp)
{
	int i, c;
	char str[30], *p, *s;

	s = va_arg(*arg, char*);
	p = str;
	for(i=0; i<sizeof(double); i++) {
		c = s[i] & 0xff;
		if(c != '\\' && c != '"' && isprint(c)) {
			*p++ = c;
			continue;
		}
		*p++ = '\\';
		switch(c) {
		case 0:
			*p++ = '0';
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
		}
		*p++ = ((c>>6) & 7) + '0';
		*p++ = ((c>>3) & 7) + '0';
		*p++ = ((c>>0) & 7) + '0';
	}
	*p = 0;
	strconv(str, fp);
	return 0;
}
