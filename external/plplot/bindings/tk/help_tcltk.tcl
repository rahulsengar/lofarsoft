# $Id: help_tcltk.tcl 2351 2001-01-02 03:04:28Z mlebrun $
#----------------------------------------------------------------------------
# help_tcltk w
#
# Create a top-level window that displays info for On Tcl/TK.. help item.
# Maurice LeBrun, IFS
# Adapted from mkStyles.tcl from widget demo.
#
# Arguments:
#    w -	Name to use for new top-level window.
#----------------------------------------------------------------------------

proc help_tcltk {{w .help_tcltk}} {
    catch {destroy $w}
    toplevel $w
    dpos $w
    wm title $w "Help on Tcl/TK"
    wm iconname $w "help_tcltk"
    normal_text_setup $w 64

    insertWithTags $w.t {\
Tcl/TK (written by John Ousterhout, UC Berkeley) is rapidly gaining in
popularity, and already has a world-wide following.  Software developers
usually prefer it to Motif for building user interfaces due to the higher
level of abstraction employed by Tcl/TK.  Widgets can be easily created,
destroyed, and manipulated via simple Tcl expressions.  Further, Tcl/TK is
non-proprietary, so you can in principle run it on any X-based platform,
as well as have unrestricted access to the source code.  The package is
well documented, with a book on Tcl/TK due out in late 1993 (draft copies
are available via anonymous ftp), numerous man pages, and a Usenet
newsgroup comp.lang.tcl for discussion of all aspects of using Tcl/TK.

Some selected excerpts from the Tcl/TK "Frequently Asked Questions" list
follow:

---------------------------------------------------------

From: FAQ General information
Subject: -II- What is Tcl?  Tk?  Extended Tcl? 

    Tcl (Version 6.7) stands for ``tool command language'' and is
pronounced ``tickle.'' It is actually two things: a language and a
library.  First, Tcl is a simple textual language, intended primarily for
issuing commands to interactive programs such as text editors, debuggers,
illustrators, and shells.  It has a simple syntax and is also
programmable, so Tcl users can write command procedures to provide more
powerful commands than those in the built-in set.

    Second, Tcl is a library package that can be embedded in application
programs.  The Tcl library consists of a parser for the Tcl language,
routines to implement the Tcl built-in commands, and procedures that allow
each application to extend Tcl with additional commands specific to that
application.  The application program generates Tcl commands and passes
them to the Tcl parser for execution.  Commands may be generated by
reading characters from an input source, or by associating command strings
with elements of the application's user interface, such as menu entries,
buttons, or keystrokes.  When the Tcl library receives commands it parses
them into component fields and executes built-in commands directly.  For
commands implemented by the application, Tcl calls back to the application
to execute the commands.  In many cases commands will invoke recursive
invocations of the Tcl interpreter by passing in additional strings to
execute (procedures, looping commands, and conditional commands all work
in this way).

    An application program gains three advantages by using Tcl for its
command language.  First, Tcl provides a standard syntax: once users know
Tcl, they will be able to issue commands easily to any Tcl-based
application.  Second, Tcl provides programmability.  All a Tcl application
needs to do is to implement a few application-specific low-level commands.
Tcl provides many utility commands plus a general programming interface
for building up complex command procedures.  By using Tcl, applications
need not re-implement these features.  Third, extensions to Tcl, such as
the Tk toolkit, provide mechanisms for communicating between applications
by sending Tcl commands back and forth.  The common Tcl language framework
makes it easier for applications to communicate with one another.

    Tk (Version 3.2) - an extension to Tcl which provides the programmer
with an interface to the X11 windowing system.  Note that many users will
encounter Tk via the ``wish'' command.  Wish is a simple windowing shell
which permits the user to write Tcl applications in a prototyping
environment.

    Extended Tcl (tclX) (Version 6.7c) - This extended set of commands are
provided by NeoSoft, and provide additional Unix system interfaces to the
Tcl programmer.  Many users will encounter Tcl via the ``tcl'' command -
an interactive Tcl command interpreter.

------------------------------

From: FAQ Tcl/Tk Package Catalog
Subject: -XI- Where can I get these packages?

The "home site" for Tcl on the Internet is sprite.berkeley.edu.
Sprite is an experimental research machine whose IP servers
occasionally flake out.  If you find that sprite is refusing
connections, send mail to "root@sprite.berkeley.edu", wait a few
hours, and try again.

Tcl and Extended Tcl were posted to comp.sources.misc, appearing
In volume 25 and then Tcl appeared again in volume 26 and can be found at 
most comp.sources.misc archive sites in the tcl and tclx directories.
These are quite old versions actually, but are still usable.

a. Tcl	- available on sprite.berkeley.edu and harbor.ecn.purdue.edu
b. Tk	- available on sprite.berkeley.edu and harbor.ecn.purdue.edu
c. Extended Tcl - available on sprite.berkeley.edu and harbor.ecn.purdue.edu

The IP address for harbor.ecn.purdue.edu is 128.46.128.76 .
sprite.berkeley.edu is a CNAME for allspice.Berkeley.EDU, whose
IP address is 128.32.150.27.

Other sites which either mirror the Tcl/Tk submissions or have special
versions are:

ftp.uu.net:/languages/tcl		<- sprite only
gatekeeper.dec.com:/.0/BSD/UCB/sprite 	<- sprite only
sunsite.unc.edu:/pub/languages/tcl
ftp.ibp.fr:/pub/tcl			<- distrib, contrib, expect
syd.dit.csiro.au:/pub/tk		<- contrib and sprite

iskut.ucs.ubc.ca:/pub/X11/tcl
ftp.funet.fi:/pub/languages/tcl
coma.cs.tu-berlin.ed:/pub/tcl
nic.funet.fi:/pub/languages/tcl
oskgate0.mei.co.jp:/free/X/toolkits/tcl <- sprite only

sunsite.unc.edu:/pub/Linux	<- contains port of extended Tcl and 
					extended Tk to this OS.

--------------------------------------

From: -XI- Where can I get these packages?
Subject: -A- Retrieving Tcl and Tk


>From: ouster@sprite.Berkeley.EDU (John Ousterhout)
>Newsgroups: comp.lang.tcl
>Subject: Obtaining Tcl/Tk sources

For people new to the Tcl/Tk community, here is information on how
to obtain Tcl and Tk sources.  The information below describes what
I distribute; other information is available from other machines
also, such as harbor.ecn.purdue.edu.

The sources and documentation for the Tcl command
language library, for the Tk toolkit, and for a few Tcl-based
applications, are in the public FTP area on sprite.berkeley.edu.
All of these files are in the "tcl" subdirectory of the FTP area.
Here is a catalog of what's available.  Most of the files are
compressed tar files ("xxx.tar.Z").  There is some overlap
between the contents of the various packages.

tk3.2.tar.Z -		This is the latest release of the Tk toolkit, released
			in February 1993.  It includes a complete copy of the
			Tcl V6.7 release (the version of Tcl with which it is
			compatible) plus a simple windowing shell called
			"wish".  If you retrieve this file you don't need to
			retrieve Tcl separately.

tcl6.7.tar.Z -		This is the newest release of the Tcl library.
			It became available in February 1993.  This package
			includes only the Tcl library and its documentation,
			plus a simple main program for testing.

tclX6.5c.tar.Z -		Extended Tcl (or NeoSoft Tcl), created by Mark
			Diekhans and Karl Lehenbauer, which adds a number
			of useful facilities to the base Tcl release.
			Among the things in Extended Tcl are a Tcl shell,
			many new commands for things like UNIX kernel
			call access and math library routines, and an
			on-line help facility.  This file is based on
			Tcl 6.5 and also works with Tcl 6.6, 6.7 and Tk 3.1,
			Tk 3.2

			Note that tclX6.5c-6.7c.update.shar.Z exists on 
			the purdue archive site.

book.p1.ps.Z		Compressed Postscript for a draft of the first part
			of an upcoming book on Tcl and Tk to be published in
			1993 by Addison-Wesley.  This part of the book
			describes the Tcl language and how to write scripts
			in it.  About 130 pages in length.

book.p2.ps.Z		Compressed Postscript for a draft of the second part
			of an upcoming book on Tcl and Tk to be published in
			1993 by Addison-Wesley.  This part of the book
			describes the Tk extension and how to write scripts
			in it.  About 130 pages in length.

tclUsenix90.ps -		Postscript for a paper on Tcl that appeared in the
			Winter 1990 USENIX Conference.  This paper is also
			included in the Tcl and Tk distributions.

tkUsenix91.ps -		Postscript for a paper on Tk that appeared in the
			Winter 1991 USENIX Conference.  This paper is also
			included in the Tk distribution.

tkF10.ps -		Postscript for Figure 10 of the Tk paper.

tut.tar.Z -		A collection of materials from a full-day tutorial
			on Tcl and Tk.  Includes viewgraphs from five one-hour
			talks plus a sample widget.

In addition, there may be older releases of some or all of the above
files;  look for files with earlier release numbers.


To retrieve any or all of these packages, use anonymous FTP to
sprite.berkeley.edu (Internet address 128.32.150.27).  Use user
"anonymous"; when asked for a password, type your login name.  Then
retrieve the relevant file(s) with the commands like the following:
		type image (try "type binary" if this command is rejected)
		cd tcl
		get tk3.2.tar.Z

Be sure to retrieve files in image mode (type "type image" to FTP)
in order to make sure that you don't lose bits.

Any file with a .Z extension is a compressed file, which means you must
use the "uncompress" program to get back a normal file.  For example, for
the file tk3.2.tar.Z, you should type

	uncompress tk3.2.tar.Z

once you've retrieved the file.  This will produce a file named "tk3.2.tar".
Then you will need to use tar to extract the members.  Typically one
would use a command such as:

	tar xv tk3.2.tar

to extract the pieces.

Each of the releases has a README file in the top-level directory that
describes how to compile the release, where to find documentation, etc.

Questions or problems about any of these distributions should be directed
to "John Ousterhout" <ouster@cs.berkeley.edu>.

If you don't have access to Sprite, you can also retrieve some or
all of the above files from other FTP repositories.  Here is a
sampler of machines that store some or all of the Tcl/Tk information,
plus the directories in which to check:

ftp.uu.net:		/languages/tcl/*
export.lcs.mit.edu:		/contrib/tk*
harbor.ecn.purdue.edu:	/pub/tcl/*

--------------------------------------

From: -XI- Where can I get these packages?
Subject: -B- Accessing the Tcl/Tk User Contributions Archive

Contributions to the Tcl/Tk Contrib Archive are most welcome --
please upload them to:

	harbor.ecn.purdue.edu:/incoming	[128.46.128.76]

send the archive maintainer <tcl-archive@harbor.ecn.purdue.edu> a note stating
the names of the files you uploaded and a brief description for the
index.

Harbor is the central file server for the software staff of the
Engineering Computer Network, so please try to refrain from FTPing
stuff between 9am and 5pm EST (GMT -0500).  No mail-archive service is
planned as yet -- users without FTP capability should use one of the
following mail-based FTP services (send mail to the appropriate address
with "help" in the body):

BITNET users:   BITFTP <bitftp@pucc.princeton.edu>
Others:         "DEC ftpmail" <ftpmail@decwrl.dec.com>
Europe:         ftpmail@grasp1.univ-lyon1.fr
		mail-server@ftp.tu-clausthal.de
} normal
    $w.t configure -state disabled
    $w.t mark set insert 0.0
}
