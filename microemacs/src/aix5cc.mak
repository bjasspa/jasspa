# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# aix5cc.mak - Make file for AIX V5
#
# Copyright (C) 2001-2009 JASSPA (www.jasspa.com)
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
# Synopsis:    Make file for AIX V5
# Notes:
#	Run "make -f aix5cc.mak"      for optimised build produces ./me
#	Run "make -f aix5cc.mak med"  for debug build produces     ./med
#
#	Run "make -f aix5cc.mak clean"      to clean source directory
#	Run "make -f aix5cc.mak spotless"   to clean source directory even more
#
##############################################################################
#
# Installation Directory
INSTDIR	      = /usr/local/bin
INSTPROGFLAGS = -s -o root -g root -m 0775
#
# Local Definitions
CP            = cp
MV            = mv
RM            = rm -f
CC            = cc
LD            = $(CC)
STRIP         =	strip
INSTALL       =	install
CDEBUG        =	-g
COPTIMISE     =	-O2 -DNDEBUG=1
CDEFS         = -D_AIX -D_AIX5 -qMAXMEM=8192 -I. -D_LARGEFILE_SOURCE -D_LARGE_FILES
CONSOLE_DEFS  = -D_ME_CONSOLE
WINDOW_DEFS   = $(MAKEWINDEFS) -D_ME_WINDOW
NANOEMACS_DEFS= -D_NANOEMACS
LDDEBUG       =
LDOPTIMISE    =
LDFLAGS       =
LIBS          =
CONSOLE_LIBS  = -lcurses
WINDOW_LIBS   = $(MAKEWINLIBS) -lX11
#
# Rules
.SUFFIXES: .c .oc .ow .ob .on .ov .oe .odc .odw .odb .odn .odv .ode

.c.oc:
	$(CC) $(COPTIMISE) $(CDEFS) $(MICROEMACS_DEFS) $(CONSOLE_DEFS) $(MAKECDEFS) -o _$*.o -c $<
	@$(MV) _$*.o $@

.c.ow:	
	$(CC) $(COPTIMISE) $(CDEFS) $(MICROEMACS_DEFS) $(WINDOW_DEFS) $(MAKECDEFS) -o _$*.o -c $<
	@$(MV) _$*.o $@

.c.ob:	
	$(CC) $(COPTIMISE) $(CDEFS) $(MICROEMACS_DEFS) $(CONSOLE_DEFS) $(WINDOW_DEFS) $(MAKECDEFS) -o _$*.o -c $<
	@$(MV) _$*.o $@

.c.on:
	$(CC) $(COPTIMISE) $(CDEFS) $(NANOEMACS_DEFS) $(CONSOLE_DEFS) $(MAKECDEFS) -o _$*.o -c $<
	@$(MV) _$*.o $@

.c.ov:	
	$(CC) $(COPTIMISE) $(CDEFS) $(NANOEMACS_DEFS) $(WINDOW_DEFS) $(MAKECDEFS) -o _$*.o -c $<
	@$(MV) _$*.o $@

.c.oe:	
	$(CC) $(COPTIMISE) $(CDEFS) $(NANOEMACS_DEFS) $(CONSOLE_DEFS) $(WINDOW_DEFS) $(MAKECDEFS) -o _$*.o -c $<
	@$(MV) _$*.o $@

# Debug Builds
.c.odc:
	$(CC) $(CDEBUG) $(CDEFS) $(MICROEMACS_DEFS) $(CONSOLE_DEFS) $(MAKECDEFS) -o _$*.o -c $<
	@$(MV) _$*.o $@

.c.odw:	
	$(CC) $(CDEBUG) $(CDEFS) $(MICROEMACS_DEFS) $(WINDOW_DEFS) $(MAKECDEFS) -o _$*.o -c $<
	@$(MV) _$*.o $@

.c.odb:	
	$(CC) $(CDEBUG) $(CDEFS) $(MICROEMACS_DEFS) $(CONSOLE_DEFS) $(WINDOW_DEFS) $(MAKECDEFS) -o _$*.o -c $<
	@$(MV) _$*.o $@

.c.odn:
	$(CC) $(CDEBUG) $(CDEFS) $(NANOEMACS_DEFS) $(CONSOLE_DEFS) $(MAKECDEFS) -o _$*.o -c $<
	@$(MV) _$*.o $@

.c.odv:	
	$(CC) $(CDEBUG) $(CDEFS) $(NANOEMACS_DEFS) $(WINDOW_DEFS) $(MAKECDEFS) -o _$*.o -c $<
	@$(MV) _$*.o $@

.c.ode:	
	$(CC) $(CDEBUG) $(CDEFS) $(NANOEMACS_DEFS) $(CONSOLE_DEFS) $(WINDOW_DEFS) $(MAKECDEFS) -o _$*.o -c $<
	@$(MV) _$*.o $@
#
# Source files
STDHDR	= ebind.h edef.h eextrn.h efunc.h emain.h emode.h eprint.h \
	  esearch.h eskeys.h estruct.h eterm.h evar.h evers.h eopt.h \
	  ebind.def efunc.def eprint.def evar.def etermcap.def emode.def eskeys.def
STDSRC	= abbrev.c basic.c bind.c buffer.c crypt.c dirlist.c display.c \
	  eval.c exec.c file.c fileio.c frame.c hilight.c history.c input.c \
	  isearch.c key.c line.c macro.c main.c narrow.c next.c osd.c \
	  print.c random.c regex.c region.c registry.c search.c spawn.c \
	  spell.c tag.c termio.c time.c undo.c window.c word.c

PLTHDR  = 
PLTSRC  = unixterm.c

HEADERS = $(STDHDR) $(PLTHDR)
SRC     = $(STDSRC) $(PLTSRC)
#
# Object files
OBJ_C    = $(SRC:.c=.oc)
OBJ_W    = $(SRC:.c=.ow)
OBJ_B    = $(SRC:.c=.ob)
OBJ_N    = $(SRC:.c=.on)
OBJ_V    = $(SRC:.c=.ov)
OBJ_E    = $(SRC:.c=.oe)

# Debug Builds
OBJ_DC   = $(SRC:.c=.odc)
OBJ_DW   = $(SRC:.c=.odw)
OBJ_DB   = $(SRC:.c=.odb)
OBJ_DN   = $(SRC:.c=.odn)
OBJ_DV   = $(SRC:.c=.odv)
OBJ_DE   = $(SRC:.c=.ode)
#
# Targets
all: me

install: me
	$(INSTALL) $(INSTPROGFLAGS) me $(INSTDIR)
	@echo "install done"

clean:
	$(RM) core me mec mew mecw ne nec new necw med medc medw medcw ned nedc nedw nedcw
	$(RM) *.oc *.ow *.ob *.on *.ov *.oe
	$(RM) *.odc *.odw *.odb *.odn *.odv *.ode

spotless: clean
	$(RM) tags *~

mec:	$(OBJ_C)
	$(RM) $@
	$(LD) $(LDFLAGS) $(LDOPTIMISE) -o $@ $(OBJ_C) $(CONSOLE_LIBS) $(LIBS)
	$(STRIP) $@

mew:	$(OBJ_W)
	$(RM) $@
	$(LD) $(LDFLAGS) $(LDOPTIMISE) -o $@ $(OBJ_W) $(WINDOW_LIBS) $(LIBS)
	$(STRIP) $@

mecw:	$(OBJ_B)
	$(RM) $@
	$(LD) $(LDFLAGS) $(LDOPTIMISE) -o $@ $(OBJ_B) $(CONSOLE_LIBS) $(WINDOW_LIBS) $(LIBS)
	$(STRIP) $@

me:	mecw
	$(CP) mecw $@

nec:	$(OBJ_N)
	$(RM) $@
	$(LD) $(LDFLAGS) $(LDOPTIMISE) -o $@ $(OBJ_N) $(CONSOLE_LIBS) $(LIBS)
	$(STRIP) $@

new:	$(OBJ_V)
	$(RM) $@
	$(LD) $(LDFLAGS) $(LDOPTIMISE) -o $@ $(OBJ_V) $(WINDOW_LIBS) $(LIBS)
	$(STRIP) $@

necw:	$(OBJ_E)
	$(RM) $@
	$(LD) $(LDFLAGS) $(LDOPTIMISE) -o $@ $(OBJ_E) $(CONSOLE_LIBS) $(WINDOW_LIBS) $(LIBS)
	$(STRIP) $@

ne:	nec
	$(CP) nec $@

# Debug Builds
medc:	$(OBJ_DC)
	$(RM) $@
	$(LD) $(LDFLAGS) $(LDDEBUG) -o $@ $(OBJ_DC) $(CONSOLE_LIBS) $(LIBS)

medw:	$(OBJ_DW)
	$(RM) $@
	$(LD) $(LDFLAGS) $(LDDEBUG) -o $@ $(OBJ_DW) $(WINDOW_LIBS) $(LIBS)

medcw:	$(OBJ_DB)
	$(RM) $@
	$(LD) $(LDFLAGS) $(LDDEBUG) -o $@ $(OBJ_DB) $(CONSOLE_LIBS) $(WINDOW_LIBS) $(LIBS)

med:	medcw
	$(CP) medcw $@

nedc:	$(OBJ_DN)
	$(RM) $@
	$(LD) $(LDFLAGS) $(LDDEBUG) -o $@ $(OBJ_DN) $(CONSOLE_LIBS) $(LIBS)

nedw:	$(OBJ_DV)
	$(RM) $@
	$(LD) $(LDFLAGS) $(LDDEBUG) -o $@ $(OBJ_DV) $(WINDOW_LIBS) $(LIBS)

nedcw:	$(OBJ_DE)
	$(RM) $@
	$(LD) $(LDFLAGS) $(LDDEBUG) -o $@ $(OBJ_DE) $(CONSOLE_LIBS) $(WINDOW_LIBS) $(LIBS)

ned:	nedc
	$(CP) nedc $@
#
# Dependancies
$(OBJ_C): $(HEADERS)
$(OBJ_W): $(HEADERS)
$(OBJ_B): $(HEADERS)
$(OBJ_N): $(HEADERS)
$(OBJ_V): $(HEADERS)
$(OBJ_E): $(HEADERS)

# Debug Builds
$(OBJ_DC): $(HEADERS)
$(OBJ_DW): $(HEADERS)
$(OBJ_DB): $(HEADERS)
$(OBJ_DN): $(HEADERS)
$(OBJ_DV): $(HEADERS)
$(OBJ_DE): $(HEADERS)
