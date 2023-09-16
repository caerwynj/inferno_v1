implement Init;
#
# init program for standalone wm using TK
#
include "sys.m";
sys: Sys;
FD, Connection, sprint, Dir: import sys;
print, fprint, open, bind, mount, dial, sleep, read: import sys;

include "draw.m";
draw: Draw;
Context: import draw;

include "keyring.m";
kr: Keyring;

Init: module
{
	init:	fn();
};

Logon: module
{
	init:	fn(ctxt: ref Draw->Context, argv: list of string);
};

rootfs(server: string): int
{
	ok, n: int;
	c: Connection;

	(ok, c) = dial("tcp!" + server + "!6666", nil);
	if(ok < 0)
		return -1;

	sys->print("Connected ...");
	if(kr != nil){
		sys->print("Authenticate ...");
		ai := kr->readauthinfo("/nvfs/default");
		if(ai == nil)
			print("readauthinfo failed: %r\n");
		(id_or_err, secret) := kr->auth(c.dfd, ai, 0);
		if(secret == nil)
			print("authentication failed: %s\n", id_or_err);
		else {
			# no line encryption
			algbuf := array of byte "none";
			kr->sendmsg(c.dfd, algbuf, len algbuf);
		}
	}

	sys->print("mount ...");

	c.cfd = nil;
	n = mount(c.dfd, "/", sys->MREPL, "");
	if(n > 0)
		return 0;
	return -1;
}

Bootpreadlen: con 128;

init()
{
	spec: string;

	sys = load Sys Sys->PATH;
	kr = load Keyring Keyring->PATH;

	sys->print("**\n** Inferno\n** Lucent Technologies\n**\n");

	sys->print("Setup boot net services ...\n");
	
	#
	# Setup what we need to call a server and
	# Authenticate
	#
	bind("#l", "/net", sys->MREPL);
	bind("#I", "/net", sys->MAFTER);
	bind("#c", "/dev", sys->MAFTER);
	nvramfd := sys->open("#H/hd0nvram", sys->ORDWR);
	if(nvramfd != nil){
		spec = sys->sprint("#F%d", nvramfd.fd);
		if(bind(spec, "/nvfs", sys->MAFTER) < 0)
			print("init: bind %s: %r\n", spec);
	}

	setsysname();

	sys->print("bootp ...");

	fd := open("/net/ipifc", sys->OWRITE);
	if(fd == nil) {
		print("init: open /net/ipifc: %r");
		exit;
	}
	fprint(fd, "bootp /net/ether0");

	fd = open("/net/bootp", sys->OREAD);
	if(fd == nil) {
		print("init: open /net/bootp: %r");
		exit;
	}

	buf := array[Bootpreadlen] of byte;
	nr := read(fd, buf, len buf);
	fd = nil;
	if(nr <= 0) {
		print("init: read /net/bootp: %r");
		exit;
	}

	(ntok, ls) := sys->tokenize(string buf, " \t\n");
	while(ls != nil) {
		if(hd ls == "fsip"){
			ls = tl ls;
			break;
		}
		ls = tl ls;
	}
	if(ls == nil) {
		print("init: server address not in bootp read");
		exit;
	}

	srv := hd ls;
	sys->print("server %s\nConnect ...\n", srv);

	while(rootfs(srv) < 0)
		sleep(1000);

	sys->print("done\n");

	#
	# default namespace
	#
	sys->unmount(nil, "/dev");
	bind("#c", "/dev", sys->MBEFORE);			# console
	if(spec != nil)
		bind(spec, "/nvfs", sys->MBEFORE|sys->MCREATE);	# our keys
	bind("#E", "/dev", sys->MBEFORE);		# mpeg
	bind("#l", "/net", sys->MBEFORE);		# ethernet
	bind("#I", "/net", sys->MBEFORE);		# TCP/IP
	bind("#V", "/dev", sys->MAFTER);		# hauppauge TV
	bind("#p", "/prog", sys->MREPL);		# prog device

	sys->print("clock...\n");
	setclock();

	sys->print("logon...\n");

	logon := load Logon "/dis/wm/logon.dis";
	if(logon == nil) {
		print("init: load /dis/wm/logon.dis: %r");
		exit;
	}
	dc: ref Context;
	spawn logon->init(dc, nil);
}

setclock()
{
	(ok, dir) := sys->stat("/");
	if (ok < 0) {
		print("init: stat /: %r");
		return;
	}

	fd := sys->open("/dev/time", sys->OWRITE);
	if (fd == nil) {
		print("init: open /dev/time: %r");
		return;
	}

	# Time is kept as microsecs, atime is in secs
	b := array of byte sprint("%d000000", dir.atime);
	if (sys->write(fd, b, len b) != len b)
		print("init: write /dev/time: %r");
}

#
# Set system name from nvram
#
setsysname()
{
	fd := open("/nvfs/ID", sys->OREAD);
	if(fd == nil)
		return;
	fds := open("/dev/sysname", sys->OWRITE);
	if(fds == nil)
		return;
	buf := array[128] of byte;
	nr := sys->read(fd, buf, len buf);
	if(nr <= 0)
		return;
	sys->write(fds, buf, nr);
}
