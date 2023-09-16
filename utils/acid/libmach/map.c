/*
 * file map routines
 */
#include <lib9.h>
#include <bio.h>
#include "mach.h"

Map *
newmap(Map *map, int n)
{
	int size;

	size = sizeof(Map)+(n-1)*sizeof(struct segment);
	if (map == 0)
		map = malloc(size);
	else
		map = realloc(map, size);
	if (map == 0) {
		werrstr("out of memory: %r");
		return 0;
	}
	memset(map, 0, size);
	map->nsegs = n;
	return map;
}

int
setmap(Map *map, int fd, ulong b, ulong e, ulong f, char *name)
{
	int i;

	if (map == 0)
		return 0;
	for (i = 0; i < map->nsegs; i++)
		if (!map->seg[i].inuse)
			break;
	if (i >= map->nsegs)
		return 0;
	map->seg[i].b = b;
	map->seg[i].e = e;
	map->seg[i].f = f;
	map->seg[i].inuse = 1;
	map->seg[i].name = name;
	map->seg[i].fd = fd;
	return 1;
}

static ulong
stacktop(int pid)
{
	char buf[64];
	int fd;
	int n;
	char *cp;

	sprint(buf, "/proc/%d/segment", pid);
	fd = open(buf, 0);
	if (fd < 0)
		return 0;
	n = read(fd, buf, sizeof(buf)-1);
	close(fd);
	buf[n] = 0;
	if (strncmp(buf, "Stack", 5))
		return 0;
	for (cp = buf+5; *cp && *cp == ' '; cp++)
		;
	if (!*cp)
		return 0;
	cp = strchr(cp, ' ');
	if (!cp)
		return 0;
	while (*cp && *cp == ' ')
		cp++;
	if (!*cp)
		return 0;
	return strtoul(cp, 0, 16);
}

Map*
attachproc(int pid, int kflag, int corefd, Fhdr *fp)
{
	char buf[64], *regs;
	int fd;
	Map *map;
	ulong n;
	int mode;

	map = newmap(0, 4);
	if (!map)
		return 0;
	if(kflag) {
		regs = "kregs";
		mode = OREAD;
	} else {
		regs = "regs";
		mode = ORDWR;
	}
	if (mach->regsize) {
		sprint(buf, "/proc/%d/%s", pid, regs);
		fd = open(buf, mode);
		if(fd < 0) {
			free(map);
			return 0;
		}
		setmap(map, fd, 0, mach->regsize, 0, "regs");
	}
	if (mach->fpregsize) {
		sprint(buf, "/proc/%d/fpregs", pid);
		fd = open(buf, mode);
		if(fd < 0) {
			close(map->seg[0].fd);
			free(map);
			return 0;
		}
		setmap(map, fd, mach->regsize, mach->regsize+mach->fpregsize, 0, "fpregs");
	}
	setmap(map, corefd, fp->txtaddr, fp->txtaddr+fp->txtsz, fp->txtaddr, "text");
	if(kflag || (ulong) fp->dataddr >= 0x7fffffff) {
		setmap(map, corefd, fp->dataddr, 0xffffffff, fp->dataddr, "data");
		return map;
	}
	n = stacktop(pid);
	if (n == 0) {
		setmap(map, corefd, fp->dataddr, 0x7fffffff, fp->dataddr, "data");
		return map;
	}
	setmap(map, corefd, fp->dataddr, n, fp->dataddr, "data");
	return map;
}
	
int
findseg(Map *map, char *name)
{
	int i;

	if (!map)
		return -1;
	for (i = 0; i < map->nsegs; i++)
		if (map->seg[i].inuse && !strcmp(map->seg[i].name, name))
			return i;
	return -1;
}

void
unusemap(Map *map, int i)
{
	if (map != 0 && 0 <= i && i < map->nsegs)
		map->seg[i].inuse = 0;
}

Map*
attachremt(int fd, int rdebug, Fhdr *f)
{
	int i;
	Map *m;
	ulong txt;

	m = newmap(0, 3);
	if (m == 0)
		return 0;

	/* Space for mach structures */
	if(f->txtaddr > 8*4096)
		txt = f->txtaddr - 8*4096;

	setmap(m, fd, txt, f->txtaddr+f->txtsz, txt, "*text");
	setmap(m, fd, f->dataddr, 0xffffffff, f->dataddr, "*data");
	setmap(m, fd, 0x0, mach->regsize, 0, "kreg");

	if(rdebug)
		for(i = 0; i < m->nsegs; i++)
			m->seg[i].remote = 1;

	return m;
}

Map*
loadmap(Map *map, int fd, Fhdr *fp)
{
	map = newmap(map, 2);
	if (map == 0)
		return 0;

	map->seg[0].b = fp->txtaddr;
	map->seg[0].e = fp->txtaddr+fp->txtsz;
	map->seg[0].f = fp->txtoff;
	map->seg[0].fd = fd;
	map->seg[0].inuse = 1;
	map->seg[0].name = "text";
	map->seg[1].b = fp->dataddr;
	map->seg[1].e = fp->dataddr+fp->datsz;
	map->seg[1].f = fp->datoff;
	map->seg[1].fd = fd;
	map->seg[1].inuse = 1;
	map->seg[1].name = "data";
	return map;
}
