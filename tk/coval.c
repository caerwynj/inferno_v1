#include "lib9.h"
#include "image.h"
#include "tk.h"

#define	O(t, e)		((long)(&((t*)0)->e))
typedef void	(*Drawfn)(Image*, Point, int, int, Image*, int);

/* Oval Options (+ means implemented)
	+fill
	+outline
	+stipple
	+tags
	+width
*/

typedef struct TkCoval TkCoval;
struct TkCoval
{
	int	width;
	Image*	stipple;
	Image*	pen;
};

static
TkOption ovalopts[] =
{
	"width",	OPTfrac,	O(TkCoval, width),	nil,
	"stipple",	OPTbmap,	O(TkCoval, stipple),	nil,
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
tkcvsovalsize(TkCitem *i)
{
	int w;
	TkCoval *o;

	o = TKobj(TkCoval, i);
	w = TKF2I(o->width)*2;

	i->p.bb = bbnil;
	tkpolybound(i->p.drawpt, i->p.npoint, &i->p.bb);
	i->p.bb = insetrect(i->p.bb, -w);
}

char*
tkcvsovalcreat(Tk* tk, char *arg, char **val)
{
	char *e;
	TkCoval *o;
	TkCitem *i;
	TkCanvas *c;
	TkOptab tko[3];

	c = TKobj(TkCanvas, tk);

	i = tkcnewitem(tk, TkCVoval, sizeof(TkCitem)+sizeof(TkCoval));
	if(i == nil)
		return TkNomem;

	o = TKobj(TkCoval, i);
	o->width = TKI2F(1);

	e = tkparsepts(tk->env->top, &i->p, &arg);
	if(e != nil) {
		tkcvsfreeitem(i);
		return e;
	}
	if(i->p.npoint != 2) {
		tkcvsfreeitem(i);
		return TkFewpt;
	}

	tko[0].ptr = o;
	tko[0].optab = ovalopts;
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

	tkcvsovalsize(i);
	tkmkpen(&o->pen, i->env, o->stipple);

	tkcvsappend(c, i);

	if(tk->master || tk->parent) {
		tkbbmax(&c->update, &i->p.bb);
		tk->flag |= Tkdirty;
	}
	return tkvalue(val, "%d", i->id);
}

char*
tkcvsovalcget(TkCitem *i, char *arg, char **val)
{
	TkOptab tko[3];
	TkCoval *o = TKobj(TkCoval, i);

	tko[0].ptr = o;
	tko[0].optab = ovalopts;
	tko[1].ptr = i;
	tko[1].optab = itemopts;
	tko[2].ptr = nil;

	return tkgencget(tko, arg, val);
}

char*
tkcvsovalconf(Tk *tk, TkCitem *i, char *arg)
{
	char *e;
	TkOptab tko[3];
	TkCoval *o = TKobj(TkCoval, i);

	tko[0].ptr = o;
	tko[0].optab = ovalopts;
	tko[1].ptr = i;
	tko[1].optab = itemopts;
	tko[2].ptr = nil;

	e = tkparse(tk->env->top, arg, tko, nil);
	tkcvsovalsize(i);
	tkmkpen(&o->pen, i->env, o->stipple);

	return e;
}

void
tkcvsovalfree(TkCitem *i)
{
	TkCoval *o;

	o = TKobj(TkCoval, i);
	if(o->stipple)
		freeimage(o->stipple);
	if(o->pen)
		freeimage(o->pen);
}

void
tkcvsovaldraw(Image *img, TkCitem *i)
{
	Point c;
	TkEnv *e;
	Image *pen;
	TkCoval *o;
	Rectangle d;
	int w, dx, dy;

	d.min = i->p.drawpt[0];
	d.max = i->p.drawpt[1];

	e = i->env;
	o = TKobj(TkCoval, i);

	pen = o->pen;
	if(pen == nil && (e->set & (1<<TkCfill)))
		pen = tkgc(e, TkCfill);

	w = TKF2I(o->width)/2;
	if(w < 0)
		return;

	d = canonrect(d);
	dx = Dx(d)/2;
	dy = Dy(d)/2;
	c.x = d.min.x + dx;
	c.y = d.min.y + dy;
	if(pen != nil)
		fillellipse(img, c, dx, dy, pen, c);

	ellipse(img, c, dx, dy, w, tkgc(e, TkCforegnd), c);
}

char*
tkcvsovalcoord(TkCitem *i, char *arg, int x, int y)
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
	tkcvsovalsize(i);
	return nil;
}
