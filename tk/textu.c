#include <lib9.h>
#include <kernel.h>
#include "image.h"
#include "tk.h"

#define istring u.string
#define iwin u.win
#define imark u.mark
#define iline u.line

/* debugging */
int tktdbg;
extern void tktprinttext(TkText*);
extern void tktprintindex(TkTindex*);
extern void tktprintitem(TkTitem*);
extern void tktprintline(TkTline*);
extern void tktcheck(TkText*, char*);

char*
tktnewitem(int kind, int tagextra,TkTitem **ret)
{
	int n;
	TkTitem *i;

	n = sizeof(TkTitem) + tagextra * sizeof(ulong);
	i = malloc(n);
	if(i == nil)
		return TkNomem;

	memset(i, 0, n);
	i->kind = kind;
	i->tagextra = tagextra;
	*ret = i;
	return nil;
}

char*
tktnewline(int flags, TkTitem *items, TkTline *prev, TkTline *next, TkTline **ret)
{
	TkTline *l;
	TkTitem *i;

	l = malloc(sizeof(TkTline));
	if(l == nil)
		return TkNomem;

	memset(l, 0, sizeof(TkTline));
	l->flags = flags;
	l->items = items;
	l->prev = prev;
	l->next = next;
	next->prev = l;
	prev->next = l;

	for(i = items; i->next != nil;)
		i = i->next;
	if(tktdbg && !(i->kind == TkTnewline || i->kind == TkTcontline))
		print("text:tktnewline botch\n");
	i->iline = l;

	*ret = l;
	return nil;
}

void
tktfreeitems(TkText *tkt, TkTitem *i)
{
	TkTitem *n;
	Tk *tk;

	while(i != nil) {
		n = i->next;
		if(tkt->mouse == i)
			tkt->mouse = nil;
		if(tkt->focus == i)
			tkt->focus = nil;
		if(tkt->mouse == i)
			tkt->mouse = nil;
		switch(i->kind) {
		case TkTascii:
		case TkTrune:
			if(i->istring != nil)
				free(i->istring);
			break;
		case TkTwin:
			if(i->iwin != nil) {
				tk = i->iwin->sub;
				if(tk != nil) {
					tk->geom = nil;
					tk->destroyed = nil;
					tkdestroy(tk->env->top, tk->name->name, nil);
				}
				if(i->iwin->create != nil)
					free(i->iwin->create);
				free(i->iwin);
			}
			break;
		case TkTmark:
			break;
		}
		free(i);
		i = n;
	}
}

void
tktfreelines(TkText *tkt, TkTline *l)
{
	TkTline *n;

	while(l != nil) {
		n = l->next;
		tktfreeitems(tkt, l->items);
		free(l);
		l = n;
	}
}

void
tktfreetabs(TkTtabstop *t)
{
	TkTtabstop *n;

	while(t != nil) {
		n = t->next;
		free(t);
		t = n;
	}
}

void
tkfreetext(Tk *tk)
{
	TkText *tkt = TKobj(TkText, tk);

	if(tkt->start.next != nil && tkt->start.next != &(tkt->end)) {
		tkt->end.prev->next = nil;
		tktfreelines(tkt, tkt->start.next);
	}
	tktfreeitems(tkt, tkt->start.items);
	tktfreeitems(tkt, tkt->end.items);
	tktfreetabs(tkt->tabs);
	if(tkt->tagshare == nil)
		tktfreetags(tkt->tags);
	else
		tk->binds = nil;
	tktfreemarks(tkt->marks);
	if(tkt->xscroll != nil)
		free(tkt->xscroll);
	if(tkt->yscroll != nil)
		free(tkt->yscroll);
	/* don't free image because it belongs to window */
}

/*
 * Remove the item at ix, joining previous and next items.
 * If item is at end of line, remove next line and join
 * its items to this one (except at end).
 * On return, ix is adjusted to point to the next item.
 */
void
tktremitem(TkText *tkt, TkTindex *ix)
{
	TkTline *l, *lnext;
	TkTindex prev, nx;
	TkTitem *i, *ilast;

	l = ix->line;
	i = ix->item;

	if(i->next == nil) {
		if(tktdbg && !(i->kind == TkTnewline || i->kind == TkTcontline)) {
			print("tktremitem: botch 1\n");
			return;
		}
		lnext = l->next;
		if(lnext == &tkt->end)
			/* not supposed to remove final newline */
			return;
		if(i->kind == TkTnewline)
			tkt->nlines--;
		ilast = tktlastitem(lnext->items);
		ilast->iline = l;
		i->next = lnext->items;
		l->flags = (l->flags & ~TkTlast) | (lnext->flags & TkTlast);
		l->next = lnext->next;
		lnext->next->prev = l;
		free(lnext);
	}
	if(l->items == i)
		l->items = i->next;
	else {
		prev = *ix;
		if(!tktadjustind(tkt, TkTbyitemback, &prev) && tktdbg) {
			print("tktremitem: botch 2\n");
			return;
		}
		prev.item->next = i->next;
	}
	ix->item = i->next;
	ix->pos = 0;
	i->next = nil;
	nx = *ix;
	tktadjustind(tkt, TkTbycharstart, &nx);

	/* check against cached items */
	if(tkt->selfirst == i)
		tkt->selfirst = nx.item;
	if(tkt->sellast == i)
		tkt->sellast = nx.item;
	if(tkt->selfirst == tkt->sellast) {
		tkt->selfirst = nil;
		tkt->sellast = nil;
	}

	tktfreeitems(tkt, i);
}

int
tktdispwidth(Tk *tk, TkTitem *i, Font *f, int x, int pos, int nchars)
{
	int w, del, locked;
	TkTtabstop *tb, *tbprev;
	Display *d;
	TkText *tkt;
	TkEnv env;
 	int opts[TkTnumopts];

	tkt = TKobj(TkText, tk);
	d = tk->env->top->screen->display;

	switch(i->kind) {
	case TkTascii:
	case TkTrune:
		if(f == nil) {
			if(!tktanytags(i))
				f = tk->env->font;
			else {
				tkttagopts(tk, i, opts, &env, 1);
				f = env.font;
			}
		}
		locked = 0;
		if(!(tkt->tflag&TkTdlocked))
			locked = lockdisplay(d, 0);
		if(nchars >= 0)
			w = stringnwidth(f, i->istring+pos, nchars);
		else
			w = stringwidth(f, i->istring+pos);
		if(locked)
			unlockdisplay(d);
		break;
	case TkTtab:
		tb = tkt->tabs;
		if(tb == nil)
			w = 0;
		else {
			tbprev = nil;
			while(tb->pos <= x && tb->next != nil) {
					tbprev = tb;
					tb = tb->next;
				}
			w = tb->pos - x;
			if(w <= 0) {
				del = tb->pos;
				if(tbprev != nil)
					del -= tbprev->pos;
				while(w <= 0)
					w += del;
			}
			/* todo: other kinds of justification */
		}
		break;
	case TkTwin:
		if(i->iwin->sub == 0)
			w = 0;
		else
			w = i->iwin->sub->act.width + 2*i->iwin->padx + 2*i->iwin->sub->borderwidth;
		break;
	default:
		w = 0;
	}
	return w;
}

int
tktiswordchar(int c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c >= 0xA0;
}

int
tktindrune(TkTindex *ix)
{
	int ans;
	Rune r;

	switch(ix->item->kind) {
		case TkTascii:
			ans = ix->item->istring[ix->pos];
			break;
		case TkTrune:
			chartorune(&r, ix->item->istring);
			ans = r;
			break;
		case TkTtab:
			ans = '\t';
			break;
		case TkTnewline:
			ans = '\n';
			break;
		default:
			ans = 0;
	}
	return ans;
}

TkTitem*
tktlastitem(TkTitem *i)
{
	while(i->next != nil)
		i = i->next;
	if(tktdbg && !(i->kind == TkTnewline || i->kind == TkTcontline))
		print("text:tktlastitem botch\n");

	return i;
}

TkTline*
tktitemline(TkTitem *i)
{
	i = tktlastitem(i);
	return i->iline;
}

int
tktlinenum(TkText *tkt, TkTindex *p)
{
	int n;
	TkTline *l;

	if(p->line->orig.y <= tkt->end.orig.y / 2) {
		/* line seems closer to beginning */
		n = 1;
		for(l = tkt->start.next; l != p->line; l = l->next) {
			if(tktdbg && l->next == nil) {
				print("text: tktlinenum botch\n");
				break;
			}
			if(l->flags & TkTlast)
				n++;
		}
	}
	else {
		n = tkt->nlines;
		for(l = tkt->end.prev; l != p->line; l = l->prev) {
			if(tktdbg && l->prev == nil) {
				print("text: tktlinenum botch\n");
				break;
			}
			if(l->flags & TkTfirst)
				n--;
		}
	}
	return n;
}

int
tktlinepos(TkText *tkt, TkTindex *p)
{
	int n;
	TkTindex ix;
	TkTitem *i;

	n = 0;
	ix = *p;
	i = ix.item;
	tktadjustind(tkt, TkTbylinestart, &ix);
	while(ix.item != i) {
		if(tktdbg && ix.item->next == nil && (ix.line->flags&TkTlast)) {
			print("text: tktlinepos botch\n");
			break;
		}
		n += tktposcount(ix.item);
		if(!tktadjustind(tkt, TkTbyitem, &ix)) {
			if(tktdbg)
				print("tktlinepos botch\n");
			break;
		}
	}
	return (n+p->pos);
}

int
tktposcount(TkTitem *i)
{
	int n;

	if(i->kind == TkTascii)
		n = strlen(i->istring);
	else
	if(i->kind == TkTmark || i->kind == TkTcontline)
		n = 0;
	else
		n = 1;
	return n;
}

/*
 * Insert item i before position ins.
 * If i is a newline or a contline, make a new line to contain the items up to
 * and including the new newline, and make the original line
 * contain the items from ins on.
 * Adjust ins so that it points just after inserted item.
 */
char*
tktiteminsert(TkText *tkt, TkTindex *ins, TkTitem *i)
{
	int hasprev, flags;
	char *e;
	TkTindex prev;
	TkTline *l;
	TkTitem *items;

	prev = *ins;
	hasprev = tktadjustind(tkt, TkTbyitemback, &prev);

	if(i->kind == TkTnewline || i->kind == TkTcontline) {
		i->next = nil;
		if(hasprev && prev.line == ins->line) {
			items = ins->line->items;
			prev.item->next = i;
		}
		else
			items = i;

		flags = ins->line->flags&TkTfirst;
		if(i->kind == TkTnewline) {
			flags |= TkTlast;
			ins->line->flags |= TkTfirst;
		}
		e = tktnewline(flags, items, ins->line->prev, ins->line, &l);
		if(e != nil)
			return e;

		if(i->kind == TkTcontline)
			ins->line->flags &= ~TkTfirst;
		ins->line->items = ins->item;
		ins->pos = 0;
	}
	else {
		if(hasprev && prev.line == ins->line)
			prev.item->next = i;
		else
			ins->line->items = i;
		i->next = ins->item;
	}

	return nil;
}

/*
 * If index p doesn't point at the beginning of an item,
 * split the item at p.  Adjust p to point to the beginning of
 * the item after the split (same character it used to point at).
 * If there is a split, the old item gets the characters before
 * the split, and a new item gets the characters after it.
 */
char*
tktsplititem(TkTindex *p)
{
	int n;
	char *s1, *s2, *e;
	TkTitem *i, *i1;

	i = p->item;

	if(p->pos != 0) {
		/*
		 * Must be Tkascii kind
		 *
		 * Make new item i1, to be inserted after i,
		 * with portion of string from p->pos on
		 */

		n = strlen(i->istring) - p->pos;
		s1 = malloc(p->pos+1);
		s2 = malloc(n+1);
		if(s1 == nil || s2 == nil)
			return TkNomem;

		memmove(s1, i->istring, p->pos);
		s1[p->pos] = '\0';
		memmove(s2, i->istring + p->pos, n);
		s2[n] = '\0';

		e = tktnewitem(TkTascii, i->tagextra, &i1);
		if(e != nil) {
			free(s1);
			free(s2);
			return e;
		}

		free(i->istring);

		tkttagcomb(i1, i, 1);
		i1->next = i->next;
		i->next = i1;
		i->istring = s1;
		i1->istring = s2;

		p->item = i1;
		p->pos = 0;
	}

	return nil;
}

int
tktmaxwid(TkTline *l)
{
	int w, maxw;

	maxw = 0;
	while(l != nil) {
		w = l->width;
		if(w > maxw)
			maxw = w;
		l = l->next;
	}
	return maxw;
}

Rectangle
tktbbox(Tk *tk, TkTindex *ix)
{
	Rectangle r;
	int d, w;
	TkTitem *i;
	TkTline *l;
	TkEnv env;
	Tk *sub;
	TkText *tkt = TKobj(TkText, tk);
 	int opts[TkTnumopts];

	l = ix->line;

	/* r in V space */
	r.min = subpt(l->orig, tkt->deltatv);
	r.min.y += l->ascent;

	for(i = l->items; i != nil; i = i->next) {
		d = tktdispwidth(tk, i, nil, r.min.x, 0, -1);
		if(i == ix->item) {
			tkttagopts(tk, i, opts, &env, 1);
			r.min.y -= opts[TkToffset];
			if(i->kind == TkTascii) {
				w = tktdispwidth(tk, i, nil, r.min.x, ix->pos, 1);
				r.min.x += d - w;
				r.min.y -= env.font->ascent;
				r.max.x = r.min.x + w;
				r.max.y = r.min.y + env.font->height;
			}
			else {
				if(i->kind == TkTwin) {
					sub = i->iwin->sub;
					if(sub == nil)
						break;
					r.min.x += sub->act.x;
					r.min.y += sub->act.y;
					r.max.x = r.min.x + sub->act.width + 2*sub->borderwidth;
					r.max.y = r.min.y + sub->act.height + 2*sub->borderwidth;
				}
				else {
					r.max.x = r.min.x + d;
					r.max.y = r.min.y;
				}
			}
			return r;
		}
		else
			r.min.x += d;
	}
	r.min.x = 0;
	r.min.y = 0;
	r.max.x = 0;
	r.max.x = 0;
	return r;
}

/* Return left-at-baseline position of given item, in V coords */
static Point
tktitempos(Tk *tk, TkTindex *ix)
{
	Point p;
	TkTitem *i;
	TkTline *l;
	TkText *tkt = TKobj(TkText, tk);
 
	l = ix->line;

	/* p in V space */
	p = subpt(l->orig, tkt->deltatv);
	p.y += l->ascent;

	for(i = l->items; i != nil && i != ix->item; i = i->next)
		p.x += i->width;
	return p;
}

char *
tktxyparse(Tk* tk, char **parg, Point *p)
{
	char buf[Tkmaxitem];

	*parg = tkword(tk->env->top, *parg, buf, buf+sizeof(buf));
	if(*buf == '\0')
		return TkOparg;
	p->x = atoi(buf);

	*parg = tkword(tk->env->top, *parg, buf, buf+sizeof(buf));
	if(*buf == '\0')
		return TkOparg;
	p->y = atoi(buf);

	return nil;
}

static void
tktdeliver(Tk *tk, TkTitem *i, TkTitem *tagit, int event, void *data)
{
	Tk *ftk;
	TkTwind *w;
	TkText *tkt;
	TkTtaginfo *t;
	TkTline *l;
	TkMouse m;
	Point mp, p;
	TkTindex ix;

	if(i != nil) {
		tkt = TKobj(TkText, tk);

		if(i->kind == TkTwin) {
			w = i->iwin;
			if(w->sub != nil) {
				if(event & TkEmouse) {
					m = *(TkMouse*)data;
					mp.x = m.x;
					mp.y = m.y;
					ix.item = i;
					ix.pos = 0;
					ix.line = tktitemline(i);
					p = tktitempos(tk, &ix);
					mp.x = m.x - (tkt->deltasv.x + p.x);
					mp.y = m.y - (tkt->deltasv.y + p.y);
					ftk = tkinwindow(w->sub, mp);
					if(ftk != w->focus) {
						tkdeliver(w->focus, TkLeave, nil);
						tkdeliver(ftk, TkEnter, nil);
						w->focus = ftk;
					}
					if(ftk != nil)
						tkdeliver(ftk, event, &m);
				}
				else
					tkdeliver(w->sub, event, data);
				if(w->sub->flag & Tkdirty) {
					l = tktitemline(i);
					tktfixgeom(tk, l->prev, l);
				}
				if(event & TkKey)
					return;
			}
		}

		if(tagit != 0) {
			for(t = tkt->tags; t != nil; t = t->next) {
				if(t->binds != nil && tkttagset(tagit, t->id)) {
					if(tksubdeliver(tk, t->binds, event, data) == TkDbreak) {
						return;
					}
				}
			}
		}
	}

	tksubdeliver(tk, tk->binds, event, data);
}

void
tktextevent(Tk *tk, int event, void *data)
{
	char *e;
	TkMouse m, vm;
	TkTitem *f, *tagit;
	TkText *tkt;
	TkTindex ix;

	tkt = TKobj(TkText, tk);

	if(event == TkLeave && tkt->mouse != nil) {
		vm.x = 0;
		vm.y = 0;
		tktdeliver(tk, tkt->mouse, tkt->mouse, TkLeave, nil);
		tkt->mouse = nil;
		tkt->focus = nil;
		return;
	}

	if(event & TkEmouse) {
		/* m in S space, tm in V space */
		m = *(TkMouse*)data;
		vm = m;
		vm.x -= tkt->deltasv.x;
		vm.y -= tkt->deltasv.y;
		if((event & TkMotion) == 0 || m.b == 0) {
			tkt->current.x = vm.x;
			tkt->current.y = vm.y;
		}
		tktxyind(tk, vm.x, vm.y, &ix);
		f = ix.item;
		if(tkt->mouse != f) {
			tagit = nil;
			if(tkt->mouse != nil) {
				if(tktanytags(tkt->mouse)) {
					e = tktnewitem(0, tkt->mouse->tagextra, &tagit);
					if(e != nil)
						return;
					tkttagcomb(tagit, tkt->mouse, 1);
					tkttagcomb(tagit, f, -1);
				}
				tktdeliver(tk, tkt->mouse, tagit, TkLeave, nil);
				if(tagit)
					free(tagit);
				tagit = nil;
			}
			if(tktanytags(f)) {
				e = tktnewitem(0, f->tagextra, &tagit);
				if(e != nil)
					return;
				tkttagcomb(tagit, f, 1);
				if(tkt->mouse)
					tkttagcomb(tagit, tkt->mouse, -1);
			}
			tktdeliver(tk, f, tagit, TkEnter, nil);
			tkt->mouse = f;
			if(tagit)
				free(tagit);
		}
		if(m.b & 1) {
			if(f->kind == TkTwin && f->iwin->sub != nil) {
				tkt->focus = f;
				tktdeliver(tk, f, f, TkFocusin, nil);
			}
			else {
				if(tkt->focus != nil && tkt->focus->kind == TkTwin &&
						tkt->focus->iwin->sub != nil)
					tktdeliver(tk, tkt->focus, tkt->focus, TkFocusout, nil);
				tkt->focus = nil;
			}
		}
		if(tkt->mouse != nil)
			tktdeliver(tk, tkt->mouse, tkt->mouse, event, &m);
	}

	else if(event & TkKey)
		/* contrary to spec, don't deliver key events to "current" character */
		tktdeliver(tk, tkt->focus, tkt->focus, event, data);

	else
		tksubdeliver(tk, tk->binds, event, data);
}

/* Debugging */
void
tktprintitem(TkTitem *i)
{
	int j;

	print("%x:", i);
	switch(i->kind){
	case TkTascii:
		print("\"%s\"", i->istring);
		break;
	case TkTrune:
		print("<rune:%s>", i->istring);
		break;
	case TkTnewline:
		print("<nl:%x>", i->iline);
		break;
	case TkTcontline:
		print("<cont:%x>", i->iline);
		break;
	case TkTtab:
		print("<tab>");
		break;
	case TkTmark:
		print("<mk:%s>", i->imark->name);
		break;
	case TkTwin:
		print("<win:%s>", i->iwin->sub? i->iwin->sub->name->name : "<null>");
	}
	print("[%d]", i->width);
	if(i->tags !=0 || i->tagextra !=0) {
		print("{%ux", i->tags[0]);
		for(j=0; j < i->tagextra; j++)
			print(" %ux", i->tags[j+1]);
		print("}");
	}
	print(" ");
}

void
tktprintline(TkTline *l)
{
	TkTitem *i;

	print("line %x: orig=(%d,%d), w=%d, h=%d, a=%d, f=%x\n\t",
		l, l->orig.x, l->orig.y, l->width, l->height, l->ascent, l->flags);
	for(i = l->items; i != nil; i = i->next)
		tktprintitem(i);
	print("\n");
}

void
tktprintindex(TkTindex *ix)
{
	print("line=%x,item=%x,pos=%d\n", ix->line, ix->item, ix->pos);
}

void
tktprinttext(TkText *tkt)
{
	TkTline *l;
	TkTtaginfo *ti;
	TkTmarkinfo *mi;

	for(ti=tkt->tags; ti != nil; ti=ti->next)
		print("%s{%d} ", ti->name, ti->id);
	print("\n");
	for(mi = tkt->marks; mi != nil; mi=mi->next)
		print("%s{%x} ", mi->name? mi->name : "nil", mi->cur);
	print("\n");
	print("selfirst=%x sellast=%x\n", tkt->selfirst, tkt->sellast);

	for(l = &tkt->start; l != nil; l = l->next)
		tktprintline(l);
}

/*
 * Check that assumed invariants are true.
 *
 * - start line and end line have no items
 * - all other lines have at least one item
 * - start line leads to end line via next pointers
 * - prev pointers point to previous lines
 * - each line ends in either a TkTnewline or a TkTcontline
 *    whose iline pointer points to the line itself
 * - TkTcontline and TkTmark items have no tags
 *    (this is so they don't get realloc'd because of tag combination)
 * - all cur fields of marks point to nil or a TkTmark
 * - selfirst and sellast correctly define select region
 * - nlines counts the number of lines
 */
void
tktcheck(TkText *tkt, char *fun)
{
	int nl, insel, selfound;
	TkTline *l;
	TkTitem *i;
	TkTmarkinfo *mi;
	TkTindex ix;
	char *prob;

	prob = nil;
	nl = 0;

	if(tkt->start.items != nil || tkt->end.items != nil)
		prob = "start/end has items";
	for(l = tkt->start.next; l != &tkt->end; l = l->next) {
		if(l->prev->next != l) {
			prob = "prev mismatch";
			break;
		}
		if(l->next->prev != l) {
			prob = "next mismatch";
			break;
		}
		i = l->items;
		if(i == nil) {
			prob = "empty line";
			break;
		}
		while(i->next != nil) {
			if(i->kind == TkTnewline || i->kind == TkTcontline) {
				prob = "premature end of line";
				break;
			}
			if(i->kind == TkTmark && (i->tags[0] != 0 || i->tagextra != 0)) {
				prob = "mark has tags";
				break;
			}
			i = i->next;
		}
		if(i->kind == TkTnewline)
			nl++;
		if(!(i->kind == TkTnewline || i->kind == TkTcontline)) {
			prob = "bad end of line";
			break;
		}
		if(i->kind == TkTcontline && (i->tags[0] != 0 || i->tagextra != 0)) {
			prob = "contline has tags";
			break;
		}
		if(i->iline != l) {
			prob = "bad end-of-line pointer";
			break;
		}
	}
	for(mi = tkt->marks; mi != nil; mi=mi->next) {
		if(mi->cur != nil) {
			tktstartind(tkt, &ix);
			do {
				if(ix.item->kind == TkTmark && ix.item == mi->cur)
					goto foundmark;
			} while(tktadjustind(tkt, TkTbyitem, &ix));
			prob = "bad mark cur";
			break;
		    foundmark: ;
		}
	}
	insel = 0;
	selfound = 0;
	tktstartind(tkt, &ix);
	do {
		i = ix.item;
		if(i == tkt->selfirst) {
			if(i->kind == TkTmark || i->kind == TkTcontline) {
				prob = "selfirst not on character";
				break;
			}
			if(i == tkt->sellast) {
				prob = "selfirst==sellast, not nil";
				break;
			}
			insel = 1;
			selfound = 1;
		}
		if(i == tkt->sellast) {
			if(i->kind == TkTmark || i->kind == TkTcontline) {
				prob = "sellast not on character";
				break;
			}
			insel = 0;
		}
		if(i->kind != TkTmark && i->kind != TkTcontline) {
			if(i->tags[0] & (1<<TkTselid)) {
				if(!insel) {
					prob = "sel set outside selfirst..sellast";
					break;
				}
			}
			else {
				if(insel) {
					prob = "sel not set inside selfirst..sellast";
					break;
				}
			}
		}
	} while(tktadjustind(tkt, TkTbyitem, &ix));
	if(tkt->selfirst != nil && !selfound)
		prob = "selfirst not found";

	if(prob != nil) {
		print("tktcheck problem: %s: %s\n", fun, prob);
		tktprinttext(tkt);
abort();
	}
}

/*
struct timerec {
	char *name;
	ulong ms;
};

static struct timerec tt[100];
static int ntt = 1;

int
tktalloctime(char *name)
{
	if(ntt >= 100)
		abort();
	tt[ntt].name = strdup(name);
	tt[ntt].ms = 0;
	return ntt++;
}

void
tktstarttime(int ind)
{
return;
	tt[ind].ms -= osmillisec();
}

void
tktendtime(int ind)
{
return;
	tt[ind].ms += osmillisec();
}

void
tktdumptime(void)
{
	int i;

	for(i = 1; i < ntt; i++)
		print("%s: %d\n", tt[i].name, tt[i].ms);
}
*/
