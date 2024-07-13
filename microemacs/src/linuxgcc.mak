# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# linux32gcc.mak - Make file for Linux (v3+ Kernel) using gcc
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
# Created:     Wed Jul 28 2004
# Synopsis:    Make file for Linux v2 Kernel 2.6 using gcc
# Notes:
#	Run "make -f linux26gcc.mak"      for optimised build produces ./me
#	Run "make -f linux26gcc.mak med"  for debug build produces     ./med
#
#	Run "make -f linux26gcc.mak clean"      to clean source directory
#	Run "make -f linux26gcc.mak spotless"   to clean source directory even more
#
##############################################################################
#
# Installation Directory
INSTDIR	      = /usr/local/bin
INSTPROGFLAGS = -s -o root -g root -m 0775
#
# Local Definitions
A        = .a
EXE      = 

CC       = gcc
MK       = make
LD       = $(CC)
STRIP    = strip
AR       = ar
RM       = rm -f
RMDIR    = rm -rf

include evers.mak

ifeq "$(BIT_SIZE)" ""
BIT_SIZE = $(shell getconf LONG_BIT)
endif

PLATFORM = linux
TOOLKIT  = gcc
ARCHITEC = intel
MAKEFILE = $(PLATFORM)$(TOOLKIT)
ifeq "$(BPRF)" "1"
BUILDID  = $(PLATFORM)-$(ARCHITEC)$(BIT_SIZE)$(TOOLKIT)p
else
BUILDID  = $(PLATFORM)-$(ARCHITEC)$(BIT_SIZE)$(TOOLKIT)
endif
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
TRDPARTY = ../3rdparty

TOOLKIT_VER = $(shell $(CC) -dumpversion)
PLATFORM_VER = $(shell uname -r | cut -f 1 -d .)

CCDEFS   = -m$(BIT_SIZE) -D_LINUX -D_ARCHITEC=$(ARCHITEC) -D_TOOLKIT=$(TOOLKIT) -D_TOOLKIT_VER=$(TOOLKIT_VER) -D_PLATFORM_VER=$(PLATFORM_VER) -D_$(BIT_SIZE)BIT -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -I. -I$(TRDPARTY)/tfs -I$(TRDPARTY)/zlib -DmeVER_CN=$(meVER_CN) -DmeVER_YR=$(meVER_YR) -DmeVER_MN=$(meVER_MN) -DmeVER_DY=$(meVER_DY) $(MAKECDEFS)
CCFLAGSR = -O3 -flto -DNDEBUG=1 -Wall -Wno-uninitialized -Wno-unused-result
CCFLAGSD = -g -Wall
LDDEFS   = -m$(BIT_SIZE)
LDFLAGSR = -O3 -flto
LDFLAGSD = -g
LDLIBS   = -lm -ldl

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

ifneq "$(OPENSSLP)" ""
else ifneq (,$(wildcard /usr/include/openssl/ssl.h))
OPENSSLP = 1
else ifneq (,$(wildcard /usr/local/opt/openssl/include/openssl/ssl.h))
OPENSSLP = 1 -I/usr/local/opt/openssl/include
endif
ifeq "$(OPENSSLP)" ""
$(warning WARNING: No OpenSSL support found, https support will be disabled.)
else
OPENSSLDEFS = -DMEOPT_OPENSSL=$(OPENSSLP) -D_OPENSSLLNM=libssl$(OPENSSLV).so -D_OPENSSLCNM=libcrypto$(OPENSSLV).so
endif
BCOR     = me
BCOR_CDF = -D_SOCKET $(OPENSSLDEFS)
PRGLIBS  = $(TRDPARTY)/tfs/$(BOUTDIR)/tfs$(A) $(TRDPARTY)/zlib/$(BOUTDIR)/zlib$(A)

endif

ifeq "$(BPRF)" "1"
CCPROF = -D_ME_PROFILE -pg
LDPROF = -pg
STRIP  = - echo No strip - profile 
else
CCPROF = 
LDPROF = 
endif

ifeq "$(BTYP)" ""
BTYP = c
else ifneq "$(BTYP)" "c"

ifeq "$(shell echo '#include <stdio.h>\n#include <X11/Intrinsic.h>\nint main(){return 0;}' | $(LD) -x c $(LDDEFS) $(LDFLAGS) -o /dev/null -lX11 > /dev/null 2> /dev/null - ; echo $$? )" "0"

X11_LIBS = -lX11

else ifeq "$(BIT_SIZE)" "64"

ifneq (,$(wildcard /usr/lib/x86_64-linux-gnu/libX11.a))
X11_LIBS = -L/usr/lib/x86_64-linux-gnu -lX11
else ifneq (,$(wildcard /usr/X11R6/lib64/libX11.a))
X11_LIBS = -L/usr/X11R6/lib64 -lX11
else ifneq (,$(wildcard /usr/X11R6/lib/libX11.a))
X11_LIBS = -L/usr/X11R6/lib -lX11
endif

else

ifneq (,$(wildcard /usr/lib/x86_64-linux-gnux32/libX11.a))
X11_LIBS = -L/usr/lib/x86_64-linux-gnux32 -lX11
else ifneq (,$(wildcard /usr/X11R6/lib/libX11.a))
X11_LIBS = -L/usr/X11R6/lib -lX11
endif

endif

ifneq "$(WINDOW_LIBS)" ""
$(warning WARNING: No X11 support found, forcing build type to console only.)
BTYP = c
else ifeq "$(shell echo '#include <stdio.h>\n#include <X11/Intrinsic.h>\nint main(){return 0;}' | $(LD) -x c $(LDDEFS) $(LDFLAGS) -o /dev/null $(X11_LIBS) -lXpm > /dev/null 2> /dev/null - ; echo $$? )" "0"
WINDOW_DEFS = -D_XPM
WINDOW_LIBS = $(X11_LIBS) -lXpm
else
WINDOW_LIBS = $(X11_LIBS)
endif

endif

ifneq "$(BTYP)" "w"
#
# Preference now is to use "ncurses" rather than "termcap", figure out if ncurses is avaiable or if we must fall back to termcap.
#
ifeq "$(shell echo '#include <stdio.h>\nint main(){return 0;}' | $(LD) -x c $(LDDEFS) $(LDFLAGS) -o /dev/null -lncurses > /dev/null 2> /dev/null - ; echo $$? )" "0"
CONSOLE_LIBS  = -lncurses
CONSOLE_DEFS  = -D_USE_NCURSES
else
$(warning WARNING: No ncurses, defaulting to termcap.)
CONSOLE_LIBS  = -ltermcap
endif
endif

ifeq "$(BTYP)" "cw"
BTYP_CDF = $(CONSOLE_DEFS) $(WINDOW_DEFS) -D_ME_CONSOLE -D_ME_WINDOW
BTYP_LIB = $(CONSOLE_LIBS) $(WINDOW_LIBS)
else ifeq "$(BTYP)" "w"
BTYP_CDF = $(WINDOW_DEFS) -D_ME_WINDOW
BTYP_LIB = $(WINDOW_LIBS)
else
BTYP_CDF = $(CONSOLE_DEFS) -D_ME_CONSOLE
BTYP_LIB = $(CONSOLE_LIBS)
BTYP     = c
endif

OUTDIR   = $(BOUTDIR)-$(BCOR)$(BTYP)
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
	   $(OUTDIR)/unixterm.o
#
# Rules
.SUFFIXES: .c .o

$(OUTDIR)/%.o : %.c
	$(CC) $(CCDEFS) $(CCPROF) $(BCOR_CDF) $(BTYP_CDF) $(CCFLAGS) -c -o $@ $<


all: $(PRGLIBS) $(OUTDIR)/$(PRGFILE)

$(OUTDIR)/$(PRGFILE): $(OUTDIR) $(PRGOBJS) $(PRGLIBS)
	$(RM) $@
	$(LD) $(LDDEFS) $(LDPROF) $(LDFLAGS) -o $@ $(PRGOBJS) $(PRGLIBS) $(BTYP_LIB) $(LDLIBS)
	$(STRIP) $@

$(PRGOBJS): $(PRGHDRS)

$(OUTDIR):
	-mkdir $(OUTDIR)

$(TRDPARTY)/zlib/$(BOUTDIR)/zlib$(A):
	cd $(TRDPARTY)/zlib && $(MK) -f $(MAKEFILE).mak BCFG=$(BCFG)

$(TRDPARTY)/tfs/$(BOUTDIR)/tfs$(A):
	cd $(TRDPARTY)/tfs && $(MK) -f $(MAKEFILE).mak BCFG=$(BCFG)

clean:
	$(RMDIR) $(OUTDIR)
	cd $(TRDPARTY)/tfs && $(MK) -f $(MAKEFILE).mak clean
	cd $(TRDPARTY)/zlib && $(MK) -f $(MAKEFILE).mak clean

spotless: clean
	$(RM) *~
	$(RM) tags
	cd $(TRDPARTY)/tfs && $(MK) -f $(MAKEFILE).mak spotless
	cd $(TRDPARTY)/zlib && $(MK) -f $(MAKEFILE).mak spotless
