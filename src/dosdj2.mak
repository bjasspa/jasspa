# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# dosdj2.mak - Make file for Dos using djgpp v2.0
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
# Synopsis:    Make file for Dos using djgpp v2.0
# Notes:
#	DJDIR must be setup to point to your DJGPP installation (normally
#		done in djgpp.env)
#
#	Run "make -f dosdj2.mak"      for optimised build produces ./me.exe
#	Run "make -f dosdj2.mak med"  for debug build produces     ./med.386
#
#	Run "make -f dosdj2.mak clean"      to clean source directory
#	Run "make -f dosdj2.mak spotless"   to clean source directory even more
#
##############################################################################
#
# Installation Directory
INSTDIR		= c:\emacs
INSTPROGFLAGS	= 
#
# Local Definitions
COFF          = coff2exe
RM            = del
CC            = gcc
LD            = $(CC)
STRIP         =	strip
CDEBUG        =	-g -Wall
COPTIMISE     =	-O2 -DNDEBUG=1 -Wall
CDEFS         = -D_DOS -D__DJGPP2__ -I.
LIBS          = -lpc
LDFLAGS       = 
INSTALL       =	copy
#
# Rules
.SUFFIXES: .c .o .od

.c.o:	
	$(CC) $(COPTIMISE) $(CDEFS) -c $<

.c.od:	
	$(CC) $(CDEBUG) $(CDEFS) -o $*.od -c $<
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
PLTSRC  = dosterm.c
#
# Object files
STDOBJ	= $(STDSRC:.c=.o)	
PLTOBJ  = $(PLTSRC:.c=.o)
OBJ	= $(STDOBJ) $(PLTOBJ)

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
	$(RM) me.exe
	$(RM) med.exe
	$(RM) me.386
	$(RM) med.386
	$(RM) *.o
	$(RM) *.od

spotless: clean
	$(RM) tags
	$(RM) *~

me:	me.exe

me.exe:	$(OBJ)
	$(RM) me.386
	$(RM) me.exe
	$(LD) $(LDFLAGS) -o me.386 $(OBJ) $(LIBS)
	$(STRIP) me.386
	$(COFF) me.386

med:	med.exe

med.exe: $(DOBJ)
	$(RM) med.386
	$(RM) med.exe
	$(LD) $(LDFLAGS) -o med.386 $(DOBJ) $(LIBS)
	$(COFF) med.386
#
# Dependancies
$(STDOBJ): $(STDHDR)
$(PLTOBJ): $(STDHDR) $(PLTHDR)

$(DSTDOBJ): $(STDHDR)
$(DPLTOBJ): $(STDHDR) $(PLTHDR)

