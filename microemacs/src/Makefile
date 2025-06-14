##-*- makefile -*-############################################################
#
# Copyright (c) 2005-2025 JASSPA (www.jasspa.com).
#
# This is part of JASSPA's MicroEmacs, see the LICENSE file for licensing and
# copying information.
#
# Synopsis:      Installation Makefile
# Authors:       Jon Green
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
		me.rc		ne.rc		memsvc20.rc	nec.rc		\
		nemsvc20.rc	ncmsvc20.rc

PLTHDR	=	winterm.h	wintermr.h

ICOSRC	=	ne_logo.ico	me_logo.ico	me_logo_nc.ico	\
		me.bmp		ne.bmp

MAKEXE	=	build
MAKSRC	=	build.bat	\
		aix4.gmk	aix4.mak	\
		aix5.gmk	aix5.mak	\
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
		mingw.gmk	mingw32.gmk	\
		openbsd.gmk	\
		openstep.mak	\
		osf1.gmk	\
		sunos5.gmk	sunos5.mak	\
		suncsw.mak	suncsw86.mak	\
		sunosx86.gmk	sunosx86.mak	\
		win32b55.mak	win32bc.mak	\
		win32sv2.mak	win32v2.mak	\
		win32sv4.mak	\
		win32v5.mak	\
		win32v6.mak	win32v8.mak	\
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
RELMAKSRC	=	$(MAKSRC:%=$(SRCRELDIR)/%)
RELMAKEXE	=	$(MAKEXE:%=$(SRCRELDIR)/%)
RELWIN32S	=	$(WIN32S:%=$(SRCRELDIR)/%)

RELEASES	=	$(RELSTDHDR)	$(RELSTDDEF)	$(RELSTDSRC)	\
			$(RELPLTSRC)	$(RELPLTHDR)	$(RELICOSRC)	\
			$(RELMAKSRC)	$(RELWIN32S)	$(RELMAKEXE)
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
	$(RM) *.gz
	$(RM) tags
	$(RM) *~
