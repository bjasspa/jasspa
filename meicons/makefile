#_____________________________-!-Makefile-!-________________________________
#
# Make file for MicroEmacs icon file using the Microsoft MSCV 2.0/4.0
# development kit.
#
# Author      : Jon Green
# Created     : 22nd February 1997
# Last Edited : <991022.0041>
#
# History     : JG  220297 Original  Constructed from emacssrc
#
# Details     :
#               Build from the command line using nmake.
#
#               nmake              - builds locally.
#		nmake clean        - cleans emacs directory
#		nmake spotless     - cleans some more !!
#		nmake install	   - builds and copies to c:\emacs.
#____________________________________________________________________________
#
#
#
# TOOLS Environment - change to location of your tools.
#
# Microsoft MSCV 6.0 install directory
TOOLSDIR      = "c:\Program Files\Microsoft Visual Studio\VC98"
INCLUDE=	$(TOOLSDIR)\include
LIBRARY=	$(TOOLSDIR)\lib

#
# Destination directory.
#
BINDIR	=	c:\emacs
#
# Standard Compilation tools and flags
#
CC	=	cl
RC	=	rc
RM	=	erase
LD	=	link
INSTALL	=	copy
MKDIR	=	mkdir

# Standard File Extensions
I	=	-I
O	=	.o
EXE	=	.exe

# Extended build rules.
.SUFFIXES: .c .o
.c.o:
	$(CC) $(CDEFS) -c $< -Fo$*.o

#
# Standard include stuff
#
COPTIMISEFLAGS	=	-nologo -G5 -YX -GX -O2 -DNDEBUG=1

CDEFS           = 	-D_JASSPA_ME -D_WIN32 -DWIN32 -D_WINDOWS -D_MBCS -I. -I$(TOOLSDIR)\include
LDFLAGS		=	/SUBSYSTEM:windows /NOLOGO /INCREMENTAL:no /MACHINE:IX86 /PDB:NONE /LIBPATH:$(TOOLSDIR)\lib

SYSLIBS         =	libc.lib user32.lib gdi32.lib winspool.lib \
			comdlg32.lib advapi32.lib shell32.lib
#
# Define the files involved in the build.
#
MEICONS	=	meicons

HEADERS =	$(MEICONS).h
ICOSRC	=	$(MEICONS).c
ICOOBJ	=	$(MEICONS).o

ICONS	=	me32.ico	c.ico		cpp.ico		h.ico \
		def.ico		emc.ico		empty.ico	e_grey.ico \
		e_magent.ico	e_blue.ico	e_black.ico	e_cyan.ico \
		e_green.ico	e_red.ico	e_yellow.ico	ehf.ico \
		make.ico	doc.ico		txt.ico		nroff0.ico \
		nroff1.ico	nroff2.ico	nroff3.ico	nroff4.ico \
		nroff5.ico	nroff6.ico	nroff7.ico	nroff8.ico \
		nroff9.ico	nroffso.ico	nrofftni.ico	nroffnrs.ico \
		man.ico		erblue.ico	erblue2.ico	erbrown.ico \
		ercyan.ico	ergreen.ico	ergreen2.ico	eraqua.ico \
		erblack.ico	ergrey.ico	ermagent.ico	erred.ico \
		eryellow.ico	abr.ico		dic.ico		hash.ico \
		twiddle.ico	y.ico		l.ico		p.ico \
		etf.ico		eaf.ico		edf.ico		esf.ico	\
		emf.ico		awk.ico		i.ico		rc.ico \
		rul.ico		log.ico		err.ico		lbn.ico \
		lib.ico		htm.ico		htp.ico		sm.ico \
		pso.ico		asm.ico		erf.ico

#
# Standard build rules
#
all::	$(MEICONS)$(EXE)

$(MEICONS)$(EXE): $(ICOOBJ) $(MEICONS).res
		-$(RM) $@
		$(LD) $(LDFLAGS) /out:$@ $(ICOOBJ) $(MEICONS).res $(SYSLIBS)

$(ICOOBJ):	$(HEADERS)

$(MEICONS).res: $(MEICONS).rc $(ICONS)
		$(RC) -v -i . -i $(INCLUDE) \
			 -i $(TOOLSDIR)\mfc\include -fo $@ $*.rc
relink::
		$(RM)	$(MEICONS)$(EXE)
		$(MAKE) -f $(MAKEFILE) $(MFLAGS) $(MEICONS)$(EXE)

install:: $(MEICONS)$(EXE)
		$(INSTALL) $(MEICONS)$(EXE) $(BINDIR)
		@echo Install done.

# Using ms-dos "del". Dumb program so only one deletion item per command !!
spotless:: clean
		$(RM) *.??~
clean::
		- $(RM) *.pch
		- $(RM) *.o
		- $(RM) *.exe
		- $(RM) *.res
		- $(RM) *.vcp
