implement WmTask;

include "sys.m";
	sys: Sys;
	Dir: import sys;

include "draw.m";
	draw: Draw;

include "tk.m";
	tk: Tk;
	Toplevel: import tk;

include "tklib.m";
	tklib: Tklib;

include	"wmlib.m";
	wmlib: Wmlib;

Prog: adt
{
	pid:	int;
	size:	int;
	state:	string;
	mod:	string;
};

WmTask: module
{
	init:	fn(ctxt: ref Draw->Context, argv: list of string);
};

Wm: module
{
	init:	fn(ctxt: ref Draw->Context, argv: list of string);
};

task_cfg := array[] of {
	"frame .fl",
	"scrollbar .fl.scroll -command {.fl.l yview}",
	"listbox .fl.l -width 35w -yscrollcommand {.fl.scroll set}",
	"frame .b",
	"button .b.ref -text Refresh -command {send cmd r}",
	"button .b.stack -text Debug -command {send cmd s}",
	"button .b.kill -text Kill -command {send cmd k}",
	"pack .b.ref .b.stack .b.kill"+
	" -expand 1 -side left -fill x -padx 4 -pady 4",
	"pack .fl.scroll -side right -fill y",
	"pack .fl.l -fill both -expand 1",
	"pack .Wm_t -fill x",
	"pack .fl -fill both -expand 1",
	"pack .b -fill x -expand 1",
	"pack propagate . 0",
};

init(ctxt: ref Draw->Context, argv: list of string)
{
	sys  = load Sys  Sys->PATH;
	draw = load Draw Draw->PATH;
	tk   = load Tk   Tk->PATH;
	wmlib= load Wmlib Wmlib->PATH;
	tklib= load Tklib Tklib->PATH;

	tklib->init(ctxt);
	wmlib->init();

	sysnam := sysname();

	tkargs := "";
	argv = tl argv;
	if(argv != nil) {
		tkargs = hd argv;
		argv = tl argv;
	}
	t := tk->toplevel(ctxt.screen, tkargs+" -borderwidth 2 -relief raised");
	if(t == nil)
		return;

	menubut := wmlib->titlebar(t, sysnam, Wmlib->Appl);

	cmd := chan of string;
	tk->namechan(t, cmd, "cmd");

	tklib->tkcmds(t, task_cfg);

	readprog(t);

	tk->cmd(t, "update");

	for(;;) alt {
	menu := <-menubut =>
		if(menu[0] == 'e')
			return;
		wmlib->titlectl(t, menu);
	bcmd := <-cmd =>
		case bcmd[0] {
		's' =>
			sel := tk->cmd(t, ".fl.l curselection");
			if(sel == "")
				break;
			pid := int tk->cmd(t, ".fl.l get "+sel);
			stk := load Wm "/dis/wm/deb.dis";
			if(stk == nil)
				break;
			spawn stk->init(ctxt, "wmdeb" :: wmlib->geom(t) :: "-p "+string pid :: nil);
			stk = nil;
		'k' =>
			sel := tk->cmd(t, ".fl.l curselection");
			if(sel == "")
				break;
			pid := int tk->cmd(t, ".fl.l get "+sel);
			cfile := "/prog/"+string pid+"/ctl";
			cfd := sys->open(cfile, sys->OWRITE);
			if(cfd != nil) {
				sys->fprint(cfd, "kill");
				cfd = nil;
				tk->cmd(t, ".fl.l delete "+sel);
				break;
			}
			tklib->notice(t, "Error opening ctl file\n"+
					 "System: "+sys->sprint("%r"));
		'r' =>
			tk->cmd(t, ".fl.l delete 0 end");
			readprog(t);
			tk->cmd(t, "update");
		}
	}
}

mkprog(file: string): ref Prog
{
	fd := sys->open("/prog/"+file+"/status", sys->OREAD);
	if(fd == nil)
		return nil;

	buf := array[256] of byte;
	n := sys->read(fd, buf, len buf);
	if(n <= 0)
		return nil;

	(v, l) := sys->tokenize(string buf[0:n], " ");
	if(v < 6)
		return nil;

	prg := ref Prog;
	prg.pid = int hd l;
	l = tl l;

	l = tl l;
	l = tl l;
	# eat blanks in user name
	while(len l > 3)
		l = tl l;
	prg.state = hd l;
	l = tl l;
	prg.size = int hd l;
	l = tl l;
	prg.mod = hd l;

	return prg;
}

readprog(t: ref Toplevel): array of ref Prog
{
	n: int;
	d := array[100] of Dir;

	fd := sys->open("/prog", sys->OREAD);
	if(fd == nil)
		return nil;

	prog := array[100] of ref Prog;
	for(;;) {
		n = sys->dirread(fd, d);
		if(n <= 0)
			break;
		v := 0;
		for(i := 0; i < n; i++) {
			p := mkprog(d[i].name);
			if(p != nil){
				prog[v++] = p;
				l := sys->sprint("%4d %3dK %-7s |%s", p.pid, p.size, p.state, p.mod);
				tk->cmd(t, ".fl.l insert end '"+l);
			}
		}
	}
	return prog;
}

sysname(): string
{
	fd := sys->open("#c/sysname", sys->OREAD);
	if(fd == nil)
		return "Anon";
	buf := array[128] of byte;
	n := sys->read(fd, buf, len buf);
	if(n < 0) 
		return "Anon";
	return string buf[0:n];
}
