#include	"lib9.h"
#include	"dat.h"
#include	"fns.h"
#include	"error.h"

enum
{
	KSTACK	= 32*1024,
	DELETE	= 0x7F,
};

Proc	**Xup;

extern	void	killrefresh(void);
extern	void	tramp(char*, void (*)(void*), void*);
extern	void	vstack(void*);
extern	void	lockinit(void*);

ulong	ustack;

int		(*canlockp)(Lock*);

void
pexit(char *msg, int t)
{
	Osenv *e;

	USED(t);
	USED(msg);

	lock(&procs.l);
	if(up->prev) 
		up->prev->next = up->next;
	else
		procs.head = up->next;

	if(up->next)
		up->next->prev = up->prev;
	else
		procs.tail = up->prev;
	unlock(&procs.l);

/*	print("pexit: %s: %s\n", up->text, msg);	/**/
	e = up->env;
	if(e != nil) {
		closefgrp(e->fgrp);
		closepgrp(e->pgrp);
	}
	free(up);
	_exits("");
}

int
kproc(char *name, void (*func)(void*), void *arg)
{
	int pid;
	Proc *p;
	Pgrp *pg;
	Fgrp *fg;

	p = newproc();
	p->kstack = mallocz(KSTACK, 0);
	if(p == nil || p->kstack == nil)
		panic("kproc: no memory");
		
	pg = up->env->pgrp;
	p->env->pgrp = pg;
	fg = up->env->fgrp;
	p->env->fgrp = fg;
	incref(&pg->r);
	incref(&fg->r);

	p->env->uid = up->env->uid;
	p->env->gid = up->env->gid;
	memmove(p->env->user, up->env->user, NAMELEN);

	strcpy(p->text, name);

	p->func = func;
	p->arg = arg;

	lock(&procs.l);
	if(procs.tail != nil) {
		p->prev = procs.tail;
		procs.tail->next = p;
	}
	else {
		procs.head = p;
		p->prev = nil;
	}
	procs.tail = p;
	unlock(&procs.l);

	/*
	 * switch back to the unshared stack to do the fork
	 * only the parent returns from kproc
	 */
	up->kid = p;
	up->kidsp = p->kstack;
	pid = setjmp(up->sharestack);
	if(!pid)
		longjmp(up->privstack, 1);
	return pid;
}

void
traphandler(void *reg, char *msg)
{
	/* Ignore pipe writes from devcmd */
	if(strstr(msg, "write on closed pipe") != nil)
		noted(NCONT);

	if(up->intwait == 0)
		disfault(reg, msg);

	up->intwait = 0;
	noted(NCONT);
}

int
readfile(char *path, char *buf, int n)
{
	int fd;

	fd = open(path, OREAD);
	if(fd >= 0) {
		n = read(fd, buf, n-1);
		if(n >= 0)
			buf[n] = '\0';
		close(fd);
	}
	return n;
}

void
libinit(char *imod)
{
	char *sp;
	Proc *xup, *p;
	int fd, cfd, n, pid;

	/*
	 * setup personality
	 */
	readfile("/dev/user", eve, NAMELEN);
	readfile("/dev/sysname", sysname, 3*NAMELEN);

	/*
	 * guess at a safe stack for vstack
	 */
	ustack = (ulong)&fd;

	rfork(RFNAMEG|RFREND);

	fd = open("/dev/consctl", OWRITE);
	if(fd < 0)
		fprint(2, "libinit: open /dev/consctl: %r\n");
	n = write(fd, "rawon", 5);
	if(n != 5)
		fprint(2, "keyboard rawon (n=%d, %r)\n", n);


/*	bind("/net", "/usr/inferno/net", MREPL); /* works on brazil */
	bind("#Itcp", "/usr/inferno/net", MREPL);
	bind("#Iudp", "/usr/inferno/net", MAFTER);
	cfd = open("/srv/cs", OREAD);
	mount(cfd, "/usr/inferno/net", MAFTER, "");

	if(sflag == 0)
		notify(traphandler);

	Xup = &xup;

	/*
	 * dummy up a up and stack so the first proc
	 * calls emuinit after setting up his private jmp_buf
	 */
	p = newproc();
	p->kstack = mallocz(KSTACK, 0);
	if(p == nil || p->kstack == nil)
		panic("libinit: no memory");
	sp = p->kstack;
	p->func = emuinit;
	p->arg = imod;

	/*
	 * set up a stack for forking kids on separate stacks.
	 * longjmp back here from kproc.
	 */
	while(setjmp(p->privstack)){
		p = up->kid;
		sp = up->kidsp;
		switch(pid = rfork(RFPROC|RFMEM)){
		case 0:
			/*
			 * send the kid around the loop to set up his private jmp_buf
			 */
			break;
		default:
			/*
			 * parent just returns to his shared stack in kproc
			 */
			longjmp(up->sharestack, pid);
			panic("longjmp failed");
		}
	}

	/*
	 * you get here only once per Proc
	 * go to the shared memory stack
	 */
	up = p;
	up->sigid = getpid();
	tramp(sp+KSTACK, up->func, up->arg);
	panic("tramp returned");
}

void
oshostintr(Proc *p)
{
	postnote(PNPROC, p->sigid, Eintr);
}

void
cleanexit(int x)
{
	USED(x);
	killrefresh();
	postnote(PNGROUP, getpid(), "interrupt");
	exits("interrupt");
}

int
readkbd(void)
{
	int n;
	char buf[1];

	n = read(0, buf, sizeof(buf));
	if(n != 1) {
		print("keyboard close (n=%d, %r)\n", n);
		pexit("keyboard", 0);
	}
	switch(buf[0]) {
	case DELETE:
		cleanexit(0);
	case '\r':
		buf[0] = '\n';
	}
	return buf[0];
}

int
canlock(Lock *l)
{
	if(canlockp == nil)
		lockinit(&canlockp);
	return canlockp(l);
}

typedef struct Targ Targ;
struct Targ
{
	int	fd;
	int*	spin;
	char*	cmd;
};

void
exectramp(Targ *targ)
{
	int fd, i, nfd;
	char *argv[4], buf[KSTACK];

	fd = targ->fd;
	strncpy(buf, targ->cmd, sizeof(buf)-1);
	*targ->spin = 0;

	argv[0] = "/bin/rc";
	argv[1] = "-c";
	argv[2] = buf;
	argv[3] = nil;

	nfd = NFD;
	for(i = 0; i < nfd; i++)
		if(i != fd)
			close(i);

	dup(fd, 0);
	dup(fd, 1);
	dup(fd, 2);
	close(fd);
	exec(argv[0], argv);
	exits("");
}

int
oscmd(char *cmd, int *rfd, int *sfd)
{
	Targ targ;
	int spin, *spinptr, fd[2];

	if(pipe(fd) < 0)
		return -1;

	spinptr = &spin;
	spin = 1;

	targ.fd = fd[0];
	targ.cmd = cmd;
	targ.spin = spinptr;

	switch(fork()) {
	case -1:
		return -1;
	case 0:
		vstack(&targ);			/* Never returns */
	default:
		while(*spinptr)
			;
		break;
	}
	close(fd[0]);

	*rfd = fd[1];
	*sfd = fd[1];
	return 0;
}

long
osmillisec(void)
{
	/* vlong because computers stay up more than 24 days */
	static vlong msec0 = 0;
	static int millifd;
	static char buf[30];
	int n;

	if(msec0 == 0){
		millifd = open("/dev/msec",OREAD);  /* never closed */
		if(millifd<0){
			fprint(2,"can't open /dev/msec\n");
			return(0);
		}
		n = read(millifd,buf,sizeof(buf)-1);
		if(n<0){
			fprint(2,"read err on /dev/msec\n");
			return(0);
		}
		buf[n] = 0;
		msec0 = strtoll(buf,nil,10);
		return(0);
	}
	seek(millifd,0,0);
	n = read(millifd, buf, sizeof(buf)-1);
	if(n<0) {
		fprint(2,"read err on /dev/msec\n");
		return(0);
	}
	buf[n] = 0;
	return(strtoll(buf,nil,10)-msec0);
}

/*
 * Return the time since the epoch in microseconds
 * The epoch is defined at 1 Jan 1970
 */
vlong
osusectime(void)
{
	return (vlong)time(0) * 1000000;
}

int
osmillisleep(ulong milsec)
{
	sleep(milsec);
	return 0;
}

void
osyield(void)
{
	sleep(0);
}

/*
 * SGI Power specific
 */

enum
{
	Pagesize	= 4096,
	Semperpg	= Pagesize/(16*sizeof(uint)),
	Lockaddr	= 0x60000000,
}

powerlockinit(void)
{
	int n;

	n = segattach(SG_CEXEC, "lock", (void*)Lockaddr, Pagesize);
	if(n < 0)
		return 0;
	memset((void*)Lockaddr, 0, Pagesize);
	return 1;
}

powercanlock(Lock *lk)
{
	int *hwsem;
	int hash;

	/* Use low order lock bits to generate hash */

	hash = ((int)lk/sizeof(int)) & (Semperpg-1);
	hwsem = (int*)Lockaddr+hash;

	if((*hwsem & 1) == 0) {
		if(lk->key)
			*hwsem = 0;
		else {
			lk->key = 1;
			*hwsem = 0;
			return 1;
		}
	}
	return 0;
}

void
ospause(void)
{
	for(;;)
                sleep(1000000);
}

static Rb rb;
extern int rbnotfull(void*);

void
osspin(Rendez *prod)
{
        for(;;){
                if((rb.randomcount & 0xffff) == 0 && !rbnotfull(0)) {
                        Sleep(prod, rbnotfull, 0);
                }
                rb.randomcount++;
        }
}
 
Rb*
osraninit(void)
{
	return &rb;
}

void
oswakeupproducer(Rendez *rendez)
{
	Wakeup(rendez);
}

