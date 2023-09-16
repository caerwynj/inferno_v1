implement DebSrc;

include "sys.m";
	sys: Sys;

include "draw.m";

include "string.m";
	str: String;

include "tk.m";
	tk: Tk;

include "tklib.m";
	tklib: Tklib;

include "wmlib.m";
	wmlib: Wmlib;

include "debug.m";
	debug: Debug;
	Sym, Src, Exp, Module: import debug;

include "wmdeb.m";

mods:		list of ref Mod;
tktop:		ref Tk->Toplevel;
screen:		ref Draw->Screen;
opendir =	".";
srcid:		int;

sblpath :=	array[] of
{
	("/dis/",	"/appl/cmd/"),
	("/dis/mux/",	"/appl/mux/"),
	("/dis/lib/",	"/appl/lib/"),
	("/dis/wm/",	"/appl/wm/"),
};

init(ascreen: ref Draw->Screen,
	atktop: ref Tk->Toplevel,
	asys: Sys,
	atk: Tk,
	atklib: Tklib,
	awmlib: Wmlib,
	astr: String,
	adebug: Debug)
{
	screen = ascreen;
	tktop = atktop;
	sys = asys;
	tk = atk;
	tklib = atklib;
	wmlib = awmlib;
	str = astr;
	debug = adebug;
}

#
# make a Mod with a text widget for source file src
#
loadsrc(src: string, addpath: int): ref Mod
{
	if(src == "")
		return nil;

	m : ref Mod = nil;
	for(ml := mods; ml != nil; ml = tl ml){
		m = hd ml;
		if(m.src == src || filesuffix(src, m.src))
			break;
	}

	if(ml == nil || m.tk == nil){
		if(ml == nil)
			m = ref Mod(src, nil, nil, nil, 0, 1);
		fd := sys->open(src, sys->OREAD);
		if(fd == nil)
			return nil;
		(dir, file) := str->splitr(src, "/");
		m.tk = ".t."+tklib->tkquote(file)+string srcid++;
		tkcmd("text "+m.tk+" -bd 0 -state disabled");
		tkcmd(m.tk+" configure -insertwidth 2");
		opack := packed;
		packm(m);
		if(!loadfile(m.tk, fd)){
			fd = nil;
			packm(opack);
			tkcmd("destroy "+m.tk);
			return nil;
		}
		fd = nil;
		tkcmd(m.tk+" tag configure bpt -foreground #c00");
		tkcmd(".m.file.menu add command -label "+src+" -command {send m open "+src+"}");
		if(ml == nil)
			mods = m :: mods;

		if(addpath)
			addsearch(dir);
	}
	return m;
}

addsearch(dir: string)
{
	for(i := 0; i < len searchpath; i++)
		if(searchpath[i] == dir)
			return;
	s := array[i+1] of string;
	s[0:] = searchpath;
	s[i] = dir;
	searchpath = s;
}

#
# bring up the widget for src, if it exists
#
showstrsrc(src: string)
{
	m : ref Mod = nil;
	for(ml := mods; ml != nil; ml = tl ml){
		m = hd ml;
		if(m.src == src)
			break;
	}
	if(ml == nil)
		return;

	packm(m);
}

#
# bring up the widget for module
# at position s
#
showmodsrc(m: ref Mod, s: ref Src)
{
	if(s == nil)
		return;

	src := s.start.file;
	if(src != s.stop.file)
		s.stop = s.start;

	if(m == nil || m.tk == nil || m.src != src){
		m1 := findsrc(src);
		if(m1 == nil)
			return;
		if(m1.dis == nil)
			m1.dis = m.dis;
		if(m1.sym == nil)
			m1.sym = m.sym;
		m = m1;
	}

	tkcmd(m.tk+" mark set insert "+string s.start.line+"."+string s.start.pos);
	tkcmd(m.tk+" tag remove sel 0.0 end");
	tkcmd(m.tk+" tag add sel insert "+string s.stop.line+"."+string s.stop.pos);
	tkcmd(m.tk+" see insert");

	packm(m);
}

packm(m: ref Mod)
{
	if(packed != m && packed != nil){
		tkcmd(packed.tk+" configure -yscrollcommand {}");
		tkcmd(".body.scy configure -command {}");
		tkcmd("pack forget "+packed.tk);
	}

	if(packed != m && m != nil){
		tkcmd(m.tk+" configure -yscrollcommand {.body.scy set}");
		tkcmd(".body.scy configure -command {"+m.tk+" yview}");
		tkcmd("pack "+m.tk+" -expand 1 -fill both -in .body.ft");
	}
	packed = m;
}

#
# find the dis file associated with m
# we know that m has a valid src
#
attachdis(m: ref Mod): int
{
	c := load Dis m.dis;
	if(c == nil){
		m.dis = repsuff(m.src, ".b", ".dis");
		c = load Dis m.dis;
	}
	if(c == nil && m.sym != nil){
		m.dis = repsuff(m.sym.path, ".sbl", ".dis");
		c = load Dis m.dis;
	}
	if(c == nil){
		(dir, file) := str->splitr(repsuff(m.src, ".b", ".dis"), "/");
		m.dis = wmlib->getfilename(screen, tktop, "Dis file", file, dir);
		c = load Dis m.dis;
	}
	return c != nil;
}

#
# load the symbol file for m
# works best if m has an associated source file
#
attachsym(m: ref Mod)
{
	if(m.sym != nil)
		return;
	sbl := repsuff(m.src, ".b", ".sbl");
	err : string;
	tk->cmd(tktop, "cursor -bitmap cursor.wait");
	(m.sym, err) = debug->sym(sbl);
	tk->cmd(tktop, "cursor -default");
	if(m.sym != nil)
		return;
	if(!str->prefix("Can't open", err)){
		alert(err);
		return;
	}
	(dir, file) := str->splitr(sbl, "/");
	sbl = wmlib->getfilename(screen, tktop, "Symbol file", file, dir);
	tk->cmd(tktop, "cursor -bitmap cursor.wait");
	(m.sym, err) = debug->sym(sbl);
	tk->cmd(tktop, "cursor -default");
	if(m.sym != nil)
		return;
	if(!str->prefix("Can't open", err)){
		alert(err);
		return;
	}
}

#
# get the current selection
#
getsel(): (ref Mod, int)
{
	m := packed;
	if(m == nil || m.src == nil)
		return (nil, 0);
	attachsym(m);
	if(m.sym == nil){
		alert("No symbol file for "+m.src);
		return (nil, 0);
	}
	index := tkcmd(m.tk+" index insert");
	if(len index == 0 || index[0] == '!')
		return (nil, 0);
	(sline, spos) := str->splitl(index, ".");
	line := int sline;
	pos := int spos[1:];
	pc := m.sym.srctopc(ref Src((m.src, line, pos), (m.src, line, pos)));
	s := m.sym.pctosrc(pc);
	if(s == nil){
		alert("No pc is appropriate");
		return (nil, 0);
	}
	return (m, pc);
}

#
# return the selected string
#
snarf(): string
{
	if(packed == nil)
		return "";
	s := tk->cmd(tktop, packed.tk+" get sel.first sel.last");
	if(tklib->is_err(s))
		s = "";
	return s;
}

#
# search for another occurance of s;
# return if s was found
#
search(s: string): int
{
	if(packed == nil || s == "")
		return 0;
	pos := " sel.last";
	sel := tk->cmd(tktop, packed.tk+" get sel.last");
	if(tklib->is_err(sel))
		pos = " insert";
	pos = tk->cmd(tktop, packed.tk+" search -- "+tklib->tkquote(s)+pos);
	if(tklib->is_err(pos) || pos == "")
		return 0;
	tkcmd(packed.tk+" mark set insert "+pos);
	tkcmd(packed.tk+" tag remove sel 0.0 end");
	tkcmd(packed.tk+" tag add sel insert "+pos+"+"+string len s+"c");
	tkcmd(packed.tk+" see insert");
	return 1;
}

#
# make a Mod for debugger module mod
#
findmod(mod: ref Module): ref Mod
{
	dis := mod.dis();
	if(dis == "")
		return nil;
	m: ref Mod;
	for(ml := mods; ml != nil; ml = tl ml){
		m = hd ml;
		if(m.dis == dis || filesuffix(dis, m.dis))
			break;
	}
	if(ml == nil){
		if(len dis > 0 && dis[0] != '$')
			m = findsrc(repsuff(dis, ".dis", ".b"));
		if(m == nil)
			mods = ref Mod("", "", dis, nil, 0, 0) :: mods;
	}
	if(m != nil){
		m.srcask = 0;
		m.dis = dis;
		if(m.symask){
			attachsym(m);
			m.symask = 0;
		}
		mod.addsym(m.sym);
	}
	return m;
}

findsrc(src: string): ref Mod
{
	m := loadsrc(src, 1);
	if(m != nil)
		return m;

	(dir, file) := str->splitr(src, "/");
	for(i := 0; i < len searchpath; i++){
		m = loadsrc(searchpath[i] + file, 0);
		if(m != nil)
			return m;
	}

	ns := len src;
	for(i = 0; i < len sblpath; i++){
		(disd, srcd) := sblpath[i];
		nd := len disd;
		if(ns > nd && src[:nd] == disd){
			m = loadsrc(srcd + src[nd:], 0);
			if(m != nil)
				return m;
		}
	}

	(dir, file) = str->splitr(src, "/");
	if(dir == "")
		dir = opendir;
	src = wmlib->getfilename(screen, tktop, "Limbo Source", file, dir);
	if(src == nil)
		return nil;
	(opendir, nil) = str->splitr(src, "/");
	if(opendir == "")
		opendir = ".";
	return loadsrc(src, 1);
}

#
# load the contents of fd into tkt
#
loadfile(tkt: string, fd: ref Sys->FD): int
{
	buf := array[512] of byte;
	i := 0;

	(ok, d) := sys->fstat(fd);
	if(ok < 0)
		return 0;
	tk->cmd(tktop, "cursor -bitmap cursor.wait");
	whole := array[d.length] of byte;
	for(;;){
		n := sys->read(fd, buf, len buf);
		if(n <= 0)
			break;
		j := i+n;
		if(j > d.length)
			break;
		whole[i:] = buf[:n];
		i += n;
	}
	tk->cmd(tktop, tkt+" delete 1.0 end;"+tkt+" insert end '"+string whole[:i]);
	tk->cmd(tktop, "update; cursor -default");
	return 1;
}

delmod(mods: list of ref Mod, m: ref Mod): list of ref Mod
{
	if(mods == nil)
		return nil;
	mh := hd mods;
	if(mh == m)
		return tl mods;
	return mh :: delmod(tl mods, m);
}

#
# replace an occurance in name of suffix old with new
#
repsuff(name, old, new: string): string
{
	no := len old;
	nn := len name;
	if(nn >= no && name[nn-no:] == old)
		return name[:nn-no] + new;
	return name;
}

filesuffix(suf, s: string): int
{
	nsuf := len suf;
	ns := len s;
	return ns > nsuf
		&& suf[0] != '/'
		&& s[ns-nsuf-1] == '/'
		&& s[ns-nsuf:] == suf;
}

alert(m: string)
{
	tklib->notice(tktop, m);
}

tkcmd(s: string): string
{
	return tk->cmd(tktop, s);
}
