#include "cc.h"

void
complex(Node *n)
{

	if(n == Z)
		return;

	nearln = n->lineno;
	if(debug['t'])
		if(n->op != OCONST)
			prtree(n, "pre complex");
	if(tcom(n))
		return;
	if(debug['t'])
		if(n->op != OCONST)
			prtree(n, "t complex");
	ccom(n);
	if(debug['t'])
		if(n->op != OCONST)
			prtree(n, "c complex");
	acom(n);
	if(debug['t'])
		if(n->op != OCONST)
			prtree(n, "a complex");
	xcom(n);
	if(debug['t'])
		if(n->op != OCONST)
			prtree(n, "x complex");
}

/*
 * evaluate types
 * evaluate lvalues (addable == 1)
 */
enum
{
	ADDROF	= 1<<0,
	CASTOF	= 1<<1,
	ADDROP	= 1<<2,
};

int
tcom(Node *n)
{

	return tcomo(n, ADDROF);
}

int
tcomo(Node *n, int f)
{
	Node *l, *r;
	Type *t;
	int o;

	if(n == Z) {
		diag(Z, "Z in tcom");
		errorexit();
	}
	n->addable = 0;
	l = n->u0.s0.nleft;
	r = n->u0.s0.nright;
		
	switch(n->op) {
	default:
		diag(n, "unknown op in type complex: %O", n->op);
		goto bad;

	case ODOTDOT:
		/*
		 * tcom has already been called on this subtree
		 */
		*n = *n->u0.s0.nleft;
		if(n->type == T)
			goto bad;
		break;

	case OCAST:
		if(n->type == T)
			break;
		if(n->type->width == types[TLONG]->width) {
			if(tcomo(l, ADDROF|CASTOF))
					goto bad;
		} else
			if(tcom(l))
				goto bad;
		if(tcompat(n, l->type, n->type, tcast))
			goto bad;
		break;

	case ORETURN:
		if(l == Z) {
			if(n->type->etype != TVOID)
				warn(n, "null return of a typed function");
			break;
		}
		if(tcom(l))
			goto bad;
		typeext(n->type, l);
		if(tcompat(n, n->type, l->type, tasign))
			break;
		constas(n, n->type, l->type);
		if(!sametype(n->type, l->type)) {
			l = new1(OCAST, l, Z);
			l->type = n->type;
			n->u0.s0.nleft = l;
		}
		break;

	case OAS:
		o = tcom(l);
		if(o | tcom(r))
			goto bad;

		typeext(l->type, r);
		if(tlvalue(l) || tcompat(n, l->type, r->type, tasign))
			goto bad;
		constas(n, l->type, r->type);
		if(!sametype(l->type, r->type)) {
			r = new1(OCAST, r, Z);
			r->type = l->type;
			n->u0.s0.nright = r;
		}
		n->type = l->type;
		break;

	case OASADD:
	case OASSUB:
		o = tcom(l);
		if(o | tcom(r))
			goto bad;
		typeext1(l->type, r);
		if(tlvalue(l) || tcompat(n, l->type, r->type, tasadd))
			goto bad;
		constas(n, l->type, r->type);
		t = l->type;
		arith(n, 0);
		while(n->u0.s0.nleft->op == OCAST)
			n->u0.s0.nleft = n->u0.s0.nleft->u0.s0.nleft;
		if(!sametype(t, n->type)) {
			r = new1(OCAST, n->u0.s0.nright, Z);
			r->type = t;
			n->u0.s0.nright = r;
			n->type = t;
		}
		break;

	case OASMUL:
	case OASLMUL:
	case OASDIV:
	case OASLDIV:
		o = tcom(l);
		if(o | tcom(r))
			goto bad;
		typeext1(l->type, r);
		if(tlvalue(l) || tcompat(n, l->type, r->type, tmul))
			goto bad;
		constas(n, l->type, r->type);
		t = l->type;
		arith(n, 0);
		while(n->u0.s0.nleft->op == OCAST)
			n->u0.s0.nleft = n->u0.s0.nleft->u0.s0.nleft;
		if(!sametype(t, n->type)) {
			r = new1(OCAST, n->u0.s0.nright, Z);
			r->type = t;
			n->u0.s0.nright = r;
			n->type = t;
		}
		if(typeu[n->type->etype]) {
			if(n->op == OASDIV)
				n->op = OASLDIV;
			if(n->op == OASMUL)
				n->op = OASLMUL;
		}
		break;

	case OASLSHR:
	case OASASHR:
	case OASASHL:
		o = tcom(l);
		if(o | tcom(r))
			goto bad;
		if(tlvalue(l) || tcompat(n, l->type, r->type, tand))
			goto bad;
		n->type = l->type;
		if(typeu[n->type->etype]) {
			if(n->op == OASASHR)
				n->op = OASLSHR;
		}
		break;

	case OASMOD:
	case OASLMOD:
	case OASOR:
	case OASAND:
	case OASXOR:
		o = tcom(l);
		if(o | tcom(r))
			goto bad;
		if(tlvalue(l) || tcompat(n, l->type, r->type, tand))
			goto bad;
		t = l->type;
		arith(n, 0);
		while(n->u0.s0.nleft->op == OCAST)
			n->u0.s0.nleft = n->u0.s0.nleft->u0.s0.nleft;
		if(!sametype(t, n->type)) {
			r = new1(OCAST, n->u0.s0.nright, Z);
			r->type = t;
			n->u0.s0.nright = r;
			n->type = t;
		}
		if(typeu[n->type->etype]) {
			if(n->op == OASMOD)
				n->op = OASLMOD;
		}
		break;

	case OPREINC:
	case OPREDEC:
	case OPOSTINC:
	case OPOSTDEC:
		if(tcom(l))
			goto bad;
		if(tlvalue(l) || tcompat(n, l->type, tint, tadd))
			goto bad;
		n->type = l->type;
		if(n->type->etype == TIND)
		if(n->type->link->width < 1)
			diag(n, "illegal pointer operation");
		break;

	case OEQ:
	case ONE:
		o = tcom(l);
		if(o | tcom(r))
			goto bad;
		typeext(l->type, r);
		typeext(r->type, l);
		if(tcompat(n, l->type, r->type, trel))
			goto bad;
		arith(n, 0);
		n->type = tint;
		break;

	case OLT:
	case OGE:
	case OGT:
	case OLE:
		o = tcom(l);
		if(o | tcom(r))
			goto bad;
		typeext1(l->type, r);
		typeext1(r->type, l);
		if(tcompat(n, l->type, r->type, trel))
			goto bad;
		arith(n, 0);
		if(typeu[n->type->etype])
			n->op = logrel[relindex(n->op)];
		n->type = tint;
		break;

	case OCOND:
		o = tcom(l);
		o |= tcom(r->u0.s0.nleft);
		if(o | tcom(r->u0.s0.nright))
			goto bad;
		if(r->u0.s0.nright->type->etype == TIND && vconst(r->u0.s0.nleft) == 0) {
			r->u0.s0.nleft->type = r->u0.s0.nright->type;
			r->u0.s0.nleft->u0.nvconst = 0;
		}
		if(r->u0.s0.nleft->type->etype == TIND && vconst(r->u0.s0.nright) == 0) {
			r->u0.s0.nright->type = r->u0.s0.nleft->type;
			r->u0.s0.nright->u0.nvconst = 0;
		}
		if(sametype(r->u0.s0.nright->type, r->u0.s0.nleft->type)) {
			r->type = r->u0.s0.nright->type;
			n->type = r->type;
			break;
		}
		if(tcompat(r, r->u0.s0.nleft->type, r->u0.s0.nright->type, trel))
			goto bad;
		arith(r, 0);
		n->type = r->type;
		break;

	case OADD:
		o = tcom(l);
		if(o | tcom(r))
			goto bad;
		if(tcompat(n, l->type, r->type, tadd))
			goto bad;
		arith(n, 1);
		break;

	case OSUB:
		o = tcom(l);
		if(o | tcom(r))
			goto bad;
		if(tcompat(n, l->type, r->type, tsub))
			goto bad;
		arith(n, 1);
		break;

	case OMUL:
	case OLMUL:
	case ODIV:
	case OLDIV:
		o = tcom(l);
		if(o | tcom(r))
			goto bad;
		if(tcompat(n, l->type, r->type, tmul))
			goto bad;
		arith(n, 1);
		if(typeu[n->type->etype]) {
			if(n->op == ODIV)
				n->op = OLDIV;
			if(n->op == OMUL)
				n->op = OLMUL;
		}
		break;

	case OLSHR:
	case OASHL:
	case OASHR:
		o = tcom(l);
		if(o | tcom(r))
			goto bad;
		if(tcompat(n, l->type, r->type, tand))
			goto bad;
		n->u0.s0.nright = Z;
		arith(n, 1);
		n->u0.s0.nright = new1(OCAST, r, Z);
		n->u0.s0.nright->type = tint;
		if(typeu[n->type->etype])
			if(n->op == OASHR)
				n->op = OLSHR;
		break;

	case OAND:
	case OOR:
	case OXOR:
		o = tcom(l);
		if(o | tcom(r))
			goto bad;
		if(tcompat(n, l->type, r->type, tand))
			goto bad;
		arith(n, 1);
		break;

	case OMOD:
	case OLMOD:
		o = tcom(l);
		if(o | tcom(r))
			goto bad;
		if(tcompat(n, l->type, r->type, tand))
			goto bad;
		arith(n, 1);
		if(typeu[n->type->etype])
			n->op = OLMOD;
		break;

	case ONOT:
		if(tcom(l))
			goto bad;
		if(tcompat(n, T, l->type, tnot))
			goto bad;
		n->type = tint;
		break;

	case OANDAND:
	case OOROR:
		o = tcom(l);
		if(o | tcom(r))
			goto bad;
		if(tcompat(n, T, l->type, tnot) |
		   tcompat(n, T, r->type, tnot))
			goto bad;
		n->type = tint;
		break;

	case OCOMMA:
		o = tcom(l);
		if(o | tcom(r))
			goto bad;
		n->type = r->type;
		break;


	case OSIZE:
		if(l != Z) {
			if(l->op != OSTRING && l->op != OLSTRING)
				if(tcomo(l, 0))
					goto bad;
			if(l->op == OBIT) {
				diag(n, "sizeof bitfield");
				goto bad;
			}
			n->type = l->type;
		}
		if(n->type == T)
			goto bad;
		if(n->type->width <= 0) {
			diag(n, "sizeof undefined type");
			goto bad;
		}
		if(n->type->etype == TFUNC) {
			diag(n, "sizeof function");
			goto bad;
		}
		n->op = OCONST;
		n->u0.s0.nleft = Z;
		n->u0.s0.nright = Z;
		n->u0.nvconst = convvtox(n->type->width, tint->etype);
		n->type = tint;
		break;

	case OFUNC:
		o = tcomo(l, 0);
		if(o)
			goto bad;
		if(l->type->etype == TIND && l->type->link->etype == TFUNC) {
			l = new1(OIND, l, Z);
			l->type = l->u0.s0.nleft->type->link;
			n->u0.s0.nleft = l;
		}
		if(tcompat(n, T, l->type, tfunct))
			goto bad;
		if(o | tcoma(l, r, l->type->down, 1))
			goto bad;
		n->type = l->type->link;
		if(!debug['B'])
			if(l->type->down == T || l->type->down->etype == TOLD) {
				nerrors--;
				diag(n, "function args not checked: %F", l);
			}
		break;

	case ONAME:
		if(n->type == T) {
			diag(n, "name not declared: %F", n);
			goto bad;
		}
		if(n->type->etype == TENUM) {
			n->op = OCONST;
			n->type = n->sym->tenum;
			if(!typefd[n->type->etype])
				n->u0.nvconst = n->sym->u0.svconst;
			else
				n->u0.nfconst = n->sym->u0.sfconst;
			break;
		}
		n->addable = 1;
		if(n->class == CEXREG) {
			n->op = OREGISTER;
			n->u0.s2.nreg = n->sym->offset;
			n->u0.s2.noffset = 0;
			break;
		}
		break;

	case OLSTRING:
		if(n->type->link != types[TUSHORT]) {
			o = outstring(0, 0);
			while(o & 3) {
				outlstring(L"", sizeof(ushort));
				o = outlstring(0, 0);
			}
		}
		n->op = ONAME;
		n->u0.s2.noffset = outlstring(n->u0.nrstring, n->type->width);
		n->addable = 1;
		break;

	case OSTRING:
		if(n->type->link != types[TCHAR]) {
			o = outstring(0, 0);
			while(o & 3) {
				outstring("", 1);
				o = outstring(0, 0);
			}
		}
		n->op = ONAME;
		n->u0.s2.noffset = outstring(n->u0.ncstring, n->type->width);
		n->addable = 1;
		break;

	case OCONST:
		break;

	case ODOT:
		if(tcom(l))
			goto bad;
		if(tcompat(n, T, l->type, tdot))
			goto bad;
		if(tcomd(n))
			goto bad;
		break;

	case OADDR:
		if(tcomo(l, ADDROP))
			goto bad;
		if(tlvalue(l))
			goto bad;
		if(l->type->nbits) {
			diag(n, "address of a bit field");
			goto bad;
		}
		if(l->op == OREGISTER) {
			diag(n, "address of a register");
			goto bad;
		}
		n->type = typ(TIND, l->type);
		n->type->width = types[TIND]->width;
		break;

	case OIND:
		if(tcom(l))
			goto bad;
		if(tcompat(n, T, l->type, tindir))
			goto bad;
		n->type = l->type->link;
		n->addable = 1;
		break;

	case OSTRUCT:
		if(tcomx(n))
			goto bad;
		break;
	}
	if(n->type == T)
		goto bad;
	if(n->type->width < 0) {
		diag(n, "structure not fully declared");
		goto bad;
	}
	if(typeaf[n->type->etype]) {
		if(f & ADDROF)
			goto addaddr;
		if(f & ADDROP)
			warn(n, "address of array/func ignored");
	}
	return 0;

addaddr:
	if(tlvalue(n))
		goto bad;
	l = new1(OXXX, Z, Z);
	*l = *n;
	n->op = OADDR;
	if(l->type->etype == TARRAY)
		l->type = l->type->link;
	n->u0.s0.nleft = l;
	n->u0.s0.nright = Z;
	n->addable = 0;
	n->type = typ(TIND, l->type);
	n->type->width = types[TIND]->width;
	return 0;

bad:
	n->type = T;
	return 1;
}

int
tcoma(Node *l, Node *n, Type *t, int f)
{
	Node *n1;
	int o;

	if(t != T)
	if(t->etype == TOLD || t->etype == TDOT)	/* .../old in prototype */
		t = T;
	if(n == Z) {
		if(t != T && !sametype(t, types[TVOID])) {
			diag(n, "not enough function arguments: %F", l);
			return 1;
		}
		return 0;
	}
	if(n->op == OLIST) {
		o = tcoma(l, n->u0.s0.nleft, t, 0);
		if(t != T) {
			t = t->down;
			if(t == T)
				t = types[TVOID];
		}
		return o | tcoma(l, n->u0.s0.nright, t, 1);
	}
	if(f && t != T)
		tcoma(l, Z, t->down, 0);
	if(tcom(n) || tcompat(n, T, n->type, targ))
		return 1;
	if(sametype(t, types[TVOID])) {
		diag(n, "too many function arguments: %F", l);
		return 1;
	}
	if(t != T) {
		typeext(t, n);
		if(stcompat(nodproto, t, n->type, tasign)) {
			diag(l, "argument prototype mismatch \"%T\" for \"%T\": %F",
				n->type, t, l);
			return 1;
		}
		constas(l, t, n->type);
		switch(t->etype) {
		case TCHAR:
		case TSHORT:
			t = tint;
			break;

		case TUCHAR:
		case TUSHORT:
			t = tuint;
			break;
		}
	} else
	switch(n->type->etype)
	{
	case TCHAR:
	case TSHORT:
		t = tint;
		break;

	case TUCHAR:
	case TUSHORT:
		t = tuint;
		break;

	case TFLOAT:
		t = types[TDOUBLE];
	}
	if(t != T && !sametype(t, n->type)) {
		n1 = new1(OXXX, Z, Z);
		*n1 = *n;
		n->op = OCAST;
		n->u0.s0.nleft = n1;
		n->u0.s0.nright = Z;
		n->type = t;
		n->addable = 0;
	}
	return 0;
}

int
tcomd(Node *n)
{
	Type *t;
	long o;

	o = 0;
	t = n->u0.s0.nleft->type;
	for(;;) {
		t = dotsearch(n->sym, t->link, n);
		if(t == T) {
			diag(n, "not a member of struct/union: %F", n);
			return 1;
		}
		o += t->offset;
		if(t->sym == n->sym)
			break;
		if(sametype(t, n->sym->type))
			break;
	}
	makedot(n, t, o);
	return 0;
}

int
tcomx(Node *n)
{
	Type *t;
	Node *l, *r, **ar, **al;
	int e;

	e = 0;
	if(n->type->etype != TSTRUCT) {
		diag(n, "constructor must be a structure");
		return 1;
	}
	l = invert(n->u0.s0.nleft);
	n->u0.s0.nleft = l;
	al = &n->u0.s0.nleft;
	for(t = n->type->link; t != T; t = t->down) {
		if(l == Z) {
			diag(n, "constructor list too short");
			return 1;
		}
		if(l->op == OLIST) {
			r = l->u0.s0.nleft;
			ar = &l->u0.s0.nleft;
			al = &l->u0.s0.nright;
			l = l->u0.s0.nright;
		} else {
			r = l;
			ar = al;
			l = Z;
		}
		if(tcom(r))
			e++;
		typeext(t, r);
		if(tcompat(n, t, r->type, tasign))
			e++;
		constas(n, t, r->type);
		if(!e && !sametype(t, r->type)) {
			r = new1(OCAST, r, Z);
			r->type = t;
			*ar = r;
		}
	}
	if(l != Z) {
		diag(n, "constructor list too long");
		return 1;
	}
	return e;
}

int
tlvalue(Node *n)
{

	if(!n->addable) {
		diag(n, "not an l-value");
		return 1;
	}
	return 0;
}

/*
 *	general rewrite
 *	(IND(ADDR x)) ==> x
 *	(ADDR(IND x)) ==> x
 *	remove some zero operands
 *	remove no op casts
 *	evaluate constants
 */
void
ccom(Node *n)
{
	Node *l, *r;
	int t;

loop:
	if(n == Z)
		return;
	l = n->u0.s0.nleft;
	r = n->u0.s0.nright;
	switch(n->op) {

	case OAS:
	case OASXOR:
	case OASAND:
	case OASOR:
	case OASMOD:
	case OASLMOD:
	case OASLSHR:
	case OASASHR:
	case OASASHL:
	case OASDIV:
	case OASLDIV:
	case OASMUL:
	case OASLMUL:
	case OASSUB:
	case OASADD:
		ccom(l);
		ccom(r);
		if(n->op == OASLSHR || n->op == OASASHR || n->op == OASASHL)
		if(r->op == OCONST) {
			t = n->type->width * 8;	/* bits per byte */
			if(r->u0.nvconst >= t || r->u0.nvconst < 0)
				warn(n, "stupid shift: %ld", r->u0.nvconst);
		}
		break;

	case OCAST:
		ccom(l);
		if(l->op == OCONST) {
			evconst(n);
			if(n->op == OCONST)
				break;
		}
		if(nocast(l->type, n->type)) {
			l->type = n->type;
			*n = *l;
		}
		break;

	case OCOND:
		ccom(l);
		ccom(r);
		if(l->op == OCONST)
			if(vconst(l) == 0)
				*n = *r->u0.s0.nright;
			else
				*n = *r->u0.s0.nleft;
		break;

	case OREGISTER:
	case OINDREG:
	case OCONST:
	case ONAME:
		break;

	case OADDR:
		ccom(l);
		l->etype = TVOID;
		if(l->op == OIND) {
			l->u0.s0.nleft->type = n->type;
			*n = *l->u0.s0.nleft;
			break;
		}
		goto common;

	case OIND:
		ccom(l);
		if(l->op == OADDR) {
			l->u0.s0.nleft->type = n->type;
			*n = *l->u0.s0.nleft;
			break;
		}
		goto common;

	case OEQ:
	case ONE:

	case OLE:
	case OGE:
	case OLT:
	case OGT:

	case OLS:
	case OHS:
	case OLO:
	case OHI:
		ccom(l);
		ccom(r);
		relcon(l, r);
		relcon(r, l);
		goto common;

	case OASHR:
	case OASHL:
	case OLSHR:
		ccom(l);
		if(vconst(l) == 0 && !side(r)) {
			*n = *l;
			break;
		}
		ccom(r);
		if(vconst(r) == 0) {
			*n = *l;
			break;
		}
		if(r->op == OCONST) {
			t = n->type->width * 8;	/* bits per byte */
			if(r->u0.nvconst >= t || r->u0.nvconst <= -t)
				warn(n, "stupid shift: %ld", r->u0.nvconst);
		}
		goto common;

	case OMUL:
	case OLMUL:
		ccom(l);
		t = vconst(l);
		if(t == 0 && !side(r)) {
			*n = *l;
			break;
		}
		if(t == 1) {
			*n = *r;
			goto loop;
		}
		ccom(r);
		t = vconst(r);
		if(t == 0 && !side(l)) {
			*n = *r;
			break;
		}
		if(t == 1) {
			*n = *l;
			break;
		}
		goto common;

	case ODIV:
	case OLDIV:
		ccom(l);
		if(vconst(l) == 0 && !side(r)) {
			*n = *l;
			break;
		}
		ccom(r);
		t = vconst(r);
		if(t == 0) {
			diag(n, "divide check");
			*n = *r;
			break;
		}
		if(t == 1) {
			*n = *l;
			break;
		}
		goto common;

	case OSUB:
		ccom(r);
		if(r->op == OCONST) {
			if(typefd[r->type->etype]) {
				n->op = OADD;
				r->u0.nfconst = -r->u0.nfconst;
				goto loop;
			} else {
				n->op = OADD;
				r->u0.nvconst = -r->u0.nvconst;
				goto loop;
			}
		}
		ccom(l);
		goto common;

	case OXOR:
	case OOR:
	case OADD:
		ccom(l);
		if(vconst(l) == 0) {
			*n = *r;
			goto loop;
		}
		ccom(r);
		if(vconst(r) == 0) {
			*n = *l;
			break;
		}
		goto commun;

	case OAND:
		ccom(l);
		ccom(r);
		if(vconst(l) == 0 && !side(r)) {
			*n = *l;
			break;
		}
		if(vconst(r) == 0 && !side(l)) {
			*n = *r;
			break;
		}

	commun:
		/* look for commutative constant */
		if(r->op == OCONST) {
			if(l->op == n->op) {
				if(l->u0.s0.nleft->op == OCONST) {
					n->u0.s0.nright = l->u0.s0.nright;
					l->u0.s0.nright = r;
					goto loop;
				}
				if(l->u0.s0.nright->op == OCONST) {
					n->u0.s0.nright = l->u0.s0.nleft;
					l->u0.s0.nleft = r;
					goto loop;
				}
			}
		}
		if(l->op == OCONST) {
			if(r->op == n->op) {
				if(r->u0.s0.nleft->op == OCONST) {
					n->u0.s0.nleft = r->u0.s0.nright;
					r->u0.s0.nright = l;
					goto loop;
				}
				if(r->u0.s0.nright->op == OCONST) {
					n->u0.s0.nleft = r->u0.s0.nleft;
					r->u0.s0.nleft = l;
					goto loop;
				}
			}
		}
		goto common;

	case OANDAND:
		ccom(l);
		if(vconst(l) == 0) {
			*n = *l;
			break;
		}
		ccom(r);
		goto common;

	case OOROR:
		ccom(l);
		if(l->op == OCONST && l->u0.nvconst != 0) {
			*n = *l;
			n->u0.nvconst = 1;
			break;
		}
		ccom(r);
		goto common;

	default:
		if(l != Z)
			ccom(l);
		if(r != Z)
			ccom(r);
	common:
		if(l != Z)
		if(l->op != OCONST)
			break;
		if(r != Z)
		if(r->op != OCONST)
			break;
		evconst(n);
	}
}
