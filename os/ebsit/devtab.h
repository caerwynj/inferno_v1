void	consreset(void);
void	consinit(void);
Chan*	consattach(char*);
Chan*	consclone(Chan*, Chan*);
int	conswalk(Chan*, char*);
void	consstat(Chan*, char*);
Chan*	consopen(Chan*, int);
void	conscreate(Chan*, char*, int, ulong);
void	consclose(Chan*);
long	consread(Chan*, void*, long, ulong);
Block*	consbread(Chan*, long, ulong);
long	conswrite(Chan*, void*, long, ulong);
long	consbwrite(Chan*, Block*, ulong);
void	consremove(Chan*);
void	conswstat(Chan*, char*);
void	drawreset(void);
void	drawinit(void);
Chan*	drawattach(char*);
Chan*	drawclone(Chan*, Chan*);
int	drawwalk(Chan*, char*);
void	drawstat(Chan*, char*);
Chan*	drawopen(Chan*, int);
void	drawcreate(Chan*, char*, int, ulong);
void	drawclose(Chan*);
long	drawread(Chan*, void*, long, ulong);
Block*	drawbread(Chan*, long, ulong);
long	drawwrite(Chan*, void*, long, ulong);
long	drawbwrite(Chan*, Block*, ulong);
void	drawremove(Chan*);
void	drawwstat(Chan*, char*);
void	etherreset(void);
void	etherinit(void);
Chan*	etherattach(char*);
Chan*	etherclone(Chan*, Chan*);
int	etherwalk(Chan*, char*);
void	etherstat(Chan*, char*);
Chan*	etheropen(Chan*, int);
void	ethercreate(Chan*, char*, int, ulong);
void	etherclose(Chan*);
long	etherread(Chan*, void*, long, ulong);
Block*	etherbread(Chan*, long, ulong);
long	etherwrite(Chan*, void*, long, ulong);
long	etherbwrite(Chan*, Block*, ulong);
void	etherremove(Chan*);
void	etherwstat(Chan*, char*);
void	i82365reset(void);
void	i82365init(void);
Chan*	i82365attach(char*);
Chan*	i82365clone(Chan*, Chan*);
int	i82365walk(Chan*, char*);
void	i82365stat(Chan*, char*);
Chan*	i82365open(Chan*, int);
void	i82365create(Chan*, char*, int, ulong);
void	i82365close(Chan*);
long	i82365read(Chan*, void*, long, ulong);
Block*	i82365bread(Chan*, long, ulong);
long	i82365write(Chan*, void*, long, ulong);
long	i82365bwrite(Chan*, Block*, ulong);
void	i82365remove(Chan*);
void	i82365wstat(Chan*, char*);
void	ipreset(void);
void	ipinit(void);
Chan*	ipattach(char*);
Chan*	ipclone(Chan*, Chan*);
int	ipwalk(Chan*, char*);
void	ipstat(Chan*, char*);
Chan*	ipopen(Chan*, int);
void	ipcreate(Chan*, char*, int, ulong);
void	ipclose(Chan*);
long	ipread(Chan*, void*, long, ulong);
Block*	ipbread(Chan*, long, ulong);
long	ipwrite(Chan*, void*, long, ulong);
long	ipbwrite(Chan*, Block*, ulong);
void	ipremove(Chan*);
void	ipwstat(Chan*, char*);
void	mntreset(void);
void	mntinit(void);
Chan*	mntattach(char*);
Chan*	mntclone(Chan*, Chan*);
int	mntwalk(Chan*, char*);
void	mntstat(Chan*, char*);
Chan*	mntopen(Chan*, int);
void	mntcreate(Chan*, char*, int, ulong);
void	mntclose(Chan*);
long	mntread(Chan*, void*, long, ulong);
Block*	mntbread(Chan*, long, ulong);
long	mntwrite(Chan*, void*, long, ulong);
long	mntbwrite(Chan*, Block*, ulong);
void	mntremove(Chan*);
void	mntwstat(Chan*, char*);
void	progreset(void);
void	proginit(void);
Chan*	progattach(char*);
Chan*	progclone(Chan*, Chan*);
int	progwalk(Chan*, char*);
void	progstat(Chan*, char*);
Chan*	progopen(Chan*, int);
void	progcreate(Chan*, char*, int, ulong);
void	progclose(Chan*);
long	progread(Chan*, void*, long, ulong);
Block*	progbread(Chan*, long, ulong);
long	progwrite(Chan*, void*, long, ulong);
long	progbwrite(Chan*, Block*, ulong);
void	progremove(Chan*);
void	progwstat(Chan*, char*);
void	rootreset(void);
void	rootinit(void);
Chan*	rootattach(char*);
Chan*	rootclone(Chan*, Chan*);
int	rootwalk(Chan*, char*);
void	rootstat(Chan*, char*);
Chan*	rootopen(Chan*, int);
void	rootcreate(Chan*, char*, int, ulong);
void	rootclose(Chan*);
long	rootread(Chan*, void*, long, ulong);
Block*	rootbread(Chan*, long, ulong);
long	rootwrite(Chan*, void*, long, ulong);
long	rootbwrite(Chan*, Block*, ulong);
void	rootremove(Chan*);
void	rootwstat(Chan*, char*);
void	srvreset(void);
void	srvinit(void);
Chan*	srvattach(char*);
Chan*	srvclone(Chan*, Chan*);
int	srvwalk(Chan*, char*);
void	srvstat(Chan*, char*);
Chan*	srvopen(Chan*, int);
void	srvcreate(Chan*, char*, int, ulong);
void	srvclose(Chan*);
long	srvread(Chan*, void*, long, ulong);
Block*	srvbread(Chan*, long, ulong);
long	srvwrite(Chan*, void*, long, ulong);
long	srvbwrite(Chan*, Block*, ulong);
void	srvremove(Chan*);
void	srvwstat(Chan*, char*);
void	sslreset(void);
void	sslinit(void);
Chan*	sslattach(char*);
Chan*	sslclone(Chan*, Chan*);
int	sslwalk(Chan*, char*);
void	sslstat(Chan*, char*);
Chan*	sslopen(Chan*, int);
void	sslcreate(Chan*, char*, int, ulong);
void	sslclose(Chan*);
long	sslread(Chan*, void*, long, ulong);
Block*	sslbread(Chan*, long, ulong);
long	sslwrite(Chan*, void*, long, ulong);
long	sslbwrite(Chan*, Block*, ulong);
void	sslremove(Chan*);
void	sslwstat(Chan*, char*);
void	tinyfsreset(void);
void	tinyfsinit(void);
Chan*	tinyfsattach(char*);
Chan*	tinyfsclone(Chan*, Chan*);
int	tinyfswalk(Chan*, char*);
void	tinyfsstat(Chan*, char*);
Chan*	tinyfsopen(Chan*, int);
void	tinyfscreate(Chan*, char*, int, ulong);
void	tinyfsclose(Chan*);
long	tinyfsread(Chan*, void*, long, ulong);
Block*	tinyfsbread(Chan*, long, ulong);
long	tinyfswrite(Chan*, void*, long, ulong);
long	tinyfsbwrite(Chan*, Block*, ulong);
void	tinyfsremove(Chan*);
void	tinyfswstat(Chan*, char*);