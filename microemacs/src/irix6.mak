# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# irix6.mak - Make file for SGI Irix v6
#
# Copyright (C) 2001-2002 JASSPA (www.jasspa.com)
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 675 Mass Ave, Cambridge, MA 02139, USA.
#
##############################################################################
#
# Created:     Sat Jan 24 1998
# Synopsis:    Make file for SGI Irix v6
# Notes:
#	Run "make -f irix6.mak"      for optimised build produces ./me
#	Run "make -f irix6.mak med"  for debug build produces     ./med
#
#	Run "make -f irix6.mak clean"      to clean source directory
#	Run "make -f irix6.mak spotless"   to clean source directory even more
#
##############################################################################
#
# Installation Directory
INSTDIR	      = /usr/local/bin
INSTPROGFLAGS = -s -o -u root -g root -m 0775 -f $(INSTDIR)
#
# Local Definitions
RM            = rm -f
CC            = cc -n32 -mips3
LD            = $(CC)
STRIP         =	strip
CDEBUG        =	-g2
COPTIMISE     =	-O3 -DNDEBUG=1
CDEFS         = -D_IRIX6 -I.
LIBS          = -ltermcap
LDFLAGS       = 
INSTALL       =	install
XLIBS         = -lX11
# definition for the console only build
CCONSOLE      = -D_NO_XTERM
#
# Rules
.SUFFIXES: .c .o .on .od

.c.o:	
	$(CC) $(COPTIMISE) $(CDEFS) $(MAKECDEFS) -c $<

.c.on:
	$(CC) $(COPTIMISE) $(CDEFS) $(CCONSOLE) $(MAKECDEFS) -o $@ -c $<

.c.od:	
	$(CC) $(CDEBUG) $(CDEFS) $(MAKECDEFS) -o $*.od -c $<
#
# Source files
STDHDR	= ebind.h edef.h eextrn.h efunc.h emain.h emode.h eprint.h \
	  esearch.h eskeys.h estruct.h eterm.h evar.h evers.h \
	  ebind.def efunc.def eprint.def evar.def etermcap.def emode.def eskeys.def
STDSRC	= abbrev.c basic.c bind.c buffer.c crypt.c dirlist.c display.c \
	  eval.c exec.c file.c fileio.c hilight.c history.c input.c \
	  isearch.c key.c line.c macro.c main.c narrow.c next.c osd.c \
	  print.c random.c regex.c region.c registry.c search.c spawn.c \
	  spell.c tag.c termio.c time.c undo.c window.c word.c

PLTHDR  = 
PLTSRC  = unixterm.c
#
# Object files
STDOBJ	= $(STDSRC:.c=.o)	
PLTOBJ  = $(PLTSRC:.c=.o)
OBJ	= $(STDOBJ) $(PLTOBJ)

NSTDOBJ	= $(STDSRC:.c=.on)
NPLTOBJ = $(PLTSRC:.c=.on)
NOBJ	= $(NSTDOBJ) $(NPLTOBJ)

DSTDOBJ	= $(STDSRC:.c=.od)	
DPLTOBJ = $(PLTSRC:.c=.od)
DOBJ	= $(DSTDOBJ) $(DPLTOBJ)
#
# Targets
all: me

install: me
	$(INSTALL) $(INSTPROGFLAGS) me $(INSTDIR)
	@echo "install done"

clean:
	$(RM) me men med *.o *.on *.od core

spotless: clean
	$(RM) tags *~

me:	$(OBJ)
	$(RM) $@
	$(LD) $(LDFLAGS) -o $@ $(OBJ) $(XLIBS) $(LIBS)
	$(STRIP) $@

men:	$(NOBJ)
	$(RM) $@
	$(LD) $(LDFLAGS) -o $@ $(NOBJ) $(LIBS)

med:	$(DOBJ)
	$(RM) $@
	$(LD) $(LDFLAGS) -o $@ $(DOBJ) $(XLIBS) $(LIBS)
#
# Dependancies
$(STDOBJ): $(STDHDR)
$(PLTOBJ): $(STDHDR) $(PLTHDR)

$(NSTDOBJ): $(STDHDR)
$(NPLTOBJ): $(STDHDR) $(PLTHDR)

$(DSTDOBJ): $(STDHDR)
$(DPLTOBJ): $(STDHDR) $(PLTHDR)


