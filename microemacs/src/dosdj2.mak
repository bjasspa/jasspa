# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# dosdj2.mak - Make file for Dos using djgpp v2.0
#
# Copyright (C) 2001-2024 JASSPA (www.jasspa.com)
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
#       Run ./dosdj2.bat to compile, ./dosdj2.bat -h for more information.
#
#   DJDIR must be setup to point to your DJGPP installation (normally done in djgpp.env)
#
##############################################################################
#
# Installation Directory
INSTALL  = copy
INSTDIR	 = c:\emacs
INSTPROGFLAGS = 
#
# Local Definitions
CP       = copy
CC       = gcc
LD       = $(CC)
MK       = make
STRIP    = strip
COFF     = coff2exe
RM       = del
RMDIR    = deltree /Y

include evers.mak

PLATFORM = dos
PLATFORM_VER = 5
TOOLKIT  = djgpp2
TOOLKIT_VER = 9
ARCHITEC = intel
MAKEFILE = dosdj2
BUILDID  = dosdj2
OUTDIRR  = $(BUILDID)_r
OUTDIRD  = $(BUILDID)_d

CCDEFS   = -D_DOS -D_ARCHITEC=$(ARCHITEC) -D_TOOLKIT=$(TOOLKIT) -D_TOOLKIT_VER=$(TOOLKIT_VER) -D_PLATFORM_VER=$(PLATFORM_VER) -D__DJGPP2__ -I. -DmeVER_CN=$(meVER_CN) -DmeVER_YR=$(meVER_YR) -DmeVER_MN=$(meVER_MN) -DmeVER_DY=$(meVER_DY)
CCFLAGSR = -O2 -DNDEBUG=1 -Wall -Wno-uninitialized
CCFLAGSD = -g -Wall
LDDEFS   = 
LDFLAGSR = -O2
LDFLAGSD = -g
LDLIBS   = -lpc

ifeq "$(BCFG)" "debug"
BOUTDIR  = $(OUTDIRD)
CCFLAGS  = $(CCFLAGSD)
LDFLAGS  = $(LDFLAGSD)
STRIP    = - echo No strip - debug 
else
BOUTDIR  = $(OUTDIRR)
CCFLAGS  = $(CCFLAGSR)
LDFLAGS  = $(LDFLAGSR)
endif

ifeq "$(BCOR)" "ne"
BCOR_CDF = -D_NANOEMACS
PRGLIBS  = 
else
BCOR     = me
BCOR_CDF = 
PRGLIBS  = 
endif

#
# Rules
OUTDIR   = $(BOUTDIR).$(BCOR)
PRGNAME  = $(BCOR)$(BTYP)
PRGFILE  = $(PRGNAME)$(EXE)
PRGHDRS  = ebind.h edef.h eextrn.h efunc.h emain.h emode.h eprint.h esearch.h eskeys.h estruct.h eterm.h evar.h evers.h eopt.h \
	   ebind.def efunc.def eprint.def evar.def etermcap.def emode.def eskeys.def \
	   $(MAKEFILE).mak evers.mak
PRGOBJS  = $(OUTDIR)/abbrev.o $(OUTDIR)/basic.o $(OUTDIR)/bind.o $(OUTDIR)/buffer.o $(OUTDIR)/crypt.o $(OUTDIR)/dirlist.o $(OUTDIR)/display.o \
	   $(OUTDIR)/eval.o $(OUTDIR)/exec.o $(OUTDIR)/file.o $(OUTDIR)/fileio.o $(OUTDIR)/frame.o $(OUTDIR)/hash.o $(OUTDIR)/hilight.o $(OUTDIR)/history.o \
	   $(OUTDIR)/input.o $(OUTDIR)/isearch.o $(OUTDIR)/key.o $(OUTDIR)/line.o $(OUTDIR)/macro.o $(OUTDIR)/main.o $(OUTDIR)/narrow.o $(OUTDIR)/next.o \
	   $(OUTDIR)/osd.o $(OUTDIR)/print.o $(OUTDIR)/random.o $(OUTDIR)/regex.o $(OUTDIR)/region.o $(OUTDIR)/registry.o $(OUTDIR)/search.o $(OUTDIR)/sock.o \
	   $(OUTDIR)/spawn.o $(OUTDIR)/spell.o $(OUTDIR)/tag.o $(OUTDIR)/termio.o $(OUTDIR)/time.o $(OUTDIR)/undo.o $(OUTDIR)/window.o $(OUTDIR)/word.o \
	   $(OUTDIR)/dosterm.o
#
# Rules
.SUFFIXES: .c .o

$(OUTDIR)/%.o : %.c
	$(CC) $(CCDEFS) $(BCOR_CDF) $(CCFLAGS) -c -o $@ $<


all: $(PRGLIBS) $(OUTDIR)/$(PRGFILE)

$(OUTDIR)/$(PRGFILE): $(OUTDIR) $(PRGOBJS) $(PRGLIBS)
	$(RM) $@
	$(LD) $(LDDEFS) $(LDFLAGS) -o $@ $(PRGOBJS) $(PRGLIBS) $(BTYP_LIB) $(LDLIBS)
	$(STRIP) $@

$(PRGOBJS): $(PRGHDRS)

$(OUTDIR):
	if not exist $(OUTDIR)\ md $(OUTDIR)

clean:
	$(RMDIR) $(OUTDIR)

spotless: clean
	$(RM) *~
	$(RM) tags
