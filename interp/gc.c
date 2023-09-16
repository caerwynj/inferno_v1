#include "lib9.h"
#include "interp.h"
#include "pool.h"

enum
{
	Quanta		= 50		/* Blocks to sweep each time slice */
};

	int	nprop;
	int	gchalt;
	int	mflag;
	int	mutator = 0;
	int	gccolor = 3;
static	int	marker  = 1;
static	int	sweeper = 2;
static	Bhdr*	base;
static	Bhdr*	limit;
static	Bhdr*	ptr;
static	int	visit;
extern	Pool*	heapmem;

void
noptrs(Type *t, void *vw)
{
	USED(t);
	USED(vw);
}

void
markheap(Type *t, void *vw)
{
	Heap *h;
	uchar *p;
	int i, c, m;
	WORD **w, **q;

	if(t == nil || t->np == 0)
		return;

	w = (WORD**)vw;
	p = t->map;
	for(i = 0; i < t->np; i++) {
		c = *p++;
		if(c != 0) {
			q = w;
			for(m = 0x80; m != 0; m >>= 1) {
				if((c & m) && *q != H) {
					h = D2H(*q);
					Setmark(h);
				}
				q++;
			}
		}
		w += 8;
	}
}

/*
 * This routine MUST be modified to be incremental, but how?
 */
void
markarray(Type *t, void *vw)
{
	int i;
	Heap *h;
	uchar *v;
	Array *a;

	USED(t);

	a = vw;
	t = a->t;
	if(a->root != H) {
		h = D2H(a->root);
		Setmark(h);
	}

	if(t->np == 0)
		return;

	v = a->data;
	for(i = 0; i < a->len; i++) {
		markheap(t, v);
		v += t->size;
	}
}

void
markmodl(Type *t, void *vw)
{
	Heap *h;
	uchar *mp;
	Modlink *ml;

	USED(t);
	ml = vw;
	mp = ml->m->mp;
	if(mp != H) {
		h = D2H(mp);
		Setmark(h);
	}
}

void
marklist(Type *t, void *vw)
{
	List *l;
	Heap *h;

	USED(t);
	l = vw;
	markheap(l->t, l->data);
	while(visit > 0) {
		l = l->tail;
		if(l == H)
			return;
		h = D2H(l);
		Setmark(h);
		markheap(l->t, l->data);
		visit--;
	}
	l = l->tail;
	if(l != H) {
		D2H(l)->color = propagator;
		nprop = 1;
	}
}

static void
rootset(Prog *root)
{
	Heap *h;
	Type *t;
	Frame *f;
	Module *m;
	Stkext *sx;
	uchar *fp, *sp, *ex, *mp;

	mutator = gccolor % 3;
	marker = (gccolor-1)%3;
	sweeper = (gccolor-2)%3;

	while(root != nil) {
		mp = root->R.M->mp;
		if(mp != H) {
			h = D2H(mp);
			Setmark(h);
		}
		sp = root->R.SP;
		ex = root->R.EX;
		while(ex != nil) {
			sx = (Stkext*)ex;
			fp = sx->reg.tos.fu;
			while(fp != sp) {
				f = (Frame*)fp;
				t = f->t;
				if(t == nil)
					t = sx->reg.TR;
				fp += t->size;
				t->mark(t, f);
				m = f->mr;
				if(m != nil && m->mp != H) {
					h = D2H(m->mp);
					Setmark(h);
				}
			}
			ex = sx->reg.EX;
			sp = sx->reg.SP;
		}
		root = root->next;
	}
	for(m = modules; m != nil; m = m->link) {
		if(m->mp != H) {
			h = D2H(m->mp);
			Setmark(h);
		}
	}
}

static int
okbhdr(Bhdr *b)
{
	if(b == nil)
		return 0;
	switch(b->magic) {
	case MAGIC_A:
	case MAGIC_F:
	case MAGIC_E:
	case MAGIC_I:
		return 1;
	}
	return 0;
}

void
rungc(Prog *p)
{
	Type *t;
	Heap *h;
	Bhdr *b, *prev;

	if(gchalt)
		return;
	if(base == nil) {
		b = poolchain(heapmem);
		base = b;
		ptr = b;
		limit = B2LIMIT(b);
	}

	/* Chain broken ? */
	if(!okbhdr(ptr)) {
		base = nil;
		return;
	}

	prev = nil;
	for(visit = Quanta; visit > 0; visit--) {
		if(ptr->magic == MAGIC_A) {
			h = B2D(ptr);
			t = h->t;
			if(h->color == propagator) {
				h->color = mutator;
				if(t != nil)
					t->mark(h->t, H2D(void*, h));	
			}
			else
			if(h->color == sweeper) {
				if(mflag) {
					print("sweep h=0x%lux t=0x%lux\n", h, h->t);
					if(mflag > 1)
						abort();
				}
				if(t != nil) {
					if(t->nogc == 0) {
						gclock();
						initmem(t, H2D(void*, h));
						t->free(h);
						gcunlock();
					}
					freetype(t);
				}
				poolfree(heapmem, h);
				ptr = prev;
				if(!okbhdr(ptr)) {
					base = nil;
					return;
				}
				continue;
			}
		}
		prev = ptr;
		ptr = B2NB(ptr);
		if(ptr >= limit) {
			base = base->clink;
			if(base == nil)
				break;
			ptr = base;
			limit = B2LIMIT(base);
		}
	}

	if(base != nil)		/* Completed this iteration ? */
		return;
	if(nprop == 0)	{	/* Completed the epoch ? */
		gccolor++;
		rootset(p);
		return;
	}
	nprop = 0;
}
