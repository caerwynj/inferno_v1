#include "lib9.h"
#include "image.h"
#include "tk.h"

#define istring u.string
#define iwin u.win
#define imark u.mark
#define iline u.line

static char* tktmarkgravity(Tk*, char*, char**);
static char* tktmarknames(Tk*, char*, char**);
static char* tktmarknext(Tk*, char*, char**);
static char* tktmarkprevious(Tk*, char*, char**);
static char* tktmarkset(Tk*, char*, char**);
static char* tktmarkunset(Tk*, char*, char**);

TkCmdtab
tktmarkcmd[] =
{
	"gravity",		tktmarkgravity,
	"names",		tktmarknames,
	"next",		tktmarknext,
	"previous",	tktmarkprevious,
	"set",		tktmarkset,
	"unset",		tktmarkunset,
	nil
};

char*
tktaddmarkinfo(TkText *tkt, char *name, TkTmarkinfo **ret)
{
	TkTmarkinfo *mi;

	mi = malloc(sizeof(TkTmarkinfo));
	if(mi == nil)
		return TkNomem;

	mi->name = strdup(name);
	mi->gravity = Tkright;
	mi->cur = nil;
	mi->next = tkt->marks;
	tkt->marks = mi;
	*ret = mi;
	return nil;
}

void
tktfreemarks(TkTmarkinfo *m)
{
	TkTmarkinfo *n;

	while(m != nil) {
		n = m->next;
		free(m->name);
		free(m);
		m = n;
	}
}

TkTmarkinfo *
tktfindmark(TkTmarkinfo *m, char *name)
{
	while(m != nil) {
		if(strcmp(m->name, name) == 0)
			return m;
		m = m->next;
	}
	return nil;
}

int
tktmarkind(Tk *tk, char *name, TkTindex *ans)
{
	TkTmarkinfo *mk;
	TkText *tkt = TKobj(TkText, tk);

	if(strcmp(name, "current") == 0) {
		tktxyind(tk, tkt->current.x, tkt->current.y, ans);
		return 1;
	}

	mk = tktfindmark(tkt->marks, name);
	if(mk == nil || mk->cur == nil)
		return 0;

	ans->item = mk->cur;
	ans->line = tktitemline(ans->item);
	ans->pos = 0;
	return 1;
}

char*
tktmarkparse(Tk *tk, char **parg, TkTmarkinfo **ret)
{
	char *e, buf[Tkmaxitem];
	TkText *tkt = TKobj(TkText, tk);

	*parg = tkword(tk->env->top, *parg, buf, buf+sizeof(buf));
	if(*buf == '\0')
		return TkOparg;

	*ret = tktfindmark(tkt->marks, buf);
	if(*ret == nil) {
		e = tktaddmarkinfo(tkt, buf, ret);
		if(e != nil)
			return e;
	}

	return nil;
}

/*
 * Insert mark before ixnew, first removing it from old place, if any.
 * Make sure ixnew continues to point after mark.
 */
char*
tktmarkmove(Tk *tk, TkTmarkinfo *m, TkTindex *ixnew)
{
	char *e;
	int deleted, split;
	TkTitem *i;
	TkTindex ix, pix;
	TkText *tkt = TKobj(TkText, tk);

	deleted = 0;
	if(m->cur != nil) {
		if(m->cur == ixnew->item)
			return nil;
		ix.item = m->cur;
		ix.line = tktitemline(m->cur);
		ix.pos = 0;
		tktremitem(tkt, &ix);
		deleted = 1;
	}

	e = tktnewitem(TkTmark, 0, &i);
	if(e != nil)
		return e;

	i->imark = m;
	m->cur = i;

	/* keep adjacent marks sorted: all rights, then all lefts */
	if(m->gravity == Tkright) {
		while(ixnew->item->kind == TkTmark && ixnew->item->imark->gravity == Tkleft)
			if(!tktadjustind(tkt, TkTbyitem, ixnew))
				break;
	}
	else {
		for(;;) {
			pix = *ixnew;
			if(!tktadjustind(tkt, TkTbyitemback, &pix))
				break;
			if(pix.item->kind == TkTmark && pix.item->imark->gravity == Tkright)
				*ixnew = pix;
			else
				break;
		}
	}

	split = (ixnew->pos > 0);
	e = tktsplititem(ixnew);
	if(e != nil)
		return e;

	e = tktiteminsert(tkt, ixnew, i);
	if(e != nil)
		return nil;

	if(strcmp(m->name, "insert") == 0 || split) {
		if(deleted && ix.line != ixnew->line) {
			tktfixgeom(tk, ix.line->prev, ix.line);
			ixnew->item = i;
			ixnew->line = tktitemline(i);
			ixnew->pos = 0;
		}
		tktfixgeom(tk, ixnew->line->prev, ixnew->line);
	}
	
	ixnew->item = i;
	ixnew->line = tktitemline(i);
	ixnew->pos = 0;
	return nil;
}

/* Text Mark Commands (+ means implemented)
	+gravity
	+names
	+next
	+previous
	+set
	+unset
*/

static char*
tktmarkgravity(Tk *tk, char *arg, char **val)
{
	char *e;
	TkTmarkinfo *m;
	char buf[Tkmaxitem];

	e = tktmarkparse(tk, &arg, &m);
	if(e != nil)
		return e;

	if(*arg == '\0')
		return tkvalue(val, (m->gravity & Tkleft)? "left" : "right");
	else {
		tkword(tk->env->top, arg, buf, buf+sizeof(buf));
		if(strcmp(buf, "left") == 0)
			m->gravity = Tkleft;
		else
		if(strcmp(buf, "right") == 0)
			m->gravity = Tkright;
		else
			return TkBadcm;
	}
	return nil;
}

static char*
tktmarknames(Tk *tk, char *arg, char **val)
{
	char *r, *fmt;
	TkTmarkinfo *m;
	TkText *tkt = TKobj(TkText, tk);

	USED(arg);

	fmt = "%s";
	for(m = tkt->marks; m != nil; m = m->next) {
		r = tkvalue(val, fmt, m->name);
		if(r != nil)
			return r;
		fmt = " %s";
	}
	return nil;
}

static char*
tktmarknext(Tk *tk, char *arg, char **val)
{
	char *e;
	TkTmarkinfo *mix;
	TkTindex ix, ixend;
	TkText *tkt = TKobj(TkText, tk);

	/* special behavior if specified index is a mark name */
	mix = tktfindmark(tkt->marks, arg);

	e = tktindparse(tk, &arg, &ix);
	if(e != nil)
		return e;

	if(mix != nil)
		tktadjustind(tkt, TkTbyitem, &ix);

	/* special behavior if index is 'end' */
	tktendind(tkt, &ixend);
	if(tktindcompare(tkt, &ix, TkEq, &ixend)) {
		do {
			tktadjustind(tkt, TkTbyitemback, &ix);
		} while(ix.item->kind == TkTmark);
	}

	do {
		if(ix.item->kind == TkTmark)
			return tkvalue(val, "%s", ix.item->imark->name);
		
	} while(tktadjustind(tkt, TkTbyitem, &ix));
	
	return nil;
}

static char*
tktmarkprevious(Tk *tk, char *arg, char **val)
{
	char *e;
	TkTindex ix;
	TkText *tkt = TKobj(TkText, tk);

	e = tktindparse(tk, &arg, &ix);
	if(e != nil)
		return e;

	while(tktadjustind(tkt, TkTbyitemback, &ix)) {
		if(ix.item->kind == TkTmark)
			return tkvalue(val, "%s", ix.item->imark->name);
	}
	
	return nil;
}

static char*
tktmarkset(Tk *tk, char *arg, char **val)
{
	char *e;
	TkTmarkinfo *m;
	TkTindex ixnew;

	USED(val);

	e = tktmarkparse(tk, &arg, &m);
	if(e != nil)
		return e;
	e = tktindparse(tk, &arg, &ixnew);
	if(e != nil)
		return e;

	return tktmarkmove(tk, m, &ixnew);
}

static char*
tktmarkunset(Tk *tk, char *arg, char **val)
{
	TkText *tkt;
	TkTmarkinfo *m, **p;
	TkTindex ix;
	char *e;

	USED(val);

	tkt = TKobj(TkText, tk);

	e = tktmarkparse(tk, &arg, &m);
	if(e != nil)
		return e;

	while(m != nil) {
		if(strcmp(m->name, "insert") == 0 || strcmp(m->name, "current") == 0)
			return TkBadvl;

		if(m->cur != nil) {
			ix.item = m->cur;
			ix.line = tktitemline(m->cur);
			ix.pos = 0;
			tktremitem(tkt, &ix);
			tktfixgeom(tk, ix.line->prev, ix.line);
		}

		for(p = &tkt->marks; *p != nil; p = &(*p)->next) {
			if(*p == m) {
				*p = m->next;
				break;
			}
		}
		m->next = nil;
		tktfreemarks(m);

		if(*arg != '\0') {
			e = tktmarkparse(tk, &arg, &m);
			if(e != nil)
				return e;
		}
		else
			m = nil;
	}
	return nil;
}

