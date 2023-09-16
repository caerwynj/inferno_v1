#
#
# Distributed Sports Program: Slides
#
# slideshow [ machA [ machC ] ]
#
# DESCRIPTION:
#
# Display a series of informative slides about Inferno and the DSP 
# application, then launch the DSP application.  The arguments are
# passed to the DSP application [rundemo]. The arguments specify on 
# which machines data files exist.  If not specified, the local 
# machine name is used.
#
# NOTES:
#
# Typically, slideshow is invoked by runprofile.dis, which checks 
# to see if a file called profile exists which specifies machines 
# to be used in the application.  If profile does not exist, the local
# machine name is used.

implement Slideshow;

#
# Libraries used by the code
#
include "sys.m";
 sys:Sys;

include "string.m";
  str: String;

include "draw.m";
 draw: Draw;
Point, Rect, Display, Screen, Image: import draw;

include "tk.m";
 tk:Tk;
 Toplevel:import tk;

t: ref Toplevel;

include "tklib.m";
 tklib:Tklib;

include "wmlib.m";
 wmlib: Wmlib;
#
# Declaration of modules
#
Wm: module {
  init: fn(c: ref Draw->Context, argv: list of string);
};

Slideshow: module {
  init: fn(ctxt: ref Draw->Context, argv: list of string);
};
#
# Window set-up TK script
#
my_config := array[] of {
    "label .c -bitmap @images/slide1.bit",
    "pack .Wm_t -fill x",
    "pack .c",
    "pack progagate . 0",
    "bind .c <Button-1> {send cmd %x %y}",
    "bind .c <Button-2> {send cmd %x %y}",
    "bind .c <Button-3> {send cmd %x %y}",
    "update",
};

NS: con 12; #num slides

init(ctxt: ref Draw->Context, argv: list of string) {

# Load libraries

    sys = load Sys Sys->PATH;
    draw = load Draw Draw->PATH;
    tk    = load Tk    Tk->PATH;	# Tk toolkit
    tklib = load Tklib Tklib->PATH;	# Tk libraries
    wmlib = load Wmlib Wmlib->PATH;	# Window Manager libraries
    str = load String String->PATH;	

# Initialize toolkit/wm

    tklib->init(ctxt);
    wmlib->init();

    sys->chdir("/demo/dsp");

# Do setup of window display and communication

    (px,py) := getpos(ctxt,660,570);
    ppos := sys->sprint(" -x %d -y %d ",px,py);
    t = tk->toplevel(ctxt.screen,ppos+" -borderwidth 2 - relief raised");
    m := wmlib->titlebar(t,"Distributed Sports Program",0);
    cmd := chan of string;
    tk->namechan(t,cmd,"cmd");
    tklib->tkcmds(t,my_config);

#Define rectangle for buttons
    bk := Rect((405,425),(505,470));
    nx := Rect((515,425),(615,470));
    
    cs := 1;		#current slide

# Wait until exit button
    for (;;) alt {
	menu := <- m => {
	    if (menu[0] == 'e')
		return;
 	    wmlib->titlectl(t,menu);
	}

# When mouse button pressed, check to see if it was on Next/Back buttons.
# Then, go to next or previous slide.

	func := <- cmd => {
	    (x,y) := (str->splitl(func, " "));
	    pt := Point(int x, int y);
	    ans := pt.in(bk);
	    if (ans == 1) {
		cs = goback(cs,argv);
	    } else {
	    	ans := pt.in(nx);
	   	 if (ans == 1) {
			cs = gonext(ctxt,cs,argv);
			if (cs < 0)
			    return;
	   	 } 
	    }
	}
    }
}
#
# Decrement cs (the current slide) and display the previous slide
#
goback(cs: int, argv:list of string): int {
    if (cs > 1 ) {
	cs = cs - 1;
    }
    newsl := "images/slide"+string cs+".bit";
    newcmd := ".c configure -bitmap @"+newsl;
    tk->cmd(t, newcmd);
    tk->cmd(t,"update");
    return cs;
}
#
# Increment cs (the current slide) and display the next slide,
# unless cs is the last slide in which case launch the DSP 
# application passing the arguments that were passed to slideshow.
#
gonext(ctxt: ref Draw->Context, cs: int,argv:list of string): int {
    if (cs == NS) {
	argv = tl argv;
	if (argv != nil) {
	    a := hd argv;
	    a = string a[0:3];
	    if (a == "-x ")
		argv = nil;
	}

# Load dsp application and run it

    	appl := "startdemo.dis";
    	dsappl := load Wm appl;
    	spawn dsappl->init(ctxt,"startdemo" :: argv);
	return(-1);
    }
    if (cs < NS) {
	cs = cs + 1;
    }
    newsl := "images/slide"+string cs+".bit";
    newcmd := ".c configure -bitmap @"+newsl;
    tk->cmd(t, newcmd);
    tk->cmd(t,"update");
    return cs;
}
#
# Get coordinates of the window of width w and height h, such that
# it would appear at the center of window manager's work area.  Return
# these coordinates as (px,py) pair.
#
getpos(ctxt:ref Draw->Context,w,h:int) : (int,int) {

	r := ctxt.display.image.r;
	x := r.max.x - r.min.x;
	y := r.max.y - r.min.y;
	dx := (x - w) / 2; if (dx < 0) dx = 0;
	dy := (y - h) / 2; if (dy < 0) dy = 0;
	return(dx,dy);
}
