##############################################################################
# This is part of the JASSPA MicroEmacs.
# Copyright (C) 2004 JASSPA (www.jasspa.com)
# See the file COPYING for copying and conditions.
#
# Created:       Thu Feb 5 21:36:24 2004
# Synopsis:      Installation Makefile
# Authors:       Jon Green
# Last Modified: <20041128.2247>
#
##############################################################################

TOPDIR=..
include $(TOPDIR)/etc/makeinc
#
# Source files
#
STDHDR	=	ebind.h		edef.h		eextrn.h	efunc.h		\
		emain.h		emode.h		eprint.h	esearch.h	\
		eskeys.h	estruct.h	eterm.h		evar.h		\
		evers.h		eopt.h

STDDEF	=	ebind.def	efunc.def	eprint.def	evar.def	\
		etermcap.def	emode.def	eskeys.def

STDSRC	=	abbrev.c	basic.c		bind.c		buffer.c	\
		crypt.c		dirlist.c	display.c	eval.c		\
		exec.c		file.c		fileio.c	frame.c		\
		hilight.c	history.c	input.c		isearch.c	\
		key.c		line.c		macro.c		main.c		\
		narrow.c	next.c		osd.c		print.c		\
		random.c	regex.c		region.c	registry.c	\
		search.c	spawn.c		spell.c		tag.c		\
		termio.c	time.c		undo.c		window.c	\
		word.c

PLTSRC	=	unixterm.c	dosterm.c	winterm.c	winprint.c	\
		me.rc		ne.rc		memsvc20.rc	nec.rc

PLTHDR	=	winterm.h	wintermr.h

ICOSRC	=	icon_ne.ico	icon_me.ico	me32.bmp	ne32.bmp

TXTSRC	=	cygwin.txt	infolist.txt

MAKEXE	=	build
MAKSRC	=	build.bat	\
		aix43.gmk	aix43.mak	\
		cygwin.gmk	\
		darwin.gmk	\
		dosdj1.mak	dosdj2.mak	\
		freebsd.gmk	freebsd.mak	\
		hpux10.gmk	hpux10.mak	\
		hpux11.gmk	hpux11.mak	\
		hpux9.gmk	hpux9.mak	\
		irix5.gmk	irix5.mak	\
		irix6.gmk	irix6.mak	\
		linux2.gmk	linux26.gmk	\
		mingw.gmk	\
		openbsd.gmk	\
		openstep.mak	\
		osf1.gmk	\
		sunos55.gmk	sunos55.mak	\
		sunos56.gmk	sunos56.mak	\
		sunosx86.gmk	sunosx86.mak	\
		win32b55.mak	win32bc.mak	\
		win32sv2.mak	win32v2.mak	\
		win32sv4.mak	\
		win32v5.mak	\
		win32v6.mak	\
		zaurus.gmk

WIN32S=		win32s/win16.mak		win32s/w32sut32.lib		\
		win32s/w32sut16.lib		win32s/w32sut.h			\
		win32s/readme.txt		win32s/methnk16.dll		\
		win32s/methnk16.def		win32s/methnk16.c
#
# Released files
#
RELSTDHDR	=	$(STDHDR:%=$(SRCRELDIR)/%)
RELSTDDEF	=	$(STDDEF:%=$(SRCRELDIR)/%)
RELSTDSRC	=	$(STDSRC:%=$(SRCRELDIR)/%)
RELPLTSRC	=	$(PLTSRC:%=$(SRCRELDIR)/%)
RELPLTHDR	=	$(PLTHDR:%=$(SRCRELDIR)/%)
RELICOSRC	=	$(ICOSRC:%=$(SRCRELDIR)/%)
RELTXTSRC	=	$(TXTSRC:%=$(MERELDIR)/%)
RELMAKSRC	=	$(MAKSRC:%=$(SRCRELDIR)/%)
RELMAKEXE	=	$(MAKEXE:%=$(SRCRELDIR)/%)
RELWIN32S	=	$(WIN32S:%=$(SRCRELDIR)/%)

RELEASES	=	$(RELSTDHDR)	$(RELSTDDEF)	$(RELSTDSRC)	\
			$(RELPLTSRC)	$(RELPLTHDR)	$(RELICOSRC)	\
			$(RELTXTSRC)	$(RELMAKSRC)	$(RELWIN32S)	\
			$(RELMAKEXE)
#
# Local rules.
#
# Build the local platform
#
all:	build
	./build
#
# Build Release
#
release:	$(SRCRELDIR)	$(RELEASES)

# Release target files
$(SRCRELDIR):
	$(MKDIR) $@
$(RELSTDHDR): $(STDHDR)
	$(INSTALL_FIL) $(@F) $(@D)
$(RELSTDDEF): $(STDDEF)
	$(INSTALL_FIL) $(@F) $(@D)
$(RELSTDSRC): $(STDSRC)
	$(INSTALL_FIL) $(@F) $(@D)
$(RELPLTSRC): $(PLTSRC)
	$(INSTALL_FIL) $(@F) $(@D)
$(RELPLTHDR): $(PLTHDR)
	$(INSTALL_FIL) $(@F) $(@D)
$(RELICOSRC): $(ICOSRC)
	$(INSTALL_FIL) $(@F) $(@D)
$(RELTXTSRC): $(TXTSRC)
	$(INSTALL_FIL) $(@F) $(@D)
$(RELMAKSRC): $(MAKSRC)
	$(INSTALL_FIL) $(@F) $(@D)
$(RELMAKEXE): $(MAKEXE)
	$(INSTALL_EXE) $(@F) $(@D)
$(RELWIN32S): $(WIN32S)
	$(MKDIR) $(@D)
	$(INSTALL_FIL) win32s/$(@F) $(@D)
#
# Clean area.
#
clean:
	$(RM) build
	sh ./build.sh -C
#
# Remove all working files.
#
spotless: clean
	sh ./build.sh -S
	$(RM) tags
	$(RM) *~
