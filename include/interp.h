typedef uchar		BYTE;		/* 8  bits */
typedef int		WORD;		/* 32 bits */
typedef vlong		LONG;		/* 64 bits */
typedef double		REAL;		/* double IEEE754 */

enum
{
	Palt,				/* blocked in alt instruction */
	Psend,				/* waiting to send */
	Precv,				/* waiting to recv */
	Pdebug,				/* debugged */
	Pready,				/* ready to be scheduled */
	Prelease,			/* interpreter released */
	Pexiting,			/* exit because of kill or error */
	Pbroken,			/* thread crashed */

	propagator	= 3,		/* gc marking color */

	PRNSIZE	= 1024,
	BIHASH	= 23,

	/* STRUCTALIGN is the unit to which the compiler aligns structs. */
	/* It really should be defined somewhere else */
	STRUCTALIGN = sizeof(int)	/* must be >=2 because of Strings */
};

typedef struct Alt	Alt;
typedef struct Channel	Channel;
typedef struct Inst	Inst;
typedef struct Module	Module;
typedef struct Modlink	Modlink;
typedef struct Modl	Modl;
typedef struct Type	Type;
typedef struct Prog	Prog;
typedef struct Heap	Heap;
typedef struct Link	Link;
typedef struct List	List;
typedef struct Array	Array;
typedef struct String	String;
typedef union  Linkpc	Linkpc;
typedef struct REG	REG;
typedef struct Frame	Frame;
typedef union  Stkext	Stkext;
typedef struct Atidle	Atidle;
typedef struct Altc	Altc;

struct Frame
{
	Inst*	lr;	/* REGLINK isa.h */
	uchar*	fp;	/* REGFP */
	Module*	mr;	/* REGMOD */
	Type*	t;	/* REGTYPE */
};

union Stkext
{
	uchar	stack[1];
	struct {
		Type*	TR;
		uchar*	SP;
		uchar*	TS;
		uchar*	EX;
		union {
			uchar	fu[1];
			Frame	fr[1];
		} tos;
	} reg;
};

struct Array
{
	WORD	len;
	Type*	t;
	Array*	root;
	uchar*	data;
};

struct List
{
	List*	tail;
	Type*	t;
	WORD	data[STRUCTALIGN];
};

struct Channel
{
	Prog*	send;		/* Queue of progs ready to send */
	Prog*	sendalt;	/* Single prog waiting to send in ALT */
	Prog*	recv;		/* Queue of progs ready to receive */
	Prog*	recvalt;	/* Single prog waiting to receive in ALT */
	void	(*mover)(void);	/* Data mover */
	union {
		WORD	w;
		Type*	t;
	} mid;
};

struct String
{
	int	len;		/* string length */
	int	max;		/* maximum length in representation */
	char*	tmp;
	union {
	#define	Sascii	data.ascii
	#define Srune	data.runes
		char	ascii[STRUCTALIGN];
		Rune	runes[1];
	}data;	
};

union Linkpc
{
	void	(*runt)(void*);
	Inst*	pc;
};

struct Link
{
	Link*	next;
	int	sig;
	Module*	m;
	Type*	frame;
	Linkpc	u;
	char	name[STRUCTALIGN];
};

typedef union	Adr	Adr;
union Adr
{
	WORD	imm;
	WORD	ind;
	Inst*	ins;
	struct {
		ushort	f;	/* First indirection */	
		ushort	s;	/* Second indirection */
	} i;
};

struct Inst
{
	uchar	op;
	uchar	add;
	ushort	reg;
	Adr	s;
	Adr	d;
};

struct Altc
{
	Channel*	c;
	void*		ptr;
};

struct Alt
{
	int	nsend;
	int	nrecv;
	Altc	ac[1];
};

struct Type
{
	int	ref;
	void	(*free)(Heap*);
	void	(*mark)(Type*, void*);
	int	size;
	int	nogc;
	int	np;
	void*	destroy;
	void*	initialize;
	uchar	map[STRUCTALIGN];
};

struct REG
{
	Inst*	PC;		/* Program counter */
	uchar*	MP;		/* Module data */
	uchar*	FP;		/* Frame pointer */
	uchar*	SP;		/* Stack pointer */
	uchar*	TS;		/* Top of allocated stack */
	uchar*	EX;		/* Extent register */
	Module*	M;		/* Module */
	int	IC;		/* Instruction count for this quanta */
	Inst*	xpc;		/* Saved program counter */
	void*	s;		/* Source */
	void*	d;		/* Destination */
	void*	m;		/* Middle */
	WORD	t;		/* Middle temporary */
	WORD	st;		/* Source temporary */
	WORD	dt;		/* Destination temporary */
};

struct Prog
{
	REG	R;		/* Register set */
	Prog*	link;		/* Run queue */
	Prog*	comm;		/* Communication queue */
	void*	ptr;		/* Channel data pointer */
	int	state;		/* Scheduler state */
	int	kill;		/* Set if prog should error */
	int	pid;		/* unique Prog id */
	int	grp;		/* group id */
	int	quanta;		/* time slice */
	Prog*	prev;
	Prog*	next;
	void	(*addrun)(Prog*);
	void	(*xec)(Prog*);

	void*	osenv;
};

struct Module
{
	int	ref;		/* Use count */
	int	compiled;	/* Compiled into native assembler */
	int	shared;		/* MP is shared data */
	ulong	ss;		/* Stack size */
	ulong	rt;		/* Runtime flags */
	ulong	mtime;		/* Modtime of dis file */
	Module*	orig;		/* parent module */
	Inst*	prog;		/* text segment */
	uchar*	mp;		/* Module data */
	int	ntype;		/* Number of type descriptors */
	Type**	type;		/* Type descriptors */
	Inst*	entry;		/* Entry PC */
	Type*	entryt;		/* Entry frame */
	char	name[NAMELEN];	/* Implements type */
	char*	path;		/* File module loaded from */
	Module*	link;		/* Links */
	Link*	ext;		/* External dynamic links */
};

struct Modl
{
	Linkpc	u;		/* PC of Dynamic link */
	Type*	frame;		/* Frame type for this entry */
};

struct Modlink
{
	uchar*	MP;		/* Module data for this instance */
	Module*	m;		/* The real module */
	Modl	links[1];
};

struct Heap
{
	int	color;		/* Allocation color */
	ulong	ref;
	Type*	t;
};

struct	Atidle
{
	int	(*fn)(void*);
	void*	arg;
	Atidle*	link;
};

#define H2D(t, x)	((t)(((uchar*)(x))+sizeof(Heap)))
#define D2H(x)		((Heap*)(((uchar*)(x))-sizeof(Heap)))
#define H		((void*)(-1))
#define SEXTYPE(f)	((Stkext*)((uchar*)(f)-OA(Stkext, reg.tos.fu)))
#define Setmark(h)	if((h)->color!=mutator) { (h)->color = propagator; nprop=1; }
#define gclock()	gchalt++
#define gcunlock()	gchalt--
#define gcruns()	(gchalt == 0)

extern	int	cflag;
extern	int	nproc;
extern	Type	Tarray;
extern	Type	Tstring;
extern	Type	Tchannel;
extern	Type	Tlist;
extern	Type	Tmodlink;
extern	Type*	TImage;
extern	Type	Tptr;
extern	Type	Tbyte;
extern	REG	R;
extern	String	snil;
extern	void	(*optab[256])(void);
extern	void	(*comvec)(void);
extern	Module*	modules;
extern	int	mutator;
extern	int	nprop;
extern	int	gchalt;
extern	int	gccolor;
extern	int	minvalid;
extern	char	Tbounds[];

extern	int		Dconv(va_list*, Fconv*);
extern	void		acquire(void);
extern	void		addrun(Prog*);
extern	void		altdone(Alt*, Prog*, Channel*, int);
extern	int		atidle(int (*)(void*), void*);
extern	void		atidledont(int (*)(void*), void*);
extern	void		builtinmod(char*, void*);
extern	void		cblock(Prog*);
extern	void		cmovw(void*, void*);
extern	Channel*	cnewc(void (*)(void));
extern	int		compile(Module*, int);
extern	void		crecv(Channel*, void*);
extern	void		csend(Channel*, void*);
extern	void		csendptrs(Channel*, void*, void**);
extern	Prog*		currun(void);
extern	void		dbgexit(Prog*, int, char*);
extern	void		dbgxec(Prog*);
extern	void		delprog(Prog*, char*);
extern	Prog*		delrun(int);
extern	void		delrunq(Prog*);
extern	Prog*		delruntail(int);
extern	void		destroy(void*);
extern	void		destroyimage(ulong);
extern	void		destroylinks(Module*);
extern	void		destroystack(REG*);
extern	void		drawmodinit(void);
extern	Type*		dtype(void (*)(Heap*), int, uchar*, int);
extern	Module*		dupmod(Module*);
extern	void		error(char*);
extern	void		extend(void);
extern	void		freeheap(Heap*);
extern	void		freeptrs(void*, Type*);
extern	void		freestring(Heap*);
extern	void		freetype(Type*);
extern	int		gfltconv(va_list*, Fconv*);
extern	void		go(Module*);
extern	Heap*		heap(Type*);
extern	int		heapref(void*);
extern	Heap*		heapz(Type*);
extern	void		incmem(void*, Type*);
extern	void		initarray(Type*, Array*);
extern	void		initmem(Type*, void*);
extern	void		irestore(Prog*);
extern	Prog*		isave(void);
extern	void		keyringmodinit(void);
extern	int		killprog(Prog*);
extern	Modlink*	linkmod(Module*, BYTE*);
extern	Module*		load(char*);
extern	Module*		lookmod(char*);
extern	void		markarray(Type*, void*);
extern	void		markheap(Type*, void*);
extern	void		marklist(Type*, void*);
extern	void		markmodl(Type*, void*);
extern	void		mathmodinit(void);
extern	Array*		mem2array(void*, int);
extern	void		mlink(Module*, uchar*, int, int, Type*);
extern	void		movp(void);
extern	void		movtmp(void);
extern	void		movtmpsafe(void);
extern	Module*		newmod(char*);
extern	void		newmp(void*, void*, Type*);
extern	Prog*		newprog(Prog*);
extern	void		newstack(Prog*);
extern	Heap*		nheap(int);
extern	void		noptrs(Type*, void*);
extern	int		nprog(void);
extern	void		opinit(void);
extern	Module*		parsemod(char*, uchar*, ulong);
extern	void		prefabmodinit(void);
extern	Prog*		progn(int);
extern	Prog*		progpid(int);
extern	void		pushrun(Prog*);
extern	uchar*		readmod(char*, Module*, ulong*, int);
extern	void		irecv(void);
extern	void		release(void);
extern	void		releasex(void);
extern	void		retnstr(char*, int, String**);
extern	void		retstr(char*, String**);
extern	void		rungc(Prog*);
extern	void		runtime(Module*, char*, int, void(*)(void*), Type*);
extern	void		safemem(void*, Type*, void (*)(void*));
extern	int		segflush(void *, ulong);
extern	void		isend(void);
extern	void		seterror(char*, ...);
extern	void		sethints(String*, int);
extern	String*		splitc(String**, int);
extern	uchar*		stack(Frame*);
extern	int		stringblen(String*);
extern	char*		stringchk(String*);
extern	int		stringcmp(String*, String*);
extern	String*		stringdup(String*);
extern	String*		stringheap(int, int, int, int);
extern	char*		syserr(char*, char*, Prog*);
extern	void		sysinit(void);
extern	void		sysmodinit(void);
extern	int		timeconv(va_list*, Fconv*);
extern	void		tellsomeone(Prog*, char*);
extern	void		tkmodinit(void);
extern	void		unextend(Frame*);
extern	void		unframe(void);
extern	void		unload(Module*);
extern	int		utfnlen(char*, int, int*);
extern	int		verifysigner(uchar*, int, uchar*);
extern	void		xec(Prog*);
extern	void		xecalt(int);
extern	int		xprint(Prog*, void*, void*, String*, char*, int);
extern	void		iyield(void);
extern	String*		newstring(int);
extern	int		runeslen(Rune*, int);
extern	String*		c2string(char*, int);
extern	char*		string2c(String*);
extern	List*		cons(ulong, List**);
extern	String*		slicer(ulong, ulong, String*);
extern	String*		addstring(String*, String*);

#define	O(t,e)		((long)(&((t*)0)->e))
#define	OA(t,e)		((long)(((t*)0)->e))
