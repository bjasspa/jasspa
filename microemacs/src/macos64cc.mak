# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# macos64cc.mak - Make file for MacOS using cc 64bit compiler.
#
# Copyright (C) 2007-2009 JASSPA (www.jasspa.com)
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
# Synopsis:    Make file for Windows using MinGW development kit.
# Notes:
#     Run ./build.sh to compile, ./build.sh -h for more information.
#
#     To build from the command line using make & makefile. 
#
#	Run "make -f macos64cc.mak"            for optimised build produces ./.macos64cc-release-mew/mecw
#	Run "make -f macos64cc.mak BCFG=debug" for debug build produces     ./.macos64cc-debug-mew/med
#	Run "make -f macos64cc.mak BTYP=c"     for console support          ./.macos64cc-release-mec/mec
#	Run "make -f macos64cc.mak BCOR=ne"    for ne build produces        ./.macos64cc-release-new/ne
#
#	Run "make -f macos64cc.mak clean"      to clean source directory
#	Run "make -f macos64cc.mak spotless"   to clean source directory even more
#
##############################################################################
#
# Installation Directory
INSTDIR	      = /c/emacs
INSTPROGFLAGS = 
#
# Local Definitions
A        = .a
EXE      = 

CC       = cc
MK       = make
LD       = $(CC)
STRIP    = strip
AR       = ar
RM       = rm -f
RMDIR    = rm -r -f

include evers.mak

BUILDID  = macos64cc
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
TRDPARTY = ../3rdparty

CCDEFS   = -DmeVER_CN=$(meVER_CN) -DmeVER_YR=$(meVER_YR) -DmeVER_MN=$(meVER_MN) -DmeVER_DY=$(meVER_DY) -D_MACOS -D_64BIT -m64 -Wall -I$(TRDPARTY)/tfs -I$(TRDPARTY)/zlib
CCFLAGSR = -O3 -flto -DNDEBUG=1 -Wno-uninitialized
CCFLAGSD = -g
LDDEFS   = -m64
LDFLAGSR = -O3 -flto
LDFLAGSD = -g
LDLIBS   = 
ARFLAGSR = rcs
ARFLAGSD = rcs

ifeq "$(BCFG)" "debug"
BOUTDIR  = $(OUTDIRD)
CCFLAGS  = $(CCFLAGSD)
LDFLAGS  = $(LDFLAGSD)
ARFLAGS  = $(ARFLAGSD)
STRIP    = - echo No strip - debug 
else
BOUTDIR  = $(OUTDIRR)
CCFLAGS  = $(CCFLAGSR)
LDFLAGS  = $(LDFLAGSR)
ARFLAGS  = $(ARFLAGSR)
endif

ifeq "$(BCOR)" "ne"

BCOR_CDF = -D_NANOEMACS
PRGLIBS  = 

else

ifneq "$(OPENSSLP)" ""
else ifneq "$(wildcard /usr/local/opt/openssl@3/include/openssl/ssl.h)" ""
OPENSSLP = /usr/local/opt/openssl@3
OPENSSLV = .3
else ifneq "$(wildcard /opt/homebrew/opt/openssl@3/include/openssl/ssl.h)" ""
OPENSSLP = /opt/homebrew/opt/openssl@3
OPENSSLV = .3
else ifneq "$(wildcard /usr/local/opt/openssl@1.1/include/openssl/ssl.h)" ""
OPENSSLP = /usr/local/opt/openssl@1.1
OPENSSLV = .1.1
else ifneq "$(wildcard /opt/homebrew/opt/openssl@1.1/include/openssl/ssl.h)" ""
OPENSSLP = /opt/homebrew/opt/openssl@1.1
OPENSSLV = .1.1
else ifneq "$(wildcard /usr/local/opt/openssl/include/openssl/ssl.h)" ""
OPENSSLP = /usr/local/opt/openssl
else ifneq "$(wildcard /opt/homebrew/opt/openssl/include/openssl/ssl.h)" ""
OPENSSLP = /opt/homebrew/opt/openssl
else ifneq "$(wildcard /usr/include/openssl/ssl.h)" ""
OPENSSLP = /usr/include
endif
ifeq "$(OPENSSLP)" ""
$(warning WARNING: No OpenSSL support found, https support will be disabled.)
else
OPENSSLDEFS = -DMEOPT_OPENSSL=1 -I$(OPENSSLP)/include -D_OPENSSLLNM=libssl$(OPENSSLV).dylib -D_OPENSSLCNM=libcrypto$(OPENSSLV).dylib
endif
BCOR     = me
BCOR_CDF = -D_SOCKET $(OPENSSLDEFS)
PRGLIBS  = $(TRDPARTY)/tfs/$(BOUTDIR)/tfs$(A) $(TRDPARTY)/zlib/$(BOUTDIR)/zlib$(A)

endif

#
# Preference now is to use "ncurses" rather than "termcap", figure out if ncurses is avaiable or if we must fall back to termcap.
#
test = $(shell echo "\#include <stdio.h>" > _t.c ; echo "int main() { printf(\"Test\"); return 0; }" >> _t.c ; $(LD) $(LDFLAGS) -o _t.out -lncurses _t.c 2>&1 ; rm -f _t.c _t.out)
ifneq "$(strip $(test))" ""
$(warning WARNING: No ncurses, defaulting to termcap.)
CONSOLE_LIBS  = -ltermcap
else
CONSOLE_LIBS  = -lncurses
CONSOLE_DEFS  = -D_USE_NCURSES
endif

ifeq "$(BTYP)" "c"
BTYP_CDF = $(CONSOLE_DEFS) -D_ME_CONSOLE -D_CONSOLE
BTYP_LIB = $(CONSOLE_LIBS)
else ifeq "$(BTYP)" "w"
BTYP_CDF = $(MAKEWINDEFS) -D_ME_WINDOW -I/opt/X11/include
BTYP_LIB = $(MAKEWINLIBS) -L/opt/X11/lib -lX11
else
BTYP_CDF = $(CONSOLE_DEFS) $(MAKEWINDEFS) -D_ME_CONSOLE -D_CONSOLE -D_ME_WINDOW -I/opt/X11/include
BTYP_LIB = $(CONSOLE_LIBS) $(MAKEWINLIBS) -L/opt/X11/lib -lX11
BTYP     = cw
endif

OUTDIR   = $(BOUTDIR)-$(BCOR)$(BTYP)
PRGNAME  = $(BCOR)$(BTYP)
PRGFILE  = $(PRGNAME)$(EXE)
PRGHDRS  = ebind.h edef.h eextrn.h efunc.h emain.h emode.h eprint.h esearch.h eskeys.h estruct.h eterm.h evar.h evers.h eopt.h \
	   ebind.def efunc.def eprint.def evar.def etermcap.def emode.def eskeys.def \
	   $(BUILDID).mak evers.mak
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
	$(CC) $(CCDEFS) $(BCOR_CDF) $(BTYP_CDF) $(CCFLAGS) -c -o $@ $<


all: $(PRGLIBS) $(OUTDIR)/$(PRGFILE)

$(OUTDIR)/$(PRGFILE): $(OUTDIR) $(PRGOBJS) $(PRGLIBS)
	$(RM) $@
	$(LD) $(LDDEFS) $(LDFLAGS) -o $@ $(PRGOBJS) $(PRGLIBS) $(BTYP_LIB) $(LDLIBS)
	$(STRIP) $@

$(PRGOBJS): $(PRGHDRS)

$(OUTDIR):
	-mkdir $(OUTDIR)

$(TRDPARTY)/zlib/$(BOUTDIR)/zlib$(A):
	cd $(TRDPARTY)/zlib && $(MK) -f $(BUILDID).mak BCFG=$(BCFG)

$(TRDPARTY)/tfs/$(BOUTDIR)/tfs$(A):
	cd $(TRDPARTY)/tfs && $(MK) -f $(BUILDID).mak BCFG=$(BCFG)

clean:
	$(RMDIR) $(OUTDIR)
	cd $(TRDPARTY)/tfs && $(MK) -f $(BUILDID).mak clean
	cd $(TRDPARTY)/zlib && $(MK) -f $(BUILDID).mak clean

spotless: clean
	$(RM) *~
	$(RM) tags
	cd $(TRDPARTY)/tfs && $(MK) -f $(BUILDID).mak spotless
	cd $(TRDPARTY)/zlib && $(MK) -f $(BUILDID).mak spotless
