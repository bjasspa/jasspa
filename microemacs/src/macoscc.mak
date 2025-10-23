# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# macoscc.mak - Make file for MacOS using cc compiler.
#
# Copyright (C) 2007-2024 JASSPA (www.jasspa.com)
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
# Synopsis:    Make file for macOS using cc
# Notes:
#     Run ./build.sh to compile, ./build.sh -h for more information.
#
##############################################################################

HASH     = \#
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

TOOLKIT  = cc
TOOLKIT_VER = $(shell $(CC) -dumpversion | cut -f 1 -d .)

ifneq "$(ARCHITEC)" ""
else ifeq "$(shell uname -p)" "i386"
ARCHITEC = intel
else
ARCHITEC = apple
endif
ifeq "$(ARCHITEC)" "intel"
ARCFLAGS = -arch x86_64
else
ARCFLAGS = -arch arm64
endif
ifeq "$(BIT_SIZE)" ""
BIT_SIZE = $(shell getconf LONG_BIT)
endif

PLATFORM = macos
PLATFORM_VER = $(shell sw_vers -productVersion | cut -f 1 -d .)

MAKEFILE = $(PLATFORM)$(TOOLKIT)
ifeq "$(BPRF)" "1"
BUILDID  = $(PLATFORM)$(PLATFORM_VER)-$(ARCHITEC)$(BIT_SIZE)-$(TOOLKIT)$(TOOLKIT_VER)p
else
BUILDID  = $(PLATFORM)$(PLATFORM_VER)-$(ARCHITEC)$(BIT_SIZE)-$(TOOLKIT)$(TOOLKIT_VER)
endif
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
TRDPARTY = ../3rdparty

CCDEFS   = -m$(BIT_SIZE) $(ARCFLAGS) -D_MACOS -D_ARCHITEC=$(ARCHITEC) -D_TOOLKIT=$(TOOLKIT) -D_TOOLKIT_VER=$(TOOLKIT_VER) -D_PLATFORM_VER=$(PLATFORM_VER) -D_$(BIT_SIZE)BIT -Wall -I$(TRDPARTY)/tfs -DmeVER_CN=$(meVER_CN) -DmeVER_YR=$(meVER_YR) -DmeVER_MN=$(meVER_MN) -DmeVER_DY=$(meVER_DY)
CCFLAGSR = -O3 -flto -DNDEBUG=1 -Wno-uninitialized
CCFLAGSD = -g
LDDEFS   = -m$(BIT_SIZE) $(ARCFLAGS)
LDFLAGSR = -O3 -flto=auto
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
INSTDIR  = 
INSTPRG  = - echo No install - debug 
else
BOUTDIR  = $(OUTDIRR)
CCFLAGS  = $(CCFLAGSR)
LDFLAGS  = $(LDFLAGSR)
ARFLAGS  = $(ARFLAGSR)
INSTDIR  = ../bin/$(BUILDID)
INSTPRG  = cp
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
PRGLIBS  = $(TRDPARTY)/tfs/$(BOUTDIR)/tfs$(A)

endif

ifeq "$(BPRF)" "1"
CCPROF = -D_ME_PROFILE -pg
LDPROF = -pg
STRIP  = - echo No strip - profile 
else
CCPROF = 
LDPROF = 
endif

ifneq (,$(findstring w,$(BTYP)))

ifeq "$(shell echo '$(HASH)include <stdio.h>\n$(HASH)include <X11/Intrinsic.h>\nint main(){return 0;}' | $(LD) -x c $(LDDEFS) -o /dev/null -lX11 > /dev/null 2> /dev/null - ; echo $$? )" "0"
X11_LIBS = -lX11
else ifneq (,$(wildcard /opt/X11/lib/libX11.dylib))
X11_INCL = -I/opt/X11/include -I/opt/X11/include/freetype2
X11_LIBS = -L/opt/X11/lib -lX11
else ifneq (,$(wildcard /usr/X11R6/lib/libX11.dylib))
X11_INCL = -I/usr/X11R6/include -I/usr/X11R6/include/freetype2
X11_LIBS = -L/usr/X11R6/lib -lX11
endif

ifeq "$(X11_LIBS)" ""
$(warning WARNING: No X11 support found, forcing build type to console only.)
override BTYP = c
else

ifeq "$(shell echo '$(HASH)include <stdio.h>\n$(HASH)include <X11/Intrinsic.h>\nint main(){return 0;}' | $(LD) -x c $(LDDEFS) $(X11_INCL) -o /dev/null $(X11_LIBS) -lXpm > /dev/null 2> /dev/null - ; echo $$? )" "0"
WINDOW_DEFS = $(X11_INCL) -D_XPM
WINDOW_LIBS = $(X11_LIBS) -lXpm
else
WINDOW_DEFS = $(X11_INCL)
WINDOW_LIBS = $(X11_LIBS)
endif
ifeq "$(shell echo '$(HASH)include <stdio.h>\n$(HASH)include <X11/Intrinsic.h>\n$(HASH)include <X11/Xft/Xft.h>\nint main(){return 0;}' | $(LD) -x c $(LDDEFS) $(X11_INCL) -o /dev/null $(X11_LIBS) -lXft > /dev/null 2> /dev/null - ; echo $$? )" "0"
WINDOW_DEFS += -DMEOPT_XFT=1 
WINDOW_LIBS += -lXft
endif

endif

endif

ifneq "$(BTYP)" "w"
#
# Preference now is to use "ncurses" rather than "termcap", figure out if ncurses is avaiable or if we must fall back to termcap.
#
ifeq "$(shell echo '$(HASH)include <stdio.h>\nint main(){return 0;}' | $(LD) -x c $(LDDEFS) -o _tmptst.out -lncurses > /dev/null 2> /dev/null - ; echo $$? )" "0"
CONSOLE_LIBS  = -lncurses
CONSOLE_DEFS  = -D_USE_NCURSES
else
$(warning WARNING: No ncurses, defaulting to termcap.)
CONSOLE_LIBS  = -ltermcap
endif
ifeq "$(shell rm -rf ./_tmptst.out* )" "0"
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
override BTYP = c
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
	$(CC) $(CCDEFS) $(BCOR_CDF) $(BTYP_CDF) $(CCFLAGS) -c -o $@ $<


all: $(PRGLIBS) $(OUTDIR)/$(PRGFILE)

$(OUTDIR)/$(PRGFILE): $(OUTDIR) $(INSTDIR) $(PRGOBJS) $(PRGLIBS)
	$(RM) $@
	$(LD) $(LDDEFS) $(LDFLAGS) -o $@ $(PRGOBJS) $(PRGLIBS) $(BTYP_LIB) $(LDLIBS)
	$(STRIP) $@
	$(INSTPRG) $@ $(INSTDIR)

$(PRGOBJS): $(PRGHDRS)

$(OUTDIR):
	-mkdir $(OUTDIR)

$(INSTDIR):
	-mkdir $(INSTDIR)

$(TRDPARTY)/tfs/$(BOUTDIR)/tfs$(A):
	cd $(TRDPARTY)/tfs && $(MK) -f $(MAKEFILE).mak ARCHITEC=$(ARCHITEC) BCFG=$(BCFG)

clean:
	$(RMDIR) $(OUTDIR)
	cd $(TRDPARTY)/tfs && $(MK) -f $(MAKEFILE).mak ARCHITEC=$(ARCHITEC) BCFG=$(BCFG) clean

spotless: clean
	$(RM) *~
	$(RM) tags
	cd $(TRDPARTY)/tfs && $(MK) -f $(MAKEFILE).mak ARCHITEC=$(ARCHITEC) BCFG=$(BCFG) spotless
