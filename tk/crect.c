#include <lib9.h>
#include <kernel.h>
#include "image.h"
#include "tk.h"

#define	O(t, e)		((long)(&((t*)0)->e))

/* Rectangle Options (+ means implemented)
	+fill
	+outline
	+stipple
	+tags
	+width
*/

typedef struct TkCrect TkCrect;
struct TkCrect
{
	int	width;
	Image*	stipple;
	Image*	pen;
};

static
TkOption rectopts[] =
{
	"width",	OPTfrac,	O(TkCrect, width),	nil,
	"stipple",	OPTbmap,	O(TkCrect, stipple),	nil,
	nil
};

static
TkOption itemopts[] =
{
	"tags",		OPTctag,	O(TkCitem, tags),	nil,
	"fill",		OPTcolr,	O(TkCitem, env),	IAUX(TkCfill),
	"outline",	OPTcolr,	O(TkCitem, env),	IAUX(TkCforegnd),
	nil
};

void
tkcvsrectsize(TkCitem *i)
{
	TkCrect *r;
	int w, locked;
	Image *stipple;

	r = TKobj(TkCrect, i);
	w = TKF2I(r->width)*2;

	stipple = r->stipple;
	if(stipple != nil && stipple->repl == 0) {
		locked = lockdisplay(stipple->display, 0);
		replclipr(stipple, 1, huger);
		if(locked)
			unlockdisplay(stipple->display);
	}

	i->p.bb = bbnil;
	tkpolybound(i->p.drawpt, i->p.npoint, &i->p.bb);
	i->p.bb = insetrect(i->p.bb, -w);
}

char*
tkcvsrectcreat(Tk* tk, char *arg, char **val)
{
	char *e;
	TkCrect *r;
	TkCitem *i;
	TkCanvas *c;
	TkOptab tko[3];

	c = TKobj(TkCanvas, tk);

	i = tkcnewitem(tk, TkCVrect, sizeof(TkCitem)+sizeof(TkCrect));
	if(i == nil)
		return TkNomem;

	r = TKobj(TkCrect, i);
	r->width = TKI2F(1);

	e = tkparsepts(tk->env->top, &i->p, &arg);
	if(e != nil) {
		tkcvsfreeitem(i);
		return e;
	}
	if(i->p.npoint != 2) {
		tkcvsfreeitem(i);
		return TkFewpt;
	}

	tko[0].ptr = r;
	tko[0].optab = rectopts;
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

	tkcvsrectsize(i);
	tkcvsappend(c, i);

	if(tk->master || tk->parent) {
		tkbbmax(&c->update, &i->p.bb);
		tk->flag |= Tkdirty;
	}
	return tkvalue(val, "%d", i->id);
}

char*
tkcvsrectcget(TkCitem *i, char *arg, char **val)
{
	TkOptab tko[3];
	TkCrect *r = TKobj(TkCrect, i);

	tko[0].ptr = r;
	tko[0].optab = rectopts;
	tko[1].ptr = i;
	tko[1].optab = itemopts;
	tko[2].ptr = nil;

	return tkgencget(tko, arg, val);
}

char*
tkcvsrectconf(Tk *tk, TkCitem *i, char *arg)
{
	char *e;
	TkOptab tko[3];
	TkCrect *r = TKobj(TkCrect, i);

	tko[0].ptr = r;
	tko[0].optab = rectopts;
	tko[1].ptr = i;
	tko[1].optab = itemopts;
	tko[2].ptr = nil;

	e = tkparse(tk->env->top, arg, tko, nil);
	tkcvsrectsize(i);
	return e;
}

void
tkcvsrectfree(TkCitem *i)
{
	TkCrect *r;

	r = TKobj(TkCrect, i);
	if(r->stipple)
		freeimage(r->stipple);
	if(r->pen)
		freeimage(r->pen);
}

void
tkcvsrectdraw(Image *img, TkCitem *i)
{
	int lw, rw;
	TkEnv *e;
	TkCrect *r;
	Rectangle d, rr;
	Point tr, bl;
	Image *pen, *stipple, *ones;

	d.min = i->p.drawpt[0];
	d.max = i->p.drawpt[1];

	e = i->env;
	r = TKobj(TkCrect, i);

	pen = r->pen;
	if(pen == nil && (e->set & (1<<TkCfill)))
		pen = tkgc(e, TkCfill);

	stipple = r->stipple;
	if(stipple == nil)
		stipple = img->display->ones;

	if(pen != nil)
		draw(img, d, pen, stipple, tkzp);

	tr.x = d.max.x;
	tr.y = d.min.y;
	bl.x = d.min.x;
	bl.y = d.max.y;

	rw = (TKF2I(r->width) + 1)/2;
	if(rw <= 0)
		return;
	lw = (TKF2I(r->width))/2;

	pen = tkgc(e, TkCforegnd);
	if(pen != nil) {
		ones = pen->display->ones;
		/* horizontal lines first */
		rr.min.x = d.min.x - lw;
		rr.max.x = d.max.x + rw;
		rr.min.y = d.min.y - lw;
		rr.max.y = d.min.y + rw;
		draw(img, rr, pen, ones, rr.min);
		rr.min.y += Dy(d);
		rr.max.y += Dy(d);
		draw(img, rr, pen, ones, rr.min);
		/* now the vertical */
		/* horizontal lines first */
		rr.min.x = d.min.x - lw;
		rr.max.x = d.min.x + rw;
		rr.min.y = d.min.y + rw;
		rr.max.y = d.max.y - lw;
		draw(img, rr, pen, ones, rr.min);
		rr.min.x += Dx(d);
		rr.max.x += Dx(d);
		draw(img, rr, pen, ones, rr.min);
	}
}

char*
tkcvsrectcoord(TkCitem *i, char *arg, int x, int y)
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
		if(p.npoint != 2) {
			tkfreepoint(&p);
			return TkFewpt;
		}
		tkfreepoint(&i->p);
		i->p = p;
	}
	tkcvsrectsize(i);
	return nil;
}
