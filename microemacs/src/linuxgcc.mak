# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# linuxgcc.mak - Make file for Linux using gcc
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
# Synopsis:    Make file for Linux using gcc
# Notes:
#     Run ./build.sh to compile, ./build.sh -h for more information.
#
##############################################################################

HASH     = \#
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

TOOLKIT  = gcc
TOOLKIT_VER = $(shell $(CC) -dumpversion)

ifneq "$(ARCHITEC)" ""
else ifeq "$(shell uname -m | cut -c 1-5)" "aarch"
ARCHITEC = aarch
else ifeq "$(shell uname -m | cut -c 1-3)" "arm"
ARCHITEC = aarch
else
ARCHITEC = intel
endif
ifeq (,$(BIT_SIZE))
BIT_SIZE = $(shell getconf LONG_BIT)
else
BIT_OPT  = -m$(BIT_SIZE)
endif

PLATFORM = linux
PLATFORM_VER = $(shell uname -r | cut -f 1 -d .)

MAKEFILE = $(PLATFORM)$(TOOLKIT)
ifeq (1,$(BPRF))
BUILDID  = $(PLATFORM)$(PLATFORM_VER)-$(ARCHITEC)$(BIT_SIZE)-$(TOOLKIT)$(TOOLKIT_VER)p
else
BUILDID  = $(PLATFORM)$(PLATFORM_VER)-$(ARCHITEC)$(BIT_SIZE)-$(TOOLKIT)$(TOOLKIT_VER)
endif
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
TRDPARTY = ../3rdparty

CCDEFS   = $(BIT_OPT) -D_LINUX -D_ARCHITEC=$(ARCHITEC) -D_TOOLKIT=$(TOOLKIT) -D_TOOLKIT_VER=$(TOOLKIT_VER) -D_PLATFORM_VER=$(PLATFORM_VER) -D_$(BIT_SIZE)BIT -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -I. -I$(TRDPARTY)/tfs -DmeVER_CN=$(meVER_CN) -DmeVER_YR=$(meVER_YR) -DmeVER_MN=$(meVER_MN) -DmeVER_DY=$(meVER_DY) $(MAKECDEFS)
CCFLAGSR = -O3 -flto -DNDEBUG=1 -Wall -Wno-uninitialized -Wno-unused-result
CCFLAGSD = -g -O0 -Wall
LDDEFS   = $(BIT_OPT)
LDFLAGSR = -O3 -flto=auto
LDFLAGSD = -g -O0
LDLIBS   = -lm -ldl

ifeq (debug,$(BCFG))
BOUTDIR  = $(OUTDIRD)
CCFLAGS  = $(CCFLAGSD)
LDFLAGS  = $(LDFLAGSD)
STRIP    = - echo No strip - debug 
INSTDIR  = 
INSTPRG  = - echo No install - debug 
else
BOUTDIR  = $(OUTDIRR)
CCFLAGS  = $(CCFLAGSR)
LDFLAGS  = $(LDFLAGSR)
INSTDIR  = ../bin/$(BUILDID)
INSTPRG  = cp
endif

ifeq (ne,$(BCOR))

BCOR_CDF = -D_NANOEMACS
PRGLIBS  = 

else

ifneq (,$(OPENSSLP))
else ifneq (,$(OPENSSLPATH))
OPENSSLP = 1 -I$(OPENSSLPATH)
else ifeq (0,$(shell printf '$(HASH)include <stdio.h>\n$(HASH)include <openssl/ssl.h>\nint main(){return 0;}\n' | $(LD) -x c $(LDDEFS) $(LDFLAGS) -o /dev/null > /dev/null 2> /dev/null - ; echo $$? ))
OPENSSLP = 1
else ifeq (0,$(shell printf '$(HASH)include <stdio.h>\n$(HASH)include <openssl/ssl.h>\nint main(){return 0;}\n' | $(LD) -x c $(LDDEFS) $(LDFLAGS) -I/usr/local/opt/openssl/include -o /dev/null > /dev/null 2> /dev/null - ; echo $$? ))
OPENSSLP = 1 -I/usr/local/opt/openssl/include
else ifeq (0,$(shell printf '$(HASH)include <stdio.h>\n$(HASH)include <openssl/ssl.h>\nint main(){return 0;}\n' | $(LD) -x c $(LDDEFS) $(LDFLAGS) -I/home/linuxbrew/.linuxbrew/opt/openssl@3.0/include -o /dev/null > /dev/null 2> /dev/null - ; echo $$? ))
OPENSSLP = 1 -I/home/linuxbrew/.linuxbrew/opt/openssl@3.0/include
endif
ifeq (,$(OPENSSLP))
$(warning WARNING: No OpenSSL support found, https support will be disabled.)
else
OPENSSLDEFS = -DMEOPT_OPENSSL=$(OPENSSLP) -D_OPENSSLLNM=libssl$(OPENSSLV).so -D_OPENSSLCNM=libcrypto$(OPENSSLV).so
endif
BCOR     = me
BCOR_CDF = -D_SOCKET $(OPENSSLDEFS)
PRGLIBS  = $(TRDPARTY)/tfs/$(BOUTDIR)/tfs$(A)

endif

ifeq (1,$(BPRF))
CCPROF = -D_ME_PROFILE -pg
LDPROF = -pg
STRIP  = - echo No strip - profile 
else
CCPROF = 
LDPROF = 
endif

ifneq (,$(findstring w,$(BTYP)))

ifeq (0,$(shell printf '$(HASH)include <stdio.h>\n$(HASH)include <X11/Intrinsic.h>\nint main(){return 0;}\n' | $(LD) -x c $(LDDEFS) $(LDFLAGS) -o /dev/null -lX11 > /dev/null 2> /dev/null - ; echo $$? ))

X11_LIBS = -lX11

else ifeq (64,$(BIT_SIZE))

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

ifneq (,$(WINDOW_LIBS))
$(warning WARNING: No X11 support found, forcing build type to console only.)
BTYP = c
else

ifeq (0,$(shell printf '$(HASH)include <stdio.h>\n$(HASH)include <X11/Intrinsic.h>\nint main(){return 0;}\n' | $(LD) -x c $(LDDEFS) $(LDFLAGS) -o /dev/null $(X11_LIBS) -lXpm > /dev/null 2> /dev/null - ; echo $$? ))
WINDOW_DEFS = -D_XPM 
WINDOW_LIBS = $(X11_LIBS) -lXpm
else
WINDOW_LIBS = $(X11_LIBS)
endif
ifeq (0,$(shell pkg-config --exists xft; echo $$? ))
WINDOW_DEFS += -DMEOPT_XFT=1 $(shell pkg-config --cflags xft)  
WINDOW_LIBS += $(shell pkg-config --libs xft)
endif

endif

endif

ifneq (w,$(BTYP))
#
# Preference now is to use "ncurses" rather than "termcap", figure out if ncurses is avaiable or if we must fall back to termcap.
#
ifeq (0,$(shell printf '$(HASH)include <stdio.h>\nint main(){return 0;}\n' | $(LD) -x c $(LDDEFS) $(LDFLAGS) -o /dev/null -lncurses > /dev/null 2> /dev/null - ; echo $$? ))
CONSOLE_LIBS  = -lncurses
CONSOLE_DEFS  = -D_USE_NCURSES
else
$(warning WARNING: No ncurses, defaulting to termcap.)
CONSOLE_LIBS  = -ltermcap
endif
endif

ifeq (cw,$(BTYP))
BTYP_CDF = $(CONSOLE_DEFS) $(WINDOW_DEFS) -D_ME_CONSOLE -D_ME_WINDOW
BTYP_LIB = $(CONSOLE_LIBS) $(WINDOW_LIBS)
else ifeq (w,$(BTYP))
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

$(OUTDIR)/$(PRGFILE): $(OUTDIR) $(INSTDIR) $(PRGOBJS) $(PRGLIBS)
	$(RM) $@
	$(LD) $(LDDEFS) $(LDPROF) $(LDFLAGS) -o $@ $(PRGOBJS) $(PRGLIBS) $(BTYP_LIB) $(LDLIBS)
	$(STRIP) $@
	$(INSTPRG) $@ $(INSTDIR)

$(PRGOBJS): $(PRGHDRS)

$(OUTDIR):
	-mkdir $(OUTDIR)

$(INSTDIR):
	-mkdir $(INSTDIR)

$(TRDPARTY)/tfs/$(BOUTDIR)/tfs$(A):
	cd $(TRDPARTY)/tfs && $(MK) -f $(MAKEFILE).mak BCFG=$(BCFG)

clean:
	$(RMDIR) $(OUTDIR)
	cd $(TRDPARTY)/tfs && $(MK) -f $(MAKEFILE).mak clean

spotless: clean
	$(RM) *~
	$(RM) tags
	cd $(TRDPARTY)/tfs && $(MK) -f $(MAKEFILE).mak spotless
