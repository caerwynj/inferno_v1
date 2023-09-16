#include	"lib9.h"
#include	"dat.h"
#include	"fns.h"
#include	"error.h"
#include	"styx.h"

typedef	struct Fid	Fid;
typedef	struct Export	Export;
typedef	struct Exq	Exq;

enum
{
	Nfidhash	= 1,
	MAXRPC		= MAXMSG+MAXFDATA,
	MAXDIRREAD	= (MAXFDATA/DIRLEN)*DIRLEN
};

struct Export
{
	Ref	r;
	Exq*	work;
	Lock	fidlock;
	Fid*	fid[Nfidhash];
	Chan*	io;
	Chan*	root;
	Pgrp*	pgrp;
	int	async;
	int	npart;
	char	part[MAXRPC];
};

struct Fid
{
	Fid*	next;
	Fid**	last;
	Chan*	chan;
	ulong	offset;
	int	fid;
	int	ref;		/* fcalls using the fid; locked by Export.Lock */
	int	attached;	/* fid attached or cloned but not clunked */
};

struct Exq
{
	Exq*	next;
	int	shut;		/* has been noted for shutdown */
	Export*	export;
	Proc*	slave;
	Fcall	rpc;
	char	buf[MAXRPC];
};

struct
{
	Lock	l;
	QLock	qwait;
	Rendez	rwait;
	Exq	*head;		/* work waiting for a slave */
	Exq	*tail;
}exq;

void	exshutdown(Export*);
void	exflush(Export*, int);
void	exslave(void*);
void	exfree(Export*);
void	exportproc(void*);

char*	Exattach(Export*, Fcall*);
char*	Exclone(Export*, Fcall*);
char*	Exclunk(Export*, Fcall*);
char*	Excreate(Export*, Fcall*);
char*	Exnop(Export*, Fcall*);
char*	Exopen(Export*, Fcall*);
char*	Exread(Export*, Fcall*);
char*	Exremove(Export*, Fcall*);
char*	Exstat(Export*, Fcall*);
char*	Exwalk(Export*, Fcall*);
char*	Exwrite(Export*, Fcall*);
char*	Exwstat(Export*, Fcall*);

char	*(*fcalls[Tmax])(Export*, Fcall*);

char	Enofid[]   = "no such fid";
char	Eseekdir[] = "can't seek on a directory";
char	Ereaddir[] = "unaligned read of a directory";
int	exdebug = 0;

int
export(int fd, int async)
{
	Chan *c;
	Pgrp *pg;
	Export *fs;

	if(waserror())
		return -1;

	c = fdtochan(up->env->fgrp, fd, ORDWR, 1, 1);
	poperror();
	c->flag |= CMSG;

	fs = malloc(sizeof(Export));
	fs->r.ref = 1;

	pg = up->env->pgrp;
	fs->pgrp = pg;
	incref(&pg->r);
	fs->root = pg->slash;
	incref(&fs->root->r);
	fs->root = domount(fs->root);
	fs->io = c;
	fs->async = async;
	if(async)
		kproc("exportfs", exportproc, fs);
	else
		exportproc(fs);

	return 0;
}

static void
exportinit(void)
{
	lock(&exq.l);
	if(fcalls[Tnop] != nil) {
		unlock(&exq.l);
		return;
	}
	fcalls[Tnop] = Exnop;
	fcalls[Tattach] = Exattach;
	fcalls[Tclone] = Exclone;
	fcalls[Twalk] = Exwalk;
	fcalls[Topen] = Exopen;
	fcalls[Tcreate] = Excreate;
	fcalls[Tread] = Exread;
	fcalls[Twrite] = Exwrite;
	fcalls[Tclunk] = Exclunk;
	fcalls[Tremove] = Exremove;
	fcalls[Tstat] = Exstat;
	fcalls[Twstat] = Exwstat;
	unlock(&exq.l);
}

void
exportproc(void *a)
{
	Exq *q;
	int async;
	char *buf;
	int n, cn, len;
	Export *fs = a;

	exportinit();

	for(;;){
		q = malloc(sizeof(Exq));
		if(q == 0)
			panic("no memory");

		q->rpc.data = q->buf + MAXMSG;

		buf = q->buf;
		len = MAXRPC;
		if(fs->npart) {
			memmove(buf, fs->part, fs->npart);
			buf += fs->npart;
			len -= fs->npart;
			goto chk;
		}
		for(;;) {
			if(waserror())
				goto bad;

			n = (*devtab[fs->io->type].read)(fs->io, buf, len, 0);
			poperror();

			if(n <= 0)
				goto bad;

			buf += n;
			len -= n;
	chk:
			n = buf - q->buf;

			/* convM2S returns size of correctly decoded message */
			cn = convM2S(q->buf, &q->rpc, n);
			if(cn < 0){
				print("bad message type in exportproc\n");
				goto bad;
			}
			if(cn > 0) {
				n -= cn;
				if(n < 0){
					print("negative size in exportproc");
					goto bad;
				}
				fs->npart = n;
				if(n != 0)
					memmove(fs->part, q->buf+cn, n);
				break;
			}
		}
		if(exdebug)
			print("export <- %F\n", &q->rpc);

		if(q->rpc.type == Tflush){
			exflush(fs, q->rpc.oldtag);
			free(q);
			continue;
		}

		q->export = fs;
		incref(&fs->r);

		lock(&exq.l);
		if(exq.head == nil)
			exq.head = q;
		else
			exq.tail->next = q;
		q->next = nil;
		exq.tail = q;
		unlock(&exq.l);
		if(exq.qwait.head == nil) {
			n = kproc("exslave", exslave, nil);
			USED(n);
/*			print("launch export (pid=%d)\n", n);	/**/
		}
		Wakeup(&exq.rwait);
	}
bad:
	async = fs->async;

	free(q);
	exshutdown(fs);
	exfree(fs);

	if(async == 0)
		return;

	pexit("mount shut down", 0);
}

void
exflush(Export *fs, int tag)
{
	Exq *q, **last;
	int n;

	lock(&exq.l);
	last = &exq.head;
	for(q = exq.head; q != nil; q = q->next){
		if(q->export == fs && q->rpc.tag == tag){
			*last = q->next;
			unlock(&exq.l);

			q->rpc.type = Rerror;
			strncpy(q->rpc.ename, "interrupted", ERRLEN);
			n = convS2M(&q->rpc, q->buf);
			if(n < 0)
				panic("bad message type in exflush");
			if(!waserror()){
				(*devtab[fs->io->type].write)(fs->io, q->buf, n, 0);
				poperror();
			}
			exfree(fs);
			free(q);
			return;
		}
		last = &q->next;
	}
	unlock(&exq.l);

	lock(&fs->r.l);
	for(q = fs->work; q != nil; q = q->next){
		if(q->rpc.tag == tag){
			unlock(&fs->r.l);
			oshostintr(q->slave);
			return;
		}
	}
	unlock(&fs->r.l);
}

void
exshutdown(Export *fs)
{
	Exq *q, **last;

	lock(&exq.l);
	last = &exq.head;
	for(q = exq.head; q != nil; q = *last){
		if(q->export == fs){
			*last = q->next;
			exfree(fs);
			free(q);
			continue;
		}
		last = &q->next;
	}
	unlock(&exq.l);

	lock(&fs->r.l);
	q = fs->work;
	while(q != nil){
		if(q->shut){
			q = q->next;
			continue;
		}
		q->shut = 1;
		unlock(&fs->r.l);
		oshostintr(q->slave);
		lock(&fs->r.l);
		q = fs->work;
	}
	unlock(&fs->r.l);
}

void
exfree(Export *fs)
{
	Fid *f, *n;
	int i;

	if(decref(&fs->r) != 0)
		return;
	closepgrp(fs->pgrp);
	cclose(fs->root);
	cclose(fs->io);
	for(i = 0; i < Nfidhash; i++){
		for(f = fs->fid[i]; f != nil; f = n){
			if(f->chan != nil)
				cclose(f->chan);
			n = f->next;
			free(f);
		}
	}
	free(fs);
}

int
exwork(void *a)
{
	USED(a);
	return exq.head != nil;
}

void
exslave(void *a)
{
	Export *fs;
	Exq *q, *t, **last;
	char *err;
	int n;

	USED(a);

	closepgrp(up->env->pgrp);
	up->env->pgrp = nil;
	for(;;){
		qlock(&exq.qwait);
		Sleep(&exq.rwait, exwork, nil);

		lock(&exq.l);
		q = exq.head;
		if(q == nil) {
			unlock(&exq.l);
			qunlock(&exq.qwait);
			continue;
		}
		exq.head = q->next;
		q->slave = up;
		unlock(&exq.l);

		qunlock(&exq.qwait);

		fs = q->export;
		lock(&fs->r.l);
		q->next = fs->work;
		fs->work = q;
		unlock(&fs->r.l);

		up->env->pgrp = q->export->pgrp;

		if(exdebug > 1)
			print("exslave dispatch %F\n", &q->rpc);

		if(q->rpc.type >= Tmax || !fcalls[q->rpc.type])
			err = "bad fcall type";
		else
			err = (*fcalls[q->rpc.type])(fs, &q->rpc);

		lock(&fs->r.l);
		last = &fs->work;
		for(t = fs->work; t != nil; t = t->next){
			if(t == q){
				*last = q->next;
				break;
			}
			last = &t->next;
		}
		unlock(&fs->r.l);

		q->rpc.type++;
		if(err){
			q->rpc.type = Rerror;
			strncpy(q->rpc.ename, err, ERRLEN);
		}
		n = convS2M(&q->rpc, q->buf);
		if(n < 0)
			panic("bad message type in exslave");

		if(exdebug)
			print("exslve -> %F\n", &q->rpc);

		if(!waserror()){
			(*devtab[fs->io->type].write)(fs->io, q->buf, n, 0);
			poperror();
		}
		if(exdebug > 1)
			print("exslave written %d\n", q->rpc.tag);

		exfree(q->export);
		free(q);
	}
	print("exslave shut down");
	pexit("exslave shut down", 0);
}

Qid
Exrmtqid(Chan *c)
{
	Qid q;
	ulong qid;

	qid = c->qid.path^(c->dev<<16)^(c->type<<24);
	qid &= ~CHDIR;
	q.path = (c->qid.path&CHDIR)|qid;
	q.vers = c->qid.vers;
	return q;
}

Fid*
Exmkfid(Export *fs, int fid)
{
	ulong h;
	Fid *f, *nf;

	nf = malloc(sizeof(Fid));
	if(nf == nil)
		return nil;
	lock(&fs->fidlock);
	h = fid % Nfidhash;
	for(f = fs->fid[h]; f != nil; f = f->next){
		if(f->fid == fid){
			unlock(&fs->fidlock);
			free(nf);
			return nil;
		}
	}

	nf->next = fs->fid[h];
	if(nf->next != nil)
		nf->next->last = &nf->next;
	nf->last = &fs->fid[h];
	fs->fid[h] = nf;

	nf->fid = fid;
	nf->ref = 1;
	nf->attached = 1;
	nf->offset = 0;
	nf->chan = nil;
	unlock(&fs->fidlock);
	return nf;
}

Fid*
Exgetfid(Export *fs, int fid)
{
	Fid *f;
	ulong h;

	lock(&fs->fidlock);
	h = fid % Nfidhash;
	for(f = fs->fid[h]; f; f = f->next) {
		if(f->fid == fid){
			if(f->attached == 0)
				break;
			f->ref++;
			unlock(&fs->fidlock);
			return f;
		}
	}
	unlock(&fs->fidlock);
	return nil;
}

void
Exputfid(Export *fs, Fid *f)
{
	lock(&fs->fidlock);
	f->ref--;
	if(f->ref == 0 && f->attached == 0){
		if(f->chan != nil)
			cclose(f->chan);
		f->chan = nil;
		*f->last = f->next;
		if(f->next != nil)
			f->next->last = f->last;
		unlock(&fs->fidlock);
		free(f);
		return;
	}
	unlock(&fs->fidlock);
}

char*
Exnop(Export *e, Fcall *f)
{
	USED(e);
	USED(f);
	return nil;
}

char*
Exattach(Export *fs, Fcall *rpc)
{
	Fid *f;

	f = Exmkfid(fs, rpc->fid);
	if(f == nil)
		return Einuse;
	if(waserror()){
		f->attached = 0;
		Exputfid(fs, f);
		return up->env->error;
	}
	f->chan = cclone(fs->root, nil);
	poperror();
	rpc->qid = Exrmtqid(f->chan);
	Exputfid(fs, f);
	return nil;
}

char*
Exclone(Export *fs, Fcall *rpc)
{
	Fid *f, *nf;

	if(rpc->fid == rpc->newfid)
		return Einuse;
	f = Exgetfid(fs, rpc->fid);
	if(f == nil)
		return Enofid;
	nf = Exmkfid(fs, rpc->newfid);
	if(nf == nil){
		Exputfid(fs, f);
		return Einuse;
	}
	if(waserror()){
		Exputfid(fs, f);
		Exputfid(fs, nf);
		return up->env->error;
	}
	nf->chan = cclone(f->chan, nil);
	poperror();
	Exputfid(fs, f);
	Exputfid(fs, nf);
	return nil;
}

char*
Exclunk(Export *fs, Fcall *rpc)
{
	Fid *f;

	f = Exgetfid(fs, rpc->fid);
	if(f != nil){
		f->attached = 0;
		Exputfid(fs, f);
	}
	return nil;
}

char*
Exwalk(Export *fs, Fcall *rpc)
{
	Fid *f;
	Chan *c;

	f = Exgetfid(fs, rpc->fid);
	if(f == nil)
		return Enofid;
	if(waserror()){
		Exputfid(fs, f);
		return up->env->error;
	}
	c = walk(f->chan, rpc->name, 1);
	if(c == nil)
		error(Enonexist);
	poperror();

	f->chan = c;
	rpc->qid = Exrmtqid(c);
	Exputfid(fs, f);
	return nil;
}

char*
Exopen(Export *fs, Fcall *rpc)
{
	Fid *f;
	Chan *c;

	f = Exgetfid(fs, rpc->fid);
	if(f == nil)
		return Enofid;
	if(waserror()){
		Exputfid(fs, f);
		return up->env->error;
	}
	c = f->chan;
	c = (*devtab[c->type].open)(c, rpc->mode);
	poperror();

	f->chan = c;
	f->offset = 0;
	rpc->qid = Exrmtqid(c);
	Exputfid(fs, f);
	return nil;
}

char*
Excreate(Export *fs, Fcall *rpc)
{
	Fid *f;
	Chan *c;

	f = Exgetfid(fs, rpc->fid);
	if(f == nil)
		return Enofid;
	if(waserror()){
		Exputfid(fs, f);
		return up->env->error;
	}
	c = f->chan;
	if(c->mnt && !(c->flag&CCREATE))
		c = createdir(c);
	(*devtab[c->type].create)(c, rpc->name, rpc->mode, rpc->perm);
	poperror();

	f->chan = c;
	rpc->qid = Exrmtqid(c);
	Exputfid(fs, f);
	return nil;
}

char*
Exread(Export *fs, Fcall *rpc)
{
	Fid *f;
	Chan *c;
	long off;
	int dir, n, seek;

	f = Exgetfid(fs, rpc->fid);
	if(f == nil)
		return Enofid;

	c = f->chan;
	dir = c->qid.path & CHDIR;
	if(dir){
		rpc->count -= rpc->count%DIRLEN;
		if(rpc->offset%DIRLEN || rpc->count==0){
			Exputfid(fs, f);
			return Ereaddir;
		}
		if(f->offset > rpc->offset){
			Exputfid(fs, f);
			return Eseekdir;
		}
	}

	if(waserror()) {
		Exputfid(fs, f);
		return up->env->error;
	}

	for(;;){
		n = rpc->count;
		seek = 0;
		off = rpc->offset;
		if(dir && f->offset != off){
			off = f->offset;
			n = rpc->offset - off;
			if(n > MAXDIRREAD)
				n = MAXDIRREAD;
			seek = 1;
		}
		if(dir && c->mnt != nil)
			n = unionread(c, rpc->data, n);
		else{
			c->offset = off;
			n = (*devtab[c->type].read)(c, rpc->data, n, off);
		}
		if(n == 0 || !seek)
			break;
		f->offset = off + n;
		c->offset += n;
	}
	rpc->count = n;
	poperror();
	Exputfid(fs, f);
	return nil;
}

char*
Exwrite(Export *fs, Fcall *rpc)
{
	Fid *f;
	Chan *c;

	f = Exgetfid(fs, rpc->fid);
	if(f == nil)
		return Enofid;
	if(waserror()){
		Exputfid(fs, f);
		return up->env->error;
	}
	c = f->chan;
	if(c->qid.path & CHDIR)
		error(Eisdir);
	rpc->count = (*devtab[c->type].write)(c, rpc->data, rpc->count, rpc->offset);
	poperror();
	Exputfid(fs, f);
	return nil;
}

char*
Exstat(Export *fs, Fcall *rpc)
{
	Fid *f;
	Chan *c;

	f = Exgetfid(fs, rpc->fid);
	if(f == nil)
		return Enofid;
	if(waserror()){
		Exputfid(fs, f);
		return up->env->error;
	}
	c = f->chan;
	(*devtab[c->type].stat)(c, rpc->stat);
	poperror();
	Exputfid(fs, f);
	return nil;
}

char*
Exwstat(Export *fs, Fcall *rpc)
{
	Fid *f;
	Chan *c;

	f = Exgetfid(fs, rpc->fid);
	if(f == nil)
		return Enofid;
	if(waserror()){
		Exputfid(fs, f);
		return up->env->error;
	}
	c = f->chan;
	(*devtab[c->type].wstat)(c, rpc->stat);
	poperror();
	Exputfid(fs, f);
	return nil;
}

char*
Exremove(Export *fs, Fcall *rpc)
{
	Fid *f;
	Chan *c;

	f = Exgetfid(fs, rpc->fid);
	if(f == nil)
		return Enofid;
	if(waserror()){
		Exputfid(fs, f);
		return up->env->error;
	}
	c = f->chan;
	(*devtab[c->type].remove)(c);
	poperror();

	/*
	 * chan is already clunked by remove.
	 * however, we need to recover the chan,
	 * and follow sysremove's lead in making to point to root.
	 */
	c->type = 0;

	f->attached = 0;
	Exputfid(fs, f);
	return nil;
}
