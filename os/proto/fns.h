#include "../port/portfns.h"

void		DEBUG(void);

int		export(int);
ushort		readus(ushort*);
void		pcmciainit(void);
int		pcmspecial(char*);
void		pcmspecialclose(int);
int		inb(int);
void		outb(int, int);
ushort		ins(int);
void		inss(int, void*, int);
void		insssw(int, void*, int);
ulong		inl(int);
void		insl(int, void*, int);
void		outs(int, ushort);
void		outss(int, void*, int);
void		outsssw(int, void*, int);
void		outl(int, ulong);
void		outsl(int, void*, int);
void		lcdon(void);
void		lcdoff(void);
void		boot(void*);
ushort		flashstatus(void*);
void		arginit(void);
void		cleancache(void);
void		clock(Ureg*);
void		clockinit(void);
ulong		confeval(char*);
void		confprint(void);
void		confread(void);
void		confset(char*);
int		conschar(void);
void		consoff(void);
int		consputc(int);
void		dcflush(void*, ulong);
void		dcinvalidate(void*, ulong);
void		evenaddr(ulong);
void		firmware(int);
#define		flushpage(s)		icflush((void*)(s), BY2PG)
#define		getcallerpc(x)	getcallerpcreg()
ulong		getcallerpcreg(void);
ulong		getconfig(void);
int		getline(char*, int);
ulong		getpc(void);
ulong		getsp(void);
ulong		getstatus(void);
ulong		gettmc(void);
void		gotopc(ulong);
void		icflush(void *, ulong);
void		intr0(void);
void		intr10(void);
void		intr11(void);
void		intr12(void);
void		intr13(void);
void		intr14(void);
void		intr15(void);
void		intr16(void);
void		intr17(void);
void		intr18(void);
void		intr19(void);
void		intr1(void);
void		intr20(void);
void		intr21(void);
void		intr22(void);
void		intr23(void);
void		intr24(void);
void		intr25(void);
void		intr26(void);
void		intr27(void);
void		intr28(void);
void		intr29(void);
void		intr2(void);
void		intr30(void);
void		intr31(void);
void		intr32(void);
void		intr33(void);
void		intr34(void);
void		intr35(void);
void		intr36(void);
void		intr37(void);
void		intr38(void);
void		intr39(void);
void		intr3(void);
void		intr40(void);
void		intr41(void);
void		intr42(void);
void		intr43(void);
void		intr44(void);
void		intr45(void);
void		intr46(void);
void		intr47(void);
void		intr48(void);
void		intr49(void);
void		intr4(void);
void		intr50(void);
void		intr51(void);
void		intr52(void);
void		intr53(void);
void		intr54(void);
void		intr55(void);
void		intr56(void);
void		intr57(void);
void		intr58(void);
void		intr59(void);
void		intr5(void);
void		intr60(void);
void		intr61(void);
void		intr62(void);
void		intr63(void);
void		intr64(void);
void		intr6(void);
void		intr7(void);
void		intr8(void);
void		intr9(void);
void		ioinit(int);
int		iprint(char*, ...);
int		kbdinit(void);
void		kbdintr(void);
void		kfault(Ureg*);
int		kprint(char*, ...);
void		kproftimer(ulong);
void		kunmap(KMap*);
void		launchinit(void);
void		launch(int);
ulong		machstatus(void);
void		mntdump(void);
void		newstart(void);
int		newtlbpid(Proc*);
void		nonettoggle(void);
void		novme(int);
void		online(void);
Block*		prepend(Block*, int);
void		prflush(void);
ulong		prid(void);
void		printinit(void);
#define		procrestore(p)
#define		procsave(p)
#define		procsetup(p)		((p)->fpstate = FPinit)
void		progioinit(void);
void		purgetlb(int);
int		putnvram(ulong, void*, int);
int		puttlb(ulong, ulong, ulong);
void		puttlbx(int, ulong, ulong, ulong, int);
void*		pxalloc(ulong);
void*		pxspanalloc(ulong, int, ulong);
void		rdbginit(void);
int		readlog(ulong, char*, ulong);
long*		reg(Ureg*, int);
void		restfpregs(FPsave*, ulong);
long		rtctime(void);
void		screeninit(void);
void		screenputs(char*, int);
void		setconfig(ulong);
void		setstatus(ulong);
void		settmc(ulong);
void		settmr(ulong);
void		settrap(int, void (*)(Ureg*, void*), void*);
void		setvecbase(void*);
void		syslog(char*, int);
void		sysloginit(void);
int		tas(ulong*);
void		touser(void*);
void		uart29kinstall(void);
void		uart29kspecial(int, int, Queue**, Queue**, int(*)(Queue*, int));
void		uartclock(void);
void		uartintr(int);
void		vecinit(void);
void		vectors(void);
void		xylinxinit(void);

#define	waserror()		setlabel(&up->errlab[up->nerrlab++])
#define	kmapperm(x)		kmap(x)

#define KADDR(a)	((void*)(a))
#define PADDR(a)	((ulong)(a))

void	hnputl(void*, ulong v);
void	hnputs(void*, ushort v);
ulong	nhgetl(void*);
ushort	nhgets(void*);

void	ifwrite(void*, Block*, int);
void*	ifinit(int);
ulong	ifaddr(void*);
void	filiput(Block*);
void	fiberint(Ureg*, void*);
ulong	fwblock(ulong, void*, ulong);
ulong	frblock(ulong, void*, ulong);
void	freset(void*);
void	ifree(void*);
void	ifflush(void*);
Block*	iallocb(int);
void*	ifroute(ulong);
ulong	ifunroute(ulong);
ulong	parseip(char*, char*);
