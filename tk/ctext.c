#include <lib9.h>
#include <kernel.h>
#include "image.h"
#include "tk.h"

#define	O(t, e)		((long)(&((t*)0)->e))

/* Text Options (+ means implemented)
	+anchor
	+fill
	+font
	+justify
	+stipple
	+tags
	+text
	+width
*/

typedef struct TkCtext TkCtext;
struct TkCtext
{
	int	anchor;
	Point	anchorp;
	int	justify;
	int	icursor;
	int	focus;
	int	pixwidth;
	int	pixheight;
	int	sell;
	int	self;
	int	selfrom;
	int	sbw;
	int	width;
	int	nlines;
	Image*	stipple;
	Image*	pen;
	char*	text;
	int	tlen;
};

static
TkOption textopts[] =
{
	"anchor",	OPTstab,	O(TkCtext, anchor),	tkanchor,
	"justify",	OPTstab,	O(TkCtext, justify),	tktabjust,
	"width",	OPTdist,	O(TkCtext, width),	nil,
	"stipple",	OPTbmap,	O(TkCtext, stipple),	nil,
	"text",		OPTtext,	O(TkCtext, text),	nil,
	nil
};

static
TkOption itemopts[] =
{
	"tags",		OPTctag,	O(TkCitem, tags),	nil,
	"font",		OPTfont,	O(TkCitem, env),	nil,
	"fill",		OPTcolr,	O(TkCitem, env),	IAUX(TkCforegnd),
	nil
};

static char*
tkcvstextgetl(TkCtext *t, char *start, int *len)
{
	int w, n;
	char *lspc, *posn;

	w = t->width;
	if(w == 0)
		w = 1000000;

	n = 0;
	lspc = nil;
	posn = start;
	while(*posn && *posn != '\n') {
		if(*posn == ' ')
			lspc = posn;
		if(n >= w) {
			if(lspc != nil)
				posn = lspc;
			*len = posn - start;
			if(lspc != nil)
				posn++;
			return posn;
		}
		posn++;
		n++;
	}
	*len = posn - start;
	if(*posn == '\n')
		posn++;
	return posn;
}

void
tkcvstextsize(TkCitem *i)
{
	Point o;
	Font *font;
	TkCtext *t;
	Display *d;
	char *next, *p;
	int len, pixw, locked;

	t = TKobj(TkCtext, i);

	font = i->env->font;
	d = i->env->top->screen->display;
	t->pixwidth = 0;
	t->pixheight = 0;

	p = t->text;
	if(p != nil) {
		locked = lockdisplay(d, 0);
		while(*p) {
			next = tkcvstextgetl(t, p, &len);
			pixw = stringnwidth(font, p, len);
			if(pixw > t->pixwidth)
				t->pixwidth = pixw;
			t->pixheight += font->height;
			p = next;
		}
		if(locked)
			unlockdisplay(d);
	}

	o = tkcvsanchor(i->p.drawpt[0], t->pixwidth, t->pixheight, t->anchor);

	t->anchorp = o;

	i->p.bb.min.x = o.x;
	i->p.bb.min.y = o.y - Cvsicursor;
	i->p.bb.max.x = o.x + t->pixwidth;
	i->p.bb.max.y = o.y + t->pixheight + Cvsicursor;
	i->p.bb = insetrect(i->p.bb, -2*t->sbw);
}

char*
tkcvstextcreat(Tk* tk, char *arg, char **val)
{
	char *e;
	TkCtext *t;
	TkCitem *i;
	TkCanvas *c;
	TkOptab tko[3];

	c = TKobj(TkCanvas, tk);

	i = tkcnewitem(tk, TkCVtext, sizeof(TkCitem)+sizeof(TkCtext));
	if(i == nil)
		return TkNomem;

	t = TKobj(TkCtext, i);
	t->justify = Tkleft;
	t->anchor = Tkcenter;
	t->sell = -1;
	t->self = -1;
	t->icursor = -1;
	t->sbw = tk->sborderwidth;

	e = tkparsepts(tk->env->top, &i->p, &arg);
	if(e != nil) {
		tkcvsfreeitem(i);
		return e;
	}
	if(i->p.npoint != 1) {
		tkcvsfreeitem(i);
		return TkFewpt;
	}

	tko[0].ptr = t;
	tko[0].optab = textopts;
	tko[1].ptr = i;
	tko[1].optab = itemopts;
	tko[2].ptr = nil;
	e = tkparse(tk->env->top, arg, tko, nil);
	if(e != nil) {
		tkcvsfreeitem(i);
		return e;
	}
	
	e = tkcaddtag(tk, i, 1);
	if(e != nil) {
		tkcvsfreeitem(i);
		return e;
	}

	t->tlen = 0;
	if(t->text != nil)
		t->tlen = strlen(t->text);

	tkmkpen(&t->pen, i->env, t->stipple);
	tkcvstextsize(i);

	tkcvsappend(c, i);

	if(tk->master || tk->parent) {
		tkbbmax(&c->update, &i->p.bb);
		tk->flag |= Tkdirty;
	}
	return tkvalue(val, "%d", i->id);
}

char*
tkcvstextcget(TkCitem *i, char *arg, char **val)
{
	TkOptab tko[3];
	TkCtext *t = TKobj(TkCtext, i);

	tko[0].ptr = t;
	tko[0].optab = textopts;
	tko[1].ptr = i;
	tko[1].optab = itemopts;
	tko[2].ptr = nil;

	return tkgencget(tko, arg, val);
}

char*
tkcvstextconf(Tk *tk, TkCitem *i, char *arg)
{
	char *e;
	TkOptab tko[3];
	TkCtext *t = TKobj(TkCtext, i);

	tko[0].ptr = t;
	tko[0].optab = textopts;
	tko[1].ptr = i;
	tko[1].optab = itemopts;
	tko[2].ptr = nil;

	e = tkparse(tk->env->top, arg, tko, nil);

	t->tlen = 0;
	if(t->text != nil)
		t->tlen = strlen(t->text);

	tkmkpen(&t->pen, i->env, t->stipple);
	tkcvstextsize(i);

	return e;
}

void
tkcvstextfree(TkCitem *i)
{
	TkCtext *t;

	t = TKobj(TkCtext, i);
	if(t->stipple != nil)
		freeimage(t->stipple);
	if(t->pen != nil)
		freeimage(t->pen);
	if(t->text != nil)
		free(t->text);
}

void
tkcvstextdraw(Image *img, TkCitem *i)
{
	TkEnv *e;
	TkCtext *t;
	Point o, dp;
	Rectangle r;
	char *p, *next;
	Image *pen, *ones;
	int len, lw, end, start;

	t = TKobj(TkCtext, i);

	e = i->env;
	pen = t->pen;
	if(pen == nil)
		pen = tkgc(e, TkCforegnd);

	ones = pen->display->ones;

	o = t->anchorp;
	p = t->text;
	while(p && *p) {
		next = tkcvstextgetl(t, p, &len);
		dp = o;
		if(t->justify != Tkleft) {
			lw = stringnwidth(e->font, p, len);
			if(t->justify == Tkcenter)
				dp.x += (t->pixwidth - lw)/2;
			else
			if(t->justify == Tkright)
				dp.x += t->pixwidth - lw;
		}
		lw = p - t->text;
		if(t->self != -1 && lw+len > t->self) {
			start = t->self - lw;
			if(start < 0)
				r.min.x = o.x;
			else
				r.min.x = dp.x + stringnwidth(e->font, p, start);
			r.min.y = dp.y;
			end = t->sell - lw;
			if(end > len)
				r.max.x = o.x + t->pixwidth;
			else
				r.max.x = dp.x + stringnwidth(e->font, p, end);
			r.max.y = dp.y + e->font->height;
			tktextsdraw(img, r, e, ones, t->sbw);
		}
		stringn(img, dp, pen, dp, e->font, p, len);
		if(t->focus) {
			lw = p - t->text;
			if(t->icursor >= lw && t->icursor <= lw+len) {
				lw = t->icursor - lw;
				if(lw > 0)
					lw = stringnwidth(e->font, p, lw);
				r.min.x = dp.x + lw;
				r.min.y = dp.y - 1;
				r.max.x = r.min.x + 2;
				r.max.y = r.min.y + e->font->height + 1;
				draw(img, r, pen, ones, tkzp);
			}
		}
		o.y += e->font->height;
		p = next;
	}
}

char*
tkcvstextcoord(TkCitem *i, char *arg, int x, int y)
{
	char *e;
	TkCpoints p;

	if(arg == nil) {
		tkxlatepts(i->p.parampt, i->p.npoint, x, y);
		tkxlatepts(i->p.drawpt, i->p.npoint, TKF2I(x), TKF2I(y));
	}
	else {
		p.parampt = nil;
		p.drawpt = nil;
		e = tkparsepts(i->env->top, &p, &arg);
		if(e != nil) {
			tkfreepoint(&p);
			return e;
		}
		if(p.npoint != 1) {
			tkfreepoint(&p);
			return TkFewpt;
		}
		tkfreepoint(&i->p);
		i->p = p;
	}
	tkcvstextsize(i);
	return nil;
}

int
tkcvstextsrch(TkCitem *i, int x, int y)
{
	TkCtext *t;
	Font *font;
	Display *d;
	char *p, *next;
	int w, n, len, locked;

	t = TKobj(TkCtext, i);

	n = 0;
	font = i->env->font;
	d = i->env->top->screen->display;
	p = t->text;
	if(p == nil)
		return 0;
	while(*p) {
		next = tkcvstextgetl(t, p, &len);
		if(y <= font->height) {
			w = 0;
			n = 0;
			locked = lockdisplay(d, 0);
			while(n < len && x > w)
				w = stringnwidth(font, p, n++);
			if(locked)
				unlockdisplay(d);
			break;
		}
		y -= font->height;
		p = next;
	}	
	return p - t->text + n;
}

static char*
tkcvsparseindex(TkCitem *i, char *buf, int *index)
{
	char *p;
	int x, y;
	TkCtext *t;

	t = TKobj(TkCtext, i);

	if(strcmp(buf, "end") == 0) {
		*index = t->tlen;
		return nil;
	}
	if(strcmp(buf, "sel.first") == 0) {
		if(t->self < 0)
			return TkBadix;
		*index = t->self;
		return nil;
	}
	if(strcmp(buf, "sel.last") == 0) {
		if(t->sell < 0)
			return TkBadix;
		*index = t->sell;
		return nil;
	}
	if(strcmp(buf, "insert") == 0) {
		*index = t->icursor;
		return nil;
	}
	if(buf[0] == '@') {
		x = atoi(buf+1);
		p = strchr(buf, ',');
		if(p == nil)
			return TkBadix;
		y = atoi(p+1);
		*index = tkcvstextsrch(i, x-t->anchorp.x, y-t->anchorp.y);
		return nil;
	}

	if(buf[0] < '0' || buf[0] > '9')
		return TkBadix;
	x = atoi(buf);
	if(x < 0)
		x = 0;
	if(x > t->tlen)
		x = t->tlen;
	*index = x;	
	return nil;
}

char*
tkcvstextdchar(Tk *tk, TkCitem *i, char *arg)
{
	TkTop *top;
	TkCtext *t;
	int first, last;
	char *e, buf[Tkmaxitem];

	t = TKobj(TkCtext, i);

	top = tk->env->top;
	arg = tkword(top, arg, buf, buf+sizeof(buf));
	e = tkcvsparseindex(i, buf, &first);
	if(e != nil)
		return e;

	last = first+1;
	if(*arg != '\0') {
		tkword(top, arg, buf, buf+sizeof(buf));
		e = tkcvsparseindex(i, buf, &last);
		if(e != nil)
			return e;
	}
	if(last <= first || t->tlen == 0)
		return nil;

	tkbbmax(&TKobj(TkCanvas, tk)->update, &i->p.bb);

	memmove(t->text+first, t->text+last, t->tlen-last+1);
	t->tlen -= last-first;

	tkcvstextsize(i);
	tkbbmax(&TKobj(TkCanvas, tk)->update, &i->p.bb);

	tk->flag |= Tkdirty;
	return nil;
}

char*
tkcvstextinsert(Tk *tk, TkCitem *i, char *arg)
{
	TkTop *top;
	TkCtext *t;
	int first, n;
	char *e, *text, buf[Tkmaxitem];

	t = TKobj(TkCtext, i);

	top = tk->env->top;
	arg = tkword(top, arg, buf, buf+sizeof(buf));
	e = tkcvsparseindex(i, buf, &first);
	if(e != nil)
		return e;

	if(*arg == '\0')
		return nil;

	text = malloc(Tkcvstextins);
	if(text == nil)
		return TkNomem;

	tkword(top, arg, text, text+Tkcvstextins);
	n = strlen(text);
	t->text = realloc(t->text, t->tlen+n+1);
	if(t->text == nil) {
		free(text);
		return TkNomem;
	}
	if(t->tlen == 0)
		t->text[0] = '\0';

	tkbbmax(&TKobj(TkCanvas, tk)->update, &i->p.bb);

	memmove(t->text+first+n, t->text+first, t->tlen-first+1);
	memmove(t->text+first, text, n);
	t->tlen += n;
	free(text);

	tkcvstextsize(i);
	tkbbmax(&TKobj(TkCanvas, tk)->update, &i->p.bb);

	tk->flag |= Tkdirty;
	return nil;
}

char*
tkcvstextindex(Tk *tk, TkCitem *i, char *arg, char **val)
{
	int first;
	char *e, buf[Tkmaxitem];

	tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	e = tkcvsparseindex(i, buf, &first);
	if(e != nil)
		return e;

	return tkvalue(val, "%d", first);
}

char*
tkcvstexticursor(Tk *tk, TkCitem *i, char *arg)
{
	int first;
	TkCanvas *c;
	char *e, buf[Tkmaxitem];

	tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	e = tkcvsparseindex(i, buf, &first);
	if(e != nil)
		return e;

	TKobj(TkCtext, i)->icursor = first;

	c = TKobj(TkCanvas, tk);
	if(c->focus == i) {
		tkbbmax(&c->update, &i->p.bb);
		tk->flag |= Tkdirty;
	}
	return nil;
}

void
tkcvstextfocus(Tk *tk, TkCitem *i, int x)
{
	TkCtext *t;
	TkCanvas *c;

	if(i == nil)
		return;

	t = TKobj(TkCtext, i);
	c = TKobj(TkCanvas, tk);

	if(t->focus != x) {
		t->focus = x;
		tkbbmax(&c->update, &i->p.bb);
		tk->flag |= Tkdirty;
	}
}

void
tkcvstextclr(Tk *tk)
{
	TkCtext *t;
	TkCanvas *c;
	TkCitem *item;

	c = TKobj(TkCanvas, tk);
	item = c->selection;
	if(item == nil)
		return;

	c->selection = nil;
	t = TKobj(TkCtext, item);
	t->sell = -1;
	t->self = -1;
	tkbbmax(&c->update, &item->p.bb);
	tk->flag |= Tkdirty;
}

char*
tkcvstextselect(Tk *tk, TkCitem *i, char *arg, int op)
{
	int indx;
	TkCtext *t;
	TkCanvas *c;
	char *e, buf[Tkmaxitem];

	tkword(tk->env->top, arg, buf, buf+sizeof(buf));
	e = tkcvsparseindex(i, buf, &indx);
	if(e != nil)
		return e;

	c = TKobj(TkCanvas, tk);
	t = TKobj(TkCtext, i);
	switch(op) {
	case TkCselfrom:
		t->selfrom = indx;
		return nil;
	case TkCseladjust:
		if(c->selection == i) {
			if(abs(t->self-indx) < abs(t->sell-indx)) {
				t->self = indx;
				t->selfrom = t->sell;
			}
			else {
				t->sell = indx;
				t->selfrom = t->self;
			}
		}
		/* No break */
	case TkCselto:
		if(c->selection != i)
			tkcvstextclr(tk);
		c->selection = i;
		t->self = t->selfrom;
		t->sell = indx;
		break;
	}
	t->sbw = tk->sborderwidth;
	tkbbmax(&TKobj(TkCanvas, tk)->update, &i->p.bb);
	tk->flag |= Tkdirty;
	return nil;
}
