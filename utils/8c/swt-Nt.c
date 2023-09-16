#include "gc.h"

int
swcmp(void *a1, void *a2)
{
	C1 *p1, *p2;

	p1 = a1;
	p2 = a2;
	if(p1->val < p2->val)
		return -1;
	return  p1->val > p2->val;
}

void
doswit(Node *n)
{
	Case *c;
	C1 *q, *iq;
	long def, nc, i;

	def = 0;
	nc = 0;
	for(c = cases; c->link != C; c = c->link) {
		if(c->def) {
			if(def)
				diag(n, "more than one default in switch");
			def = c->label;
			continue;
		}
		nc++;
	}

	i = nc*sizeof(C1);
	while(nhunk < i)
		gethunk();
	iq = (C1*)hunk;
	nhunk -= i;
	hunk += i;

	q = iq;
	for(c = cases; c->link != C; c = c->link) {
		if(c->def)
			continue;
		q->label = c->label;
		q->val = c->val;
		q++;
	}
	qsort(iq, nc, sizeof(C1), swcmp);
	if(debug['W'])
	for(i=0; i<nc; i++)
		print("case %2d: = %.8lux\n", i, iq[i].val);
	if(def == 0)
		def = breakpc;
	for(i=0; i<nc-1; i++)
		if(iq[i].val == iq[i+1].val)
			diag(n, "duplicate cases in switch %ld", iq[i].val);
	swit1(iq, nc, def, n);
}

void
swit1(C1 *q, int nc, long def, Node *n)
{
	C1 *r;
	int i;
	Prog *sp;

	if(nc < 5) {
		for(i=0; i<nc; i++) {
			if(debug['W'])
				print("case = %.8lux\n", q->val);
			gopcode(OEQ, n->type, n, nodconst(q->val));
			patch(p, q->label);
			q++;
		}
		gbranch(OGOTO);
		patch(p, def);
		return;
	}
	i = nc / 2;
	r = q+i;
	if(debug['W'])
		print("case > %.8lux\n", r->val);
	gopcode(OGT, n->type, n, nodconst(r->val));
	sp = p;
	gbranch(OGOTO);
	p->as = AJEQ;
	patch(p, r->label);
	swit1(q, i, def, n);

	if(debug['W'])
		print("case < %.8lux\n", r->val);
	patch(sp, pc);
	swit1(r+1, nc-i-1, def, n);
}

void
cas(void)
{
	Case *c;

	ALLOC(c, Case);
	c->link = cases;
	cases = c;
}

void
bitload(Node *b, Node *n1, Node *n2, Node *n3, Node *nn)
{
	int sh;
	long v;
	Node *l;

	/*
	 * n1 gets adjusted/masked value
	 * n2 gets address of cell
	 * n3 gets contents of cell
	 */
	l = b->u0.s0.nleft;
	if(n2 != Z) {
		regalloc(n1, l, nn);
		reglcgen(n2, l, Z);
		regalloc(n3, l, Z);
		gmove(n2, n3);
		gmove(n3, n1);
	} else {
		regalloc(n1, l, nn);
		cgen(l, n1);
	}
	if(b->type->shift == 0 && typeu[b->type->etype]) {
		v = ~0 + (1L << b->type->nbits);
		gopcode(OAND, types[TLONG], nodconst(v), n1);
	} else {
		sh = 32 - b->type->shift - b->type->nbits;
		if(sh > 0)
			gopcode(OASHL, types[TLONG], nodconst(sh), n1);
		sh += b->type->shift;
		if(sh > 0)
			if(typeu[b->type->etype])
				gopcode(OLSHR, types[TLONG], nodconst(sh), n1);
			else
				gopcode(OASHR, types[TLONG], nodconst(sh), n1);
	}
}

void
bitstore(Node *b, Node *n1, Node *n2, Node *n3, Node *nn)
{
	long v;
	Node nod;
	int sh;

	regalloc(&nod, b->u0.s0.nleft, Z);
	v = ~0 + (1L << b->type->nbits);
	gopcode(OAND, types[TLONG], nodconst(v), n1);
	gmove(n1, &nod);
	if(nn != Z)
		gmove(n1, nn);
	sh = b->type->shift;
	if(sh > 0)
		gopcode(OASHL, types[TLONG], nodconst(sh), &nod);
	v <<= sh;
	gopcode(OAND, types[TLONG], nodconst(~v), n3);
	gopcode(OOR, types[TLONG], n3, &nod);
	gmove(&nod, n2);

	regfree(&nod);
	regfree(n1);
	regfree(n2);
	regfree(n3);
}

long
outstring(char *s, long n)
{
	long r;

	r = nstring;
	while(n) {
		string[mnstring] = *s++;
		mnstring++;
		nstring++;
		if(mnstring >= NSNAME) {
			gpseudo(ADATA, symstring, nodconst(0L));
			p->from.u0.aoffset += nstring - NSNAME;
			p->from.scale = NSNAME;
			p->to.type = D_SCONST;
			memmove(p->to.u0.asval, string, NSNAME);
			mnstring = 0;
		}
		n--;
	}
	return r;
}

long
outlstring(ushort *s, long n)
{
	char buf[2];
	int c;
	long r;

	while(nstring & 1)
		outstring("", 1);
	r = nstring;
	while(n > 0) {
		c = *s++;
		if(endian(0)) {
			buf[0] = c>>8;
			buf[1] = c;
		} else {
			buf[0] = c;
			buf[1] = c>>8;
		}
		outstring(buf, 2);
		n -= sizeof(ushort);
	}
	return r;
}

int
vlog(Node *n)
{

	int s, i;
	ulong m, v;

	if(n->op != OCONST)
		goto bad;
	if(!typechlp[n->type->etype])
		goto bad;

	v = n->u0.nvconst;

	s = 0;
	m = MASK(64);
	for(i=32; i; i>>=1) {
		m >>= i;
		if(!(v & m)) {
			v >>= i;
			s += i;
		}
	}
	if(v == 1)
		return s;

bad:
	return -1;
}

void
nullwarn(Node *l, Node *r)
{
	warn(Z, "result of operation not used");
	if(l != Z)
		cgen(l, Z);
	if(r != Z)
		cgen(r, Z);
}

void
sextern(Sym *s, Node *a, long o, long w)
{
	long e, lw;

	for(e=0; e<w; e+=NSNAME) {
		lw = NSNAME;
		if(w-e < lw)
			lw = w-e;
		gpseudo(ADATA, s, nodconst(0L));
		p->from.u0.aoffset += o+e;
		p->from.scale = lw;
		p->to.type = D_SCONST;
		memmove(p->to.u0.asval, a->u0.ncstring+e, lw);
	}
}

void
gextern(Sym *s, Node *a, long o, long w)
{
	if(a->op == OCONST && typev[a->type->etype]) {
		gpseudo(ADATA, s, nodconst((long)(a->u0.nvconst>>32)));
		p->from.u0.aoffset += o;
		p->from.scale = 4;
		gpseudo(ADATA, s, nodconst((long)(a->u0.nvconst)));
		p->from.u0.aoffset += o + 4;
		p->from.scale = 4;
		return;
	}
	gpseudo(ADATA, s, a);
	p->from.u0.aoffset += o;
	p->from.scale = w;
	switch(p->to.type) {
	default:
		p->to.index = p->to.type;
		p->to.type = D_ADDR;
	case D_CONST:
	case D_FCONST:
	case D_ADDR:
		break;
	}
}

void	zname(Biobuf*, char*, int, int);
void	zaddr(Biobuf*, Adr*, int);
void	outhist(Biobuf*);

void
outcode(void)
{
	struct { Sym *sym; short type; } h[NSYM];
	Prog *p;
	Sym *s;
	int f, sf, st, t, sym;
	Biobuf b;

	if(debug['S']) {
		for(p = firstp; p != P; p = p->link)
			if(p->as != ADATA && p->as != AGLOBL)
				pc--;
		for(p = firstp; p != P; p = p->link) {
			print("%P\n", p);
			if(p->as != ADATA && p->as != AGLOBL)
				pc++;
		}
	}
	f = open(outfile, OWRITE);
	if(f < 0) {
		diag(Z, "cannot open %s", outfile);
		return;
	}
	Binit(&b, f, OWRITE);
	Bseek(&b, 0L, 2);
	outhist(&b);
	for(sym=0; sym<NSYM; sym++) {
		h[sym].sym = S;
		h[sym].type = 0;
	}
	sym = 1;
	for(p = firstp; p != P; p = p->link) {
	jackpot:
		sf = 0;
		s = p->from.sym;
		while(s != S) {
			sf = s->sym;
			if(sf < 0 || sf >= NSYM)
				sf = 0;
			t = p->from.type;
			if(t == D_ADDR)
				t = p->from.index;
			if(h[sf].type == t)
			if(h[sf].sym == s)
				break;
			zname(&b, s->name, t, sym);
			s->sym = sym;
			h[sym].sym = s;
			h[sym].type = t;
			sf = sym;
			sym++;
			if(sym >= NSYM)
				sym = 1;
			break;
		}
		st = 0;
		s = p->to.sym;
		while(s != S) {
			st = s->sym;
			if(st < 0 || st >= NSYM)
				st = 0;
			t = p->to.type;
			if(t == D_ADDR)
				t = p->to.index;
			if(h[st].type == t)
			if(h[st].sym == s)
				break;
			zname(&b, s->name, t, sym);
			s->sym = sym;
			h[sym].sym = s;
			h[sym].type = t;
			st = sym;
			sym++;
			if(sym >= NSYM)
				sym = 1;
			if(st == sf)
				goto jackpot;
			break;
		}
		Bputc(&b, p->as);
		Bputc(&b, p->as>>8);
		Bputc(&b, p->lineno);
		Bputc(&b, p->lineno>>8);
		Bputc(&b, p->lineno>>16);
		Bputc(&b, p->lineno>>24);
		zaddr(&b, &p->from, sf);
		zaddr(&b, &p->to, st);
	}
	Bflush(&b);
	close(f);
	firstp = P;
	lastp = P;
}



void
outhist(Biobuf *b)
{
	Hist *h;
	char *p, *q, *op;
	Prog pg;
	int n;

	pg = zprog;
	pg.as = AHISTORY;
	for(h = hist; h != H; h = h->link) {
		p = h->name;
		op = 0;
		/* pathname[0]='drive', pathname[1]=':' on a PC */
		if(p && p[0] != '\\' && h->offset == 0 && pathname && pathname[2] == '\\') {
			op = p;
			p = &pathname[2];
			*p = '/';	/* change leading '\' to '/'	*/
		}
		while(p) {
			q = utfrune(p, '\\');
			if(q) {
				n = q-p;
				q++;
			} else {
				n = strlen(p);
				q = 0;
			}
			if(n) {
				Bputc(b, ANAME);
				Bputc(b, ANAME>>8);
				Bputc(b, D_FILE);
				Bputc(b, 1);
				Bputc(b, '<');
				Bwrite(b, p, n);
				Bputc(b, 0);
			}
			p = q;
			if(p == 0 && op) {
				p = op;
				op = 0;
			}
		}
		pg.lineno = h->line;
		pg.to.type = zprog.to.type;
		pg.to.u0.aoffset = h->offset;
		if(h->offset)
			pg.to.type = D_CONST;

		Bputc(b, pg.as);
		Bputc(b, pg.as>>8);
		Bputc(b, pg.lineno);
		Bputc(b, pg.lineno>>8);
		Bputc(b, pg.lineno>>16);
		Bputc(b, pg.lineno>>24);
		zaddr(b, &pg.from, 0);
		zaddr(b, &pg.to, 0);
	}
}

void
zname(Biobuf *b, char *n, int t, int s)
{

	Bputc(b, ANAME);	/* as */
	Bputc(b, ANAME>>8);	/* as */
	Bputc(b, t);		/* type */
	Bputc(b, s);		/* sym */
	while(*n) {
		Bputc(b, *n);
		n++;
	}
	Bputc(b, 0);
}

void
zaddr(Biobuf *b, Adr *a, int s)
{
	long l;
	int i, t;
	char *n;
	Ieee e;

	t = 0;
	if(a->index != D_NONE || a->scale != 0)
		t |= T_INDEX;
	if(s != 0)
		t |= T_SYM;

	switch(a->type) {
	default:
		t |= T_TYPE;
	case D_NONE:
		if(a->u0.aoffset != 0)
			t |= T_OFFSET;
		break;
	case D_FCONST:
		t |= T_FCONST;
		break;
	case D_SCONST:
		t |= T_SCONST;
		break;
	}
	Bputc(b, t);

	if(t & T_INDEX) {	/* implies index, scale */
		Bputc(b, a->index);
		Bputc(b, a->scale);
	}
	if(t & T_OFFSET) {	/* implies offset */
		l = a->u0.aoffset;
		Bputc(b, l);
		Bputc(b, l>>8);
		Bputc(b, l>>16);
		Bputc(b, l>>24);
	}
	if(t & T_SYM)		/* implies sym */
		Bputc(b, s);
	if(t & T_FCONST) {
		ieeedtod(&e, a->u0.adval);
		l = e.l;
		Bputc(b, l);
		Bputc(b, l>>8);
		Bputc(b, l>>16);
		Bputc(b, l>>24);
		l = e.h;
		Bputc(b, l);
		Bputc(b, l>>8);
		Bputc(b, l>>16);
		Bputc(b, l>>24);
		return;
	}
	if(t & T_SCONST) {
		n = a->u0.asval;
		for(i=0; i<NSNAME; i++) {
			Bputc(b, *n);
			n++;
		}
		return;
	}
	if(t & T_TYPE)
		Bputc(b, a->type);
}

void
ieeedtod(Ieee *ieee, double native)
{
	double fr, ho, f;
	int exp;

	if(native < 0) {
		ieeedtod(ieee, -native);
		ieee->h |= 0x80000000L;
		return;
	}
	if(native == 0) {
		ieee->l = 0;
		ieee->h = 0;
		return;
	}
	fr = frexp(native, &exp);
	f = 2097152L;		/* shouldnt use fp constants here */
	fr = modf(fr*f, &ho);
	ieee->h = ho;
	ieee->h &= 0xfffffL;
	ieee->h |= (exp+1022L) << 20;
	f = 65536L;
	fr = modf(fr*f, &ho);
	ieee->l = ho;
	ieee->l <<= 16;
	ieee->l |= (long)(fr*f);
}

char*	xonames[] =
{
	"INDEX",
	"XEND",
};

char*
xOconv(int a)
{
	if(a <= OEND || a > OXEND)
		return "**badO**";
	return xonames[a-OEND-1];
}

int
endian(int ii)
{
	return 0;
}

int
passbypointer(int et)
{

	return typesuv[et];
}

int
argalign(long ll0, long ll1, int ii)
{
	return 0;
}
