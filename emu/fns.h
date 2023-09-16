extern	ulong	FPcontrol(ulong,ulong);
extern	ulong	FPstatus(ulong,ulong);
extern	void	FPsave(void*);
extern	void	FPrestore(void*);
extern	void	Sleep(Rendez*, int (*)(void*), void*);
extern	void	Wakeup(Rendez*);
extern	void	FPinit(void);
extern	void	addprog(Proc*);
extern	Block*	allocb(int);
extern	int	canlock(Lock*);
extern	int	canqlock(QLock*);
extern	void	cclose(Chan*);
extern	void	chandevinit(void);
extern	void	chanfree(Chan*);
extern	Chan*	cclone(Chan*, Chan*);
extern	void	closefgrp(Fgrp*);
extern	void	closepgrp(Pgrp*);
extern	int	cmount(Chan*, Chan*, int, char*);
extern	int	convD2M(Dir*, char*);
extern	int	convM2D(char*, Dir*);
extern	Chan*	createdir(Chan*);
extern	void	cunmount(Chan*, Chan*);
extern	int	decref(Ref*);
extern	Chan*	devattach(int, char*);
extern	Block*	devbread(Chan*, long, ulong);
extern	long	devbwrite(Chan*, Block*, ulong);
extern	Chan*	devclone(Chan*, Chan*);
extern	void	devdir(Chan*, Qid, char*, long, char*, long, Dir*);
extern	long	devdirread(Chan*, char*, long, Dirtab*, int, Devgen*);
extern	int	devgen(Chan*, Dirtab*, int, int, Dir*);
extern	int	devno(int, int);
extern	Chan*	devopen(Chan*, int, Dirtab*, int, Devgen*);
extern	void	devstat(Chan*, char*, Dirtab*, int, Devgen*);
extern	int	devwalk(Chan*, char*, Dirtab*, int, Devgen*);
extern	int	dirmodeconv(va_list*, Fconv*);
extern	void	disfault(void*, char*);
extern	void	disinit(void*);
extern	Chan*	domount(Chan*);
extern	Fgrp*	dupfgrp(Fgrp*);
extern	void	emuinit(void*);
extern	int	eqchan(Chan*, Chan*, int);
extern	int	eqqid(Qid, Qid);
extern	int	export(int, int);
extern	Chan*	fdtochan(Fgrp*, int, int, int, int);
extern	void	freeb(Block*);
extern	void	freeblist(Block*);
extern	ulong	getFPcontrol(void);
extern	ulong	getFPstatus(void);
extern	int	incref(Ref*);
extern	void	isdir(Chan*);
extern	int	kannounce(char*, char*);
extern	int	kdial(char*, char*, char*, int*);
extern	int	kproc(char*, void (*)(void*), void*);
extern	int	kfgrpclose(Fgrp*, int);
extern	void	libinit(char*);
extern	void	lock(Lock*);
extern	void	mountfree(Mount*);
extern	Chan*	namec(char*, int, int, ulong);
extern	void	nameok(char*);
extern	Chan*	newchan(void);
extern	Fgrp*	newfgrp(void);
extern	Mount*	newmount(Mhead*, Chan*, int, char*);
extern	Pgrp*	newpgrp(void);
extern	Proc*	newproc(void);
extern	char*	nextelem(char*, char*);
extern	void	nexterror(void);
extern	int	openmode(ulong);
extern	int	oscmd(char*, int*, int*);
extern	void	oserrstr(char*);
extern	long	osmillisec(void);
extern	int	osmillisleep(ulong);
extern	vlong	osusectime(void);
extern	Block*	padblock(Block*, int);
extern	void	panic(char*, ...);
extern	int	parsefields(char*, char**, int, char*);
extern	void	pexit(char*, int);
extern	void	pgrpcpy(Pgrp*, Pgrp*);
extern	void	ptclose(Pthash*);
extern	Path*	ptenter(Pthash*, Path*, char*);
extern	int	ptpath(Path*, char*, int);
extern	Block*	pullupblock(Block*, int);
extern	Block*	qbread(Queue*, int);
extern	long	qbwrite(Queue*, Block*);
extern	int	qcanread(Queue*);
extern	void	qclose(Queue*);
extern	int	qclosed(Queue*);
extern	int	qconsume(Queue*, void*, int);
extern	Block*	qcopy(Queue*, int, ulong);
extern	void	qdiscard(Queue*, int);
extern	void	qflush(Queue*);
extern	void	qfree(Queue*);
extern	int	qfull(Queue*);
extern	Block*	qget(Queue*);
extern	void	qhangup(Queue*, char*);
extern	int	qiwrite(Queue*, void*, int);
extern	int	qlen(Queue*);
extern	void	qlock(QLock*);
extern	void	qnoblock(Queue*, int);
extern	Queue*	qopen(int, int, void (*)(void*), void*);
extern	int	qpass(Queue*, Block*);
extern	int	qproduce(Queue*, void*, int);
extern	long	qread(Queue*, void*, int);
extern	void	qreopen(Queue*);
extern	void	qsetlimit(Queue*, int);
extern	int	qstate(Queue*);
extern	void	qunlock(QLock*);
extern	int	qwindow(Queue*);
extern	long	qwrite(Queue*, void*, int);
extern	int	readkbd(void);
extern	int	readnum(ulong, char*, ulong, ulong, int);
extern	int	readnum_vlong(ulong, char*, ulong, vlong, int);
extern	int	readstr(ulong, char*, ulong, char*);
extern	int	rendezvous(void*, ulong);
extern	void	rlock(RWlock*);
extern	void	runlock(RWlock*);
extern	void*	sbrk(int);
extern	void	seterror(char*, ...);
extern	void	setid(char*);
extern	char*	skipslash(char*);
extern	void	srvrtinit(void);
extern	void	swiproc(Proc*);
extern	int	unionread(Chan*, void*, long);
extern	void	unlock(Lock*);
extern	void	vmachine(void*);
extern	Chan*	walk(Chan*, char*, int);
extern	void	wlock(RWlock*);
extern	void	wunlock(RWlock*);
extern	void	validaddr(void*, int, int);
extern	void	cleanexit(int);
extern	ulong	getcallerpc(void*);
extern	void	oshostintr(Proc*);
extern	void	osenter(void);
extern	void	osleave(void);
extern	void	ospause(void);
extern  void    osyield(void);
extern	void	osspin(Rendez*);
extern	Rb*	osraninit(void);
extern	void	oswakeupproducer(Rendez*);

/* Namespace Emulation */
extern	int	kbind(char*, char*, int);
extern	int	kclose(int);
extern	int	kcreate(char*, int, ulong);
extern	int	kdup(int, int);
extern	int	kfstat(int, char*);
extern	int	kfwstat(int, char*);
extern	int	kmount(int, char*, int, char*);
extern	int	kunmount(char*, char*);
extern	int	kopen(char*, int);
extern	long	kread(int, void*, long);
extern	int	kremove(char*);
extern	long	kseek(int, long, int);
extern	int	kstat(char*, char*);
extern	long	kwrite(int, void*, long);
extern	int	kwstat(char*, char*);
extern	int	kdirstat(char*, Dir*);
extern	int	kdirfstat(int, Dir*);
extern	int	kdirwstat(char*, Dir*);
extern	int	kdirfwstat(int, Dir*);
extern	long	kdirread(int, Dir*, long);
extern	int	klisten(char*, char*);

/* Common Device Drivers */
extern	void	rootinit(void);
extern	Chan*	rootattach(void*);
extern	Chan*	rootclone(Chan*, Chan*);
extern	int	rootwalk(Chan*, char*);
extern	void	rootstat(Chan*, char*);
extern	Chan*	rootopen(Chan*, int);
extern	void	rootcreate(Chan*, char*, int, ulong);
extern	void	rootclose(Chan*);
extern	long	rootread(Chan*, void*, long, ulong);
extern	long	rootwrite(Chan*, void*, long, ulong);
extern	void	rootremove(Chan*);
extern	void	rootwstat(Chan*, char*);
extern	void	srvinit(void);
extern	Chan*	srvattach(void*);
extern	Chan*	srvclone(Chan*, Chan*);
extern	int	srvwalk(Chan*, char*);
extern	void	srvstat(Chan*, char*);
extern	Chan*	srvopen(Chan*, int);
extern	void	srvcreate(Chan*, char*, int, ulong);
extern	void	srvclose(Chan*);
extern	long	srvread(Chan*, void*, long, ulong);
extern	long	srvwrite(Chan*, void*, long, ulong);
extern	void	srvremove(Chan*);
extern	void	srvwstat(Chan*, char*);
extern	void	mntinit(void);
extern	Chan*	mntattach(void*);
extern	Chan*	mntclone(Chan*, Chan*);
extern	int	mntwalk(Chan*, char*);
extern	void	mntstat(Chan*, char*);
extern	Chan*	mntopen(Chan*, int);
extern	void	mntcreate(Chan*, char*, int, ulong);
extern	void	mntclose(Chan*);
extern	long	mntread(Chan*, void*, long, ulong);
extern	long	mntwrite(Chan*, void*, long, ulong);
extern	void	mntremove(Chan*);
extern	void	mntwstat(Chan*, char*);
extern	void	ipinit(void);
extern	Chan*	ipattach(void*);
extern	Chan*	ipclone(Chan*, Chan*);
extern	int	ipwalk(Chan*, char*);
extern	void	ipstat(Chan*, char*);
extern	Chan*	ipopen(Chan*, int);
extern	void	ipcreate(Chan*, char*, int, ulong);
extern	void	ipclose(Chan*);
extern	long	ipread(Chan*, void*, long, ulong);
extern	long	ipwrite(Chan*, void*, long, ulong);
extern	void	ipremove(Chan*);
extern	void	ipwstat(Chan*, char*);
extern	void	coninit(void);
extern	Chan*	conattach(void*);
extern	Chan*	conclone(Chan*, Chan*);
extern	int	conwalk(Chan*, char*);
extern	void	constat(Chan*, char*);
extern	Chan*	conopen(Chan*, int);
extern	void	concreate(Chan*, char*, int, ulong);
extern	void	conclose(Chan*);
extern	long	conread(Chan*, void*, long, ulong);
extern	long	conwrite(Chan*, void*, long, ulong);
extern	void	conwstat(Chan*, char*);
extern	void	conremove(Chan*);
extern	void	conwstat(Chan*, char*);
extern	void	fsinit(void);
extern	Chan*	fsattach(void*);
extern	Chan*	fsclone(Chan*, Chan*);
extern	int	fswalk(Chan*, char*);
extern	void	fsstat(Chan*, char*);
extern	Chan*	fsopen(Chan*, int);
extern	void	fscreate(Chan*, char*, int, ulong);
extern	void	fsclose(Chan*);
extern	long	fsread(Chan*, void*, long, ulong);
extern	long	fswrite(Chan*, void*, long, ulong);
extern	void	fsremove(Chan*);
extern	void	fswstat(Chan*, char*);
extern	void	sslinit(void);
extern	Chan*	sslattach(void*);
extern	Chan*	sslclone(Chan*, Chan*);
extern	int	sslwalk(Chan*, char*);
extern	void	sslstat(Chan*, char*);
extern	Chan*	sslopen(Chan*, int);
extern	void	sslcreate(Chan*, char*, int, ulong);
extern	void	sslclose(Chan*);
extern	long	sslread(Chan*, void*, long, ulong);
extern	Block*	sslbread(Chan*, long, ulong);
extern	long	sslwrite(Chan*, void*, long, ulong);
extern	long	sslbwrite(Chan*, Block*, ulong);
extern	void	sslremove(Chan*);
extern	void	sslwstat(Chan*, char*);
extern	void	drawinit(void);
extern	Chan*	drawattach(void*);
extern	Chan*	drawclone(Chan*, Chan*);
extern	int	drawwalk(Chan*, char*);
extern	void	drawstat(Chan*, char*);
extern	Chan*	drawopen(Chan*, int);
extern	void	drawcreate(Chan*, char*, int, ulong);
extern	void	drawclose(Chan*);
extern	long	drawread(Chan*, void*, long, ulong);
extern	Block*	drawbread(Chan*, long, ulong);
extern	long	drawwrite(Chan*, void*, long, ulong);
extern	long	drawbwrite(Chan*, Block*, ulong);
extern	void	drawremove(Chan*);
extern	void	drawwstat(Chan*, char*);
extern	void	cmdinit(void);
extern	Chan*	cmdattach(void*);
extern	Chan*	cmdclone(Chan*, Chan*);
extern	int	cmdwalk(Chan*, char*);
extern	void	cmdstat(Chan*, char*);
extern	Chan*	cmdopen(Chan*, int);
extern	void	cmdcreate(Chan*, char*, int, ulong);
extern	void	cmdclose(Chan*);
extern	long	cmdread(Chan*, void*, long, ulong);
extern	Block*	cmdbread(Chan*, long, ulong);
extern	long	cmdwrite(Chan*, void*, long, ulong);
extern	long	cmdbwrite(Chan*, Block*, ulong);
extern	void	cmdremove(Chan*);
extern	void	cmdwstat(Chan*, char*);
extern	void	proginit(void);
extern	Chan*	progattach(void*);
extern	Chan*	progclone(Chan*, Chan*);
extern	int	progwalk(Chan*, char*);
extern	void	progstat(Chan*, char*);
extern	Chan*	progopen(Chan*, int);
extern	void	progcreate(Chan*, char*, int, ulong);
extern	void	progclose(Chan*);
extern	long	progread(Chan*, void*, long, ulong);
extern	Block*	progbread(Chan*, long, ulong);
extern	long	progwrite(Chan*, void*, long, ulong);
extern	long	progbwrite(Chan*, Block*, ulong);
extern	void	progremove(Chan*);
extern	void	progwstat(Chan*, char*);
extern	void	tinyfsinit(void);
extern	Chan*	tinyfsattach(void*);
extern	Chan*	tinyfsclone(Chan*, Chan*);
extern	int	tinyfswalk(Chan*, char*);
extern	void	tinyfsstat(Chan*, char*);
extern	Chan*	tinyfsopen(Chan*, int);
extern	void	tinyfscreate(Chan*, char*, int, ulong);
extern	void	tinyfsclose(Chan*);
extern	long	tinyfsread(Chan*, void*, long, ulong);
extern	Block*	tinyfsbread(Chan*, long, ulong);
extern	long	tinyfswrite(Chan*, void*, long, ulong);
extern	long	tinyfsbwrite(Chan*, Block*, ulong);
extern	void	tinyfsremove(Chan*);
extern	void	tinyfswstat(Chan*, char*);


extern	Pool*	poolmk(char*, int, int, int);

extern	Chan*	audioattach(void*);
extern	Chan*	audioclone(Chan*, Chan*);
extern	Chan*	audioopen(Chan*, int);
extern	int	audiowalk(Chan*, char*);
extern	long	audioread(Chan*, void*, long, ulong);
extern	long	audiowrite(Chan*, void*, long, ulong);
extern	void	audioclose(Chan*);
extern	void	audiocreate(Chan*, char*, int, ulong);
extern	void	audioinit(void);
extern	void	audioremove(Chan*);
extern	void	audiostat(Chan*, char*);
extern	void	audiowstat(Chan*, char*);
extern	Block*	audiobread(Chan*, long, ulong);
extern	long	audiobwrite(Chan*, Block*, ulong);
