# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# win32mingw.mak - Make file for Windows using MinGW development kit.
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
#	Run "mingw32-make -f winmingw.mak"            for optimised build produces ./.winmingw-release-mew/mew.exe
#	Run "mingw32-make -f winmingw.mak BCFG=debug" for debug build produces     ./.winmingw-debug-mew/mew.exe
#	Run "mingw32-make -f winmingw.mak BTYP=c"     for console support          ./.winmingw-release-mec/mec.exe
#	Run "mingw32-make -f winmingw.mak BCOR=ne"    for ne build produces        ./.winmingw-release-new/new.exe
#
#	Run "mingw32-make -f winmingw.mak clean"      to clean source directory
#	Run "mingw32-make -f winmingw.mak spotless"   to clean source directory even more
#
##############################################################################

A        = .a
EXE      = .exe
CC       = $(TOOLPREF)gcc
RC       = $(TOOLPREF)windres
MK       = mingw32-make
LD       = $(CC)
STRIP    = $(TOOLPREF)strip
AR       = $(TOOLPREF)ar
RM       = rm -f
RMDIR    = rm -r -f

TOOLKIT  = mingw
TOOLKIT_VER = $(subst -win32,,$(word 1,$(subst ., ,$(shell $(CC) -dumpversion))))

ARCHITEC = intel
ifneq "$(BIT_SIZE)" ""
else ifeq "$(PLATFORM)" "x64"
BIT_SIZE = 64
else
BIT_SIZE = 32
endif

PLATFORM = windows
ifneq "$(PLATFORM_VER)" ""
else ifeq "$(TOOLPREF)" ""
WINDOWS_VER = $(subst ., ,$(shell ver))
WINDOWS_MNV = $(word 5,$(WINDOWS_VER))
$(eval WINDOWS_MNR := $$$(WINDOWS_MNV))
PLATFORM_VER = $(word 4,$(WINDOWS_VER))$(subst $(WINDOWS_MNR),,$(WINDOWS_MNV))
else
PLATFORM_VER = 0
endif

include evers.mak

MAKEFILE = win$(TOOLKIT)
ifeq "$(BPRF)" "1"
BUILDID  = $(PLATFORM)$(PLATFORM_VER)-$(ARCHITEC)$(BIT_SIZE)-$(TOOLKIT)$(TOOLKIT_VER)p
else
BUILDID  = $(PLATFORM)$(PLATFORM_VER)-$(ARCHITEC)$(BIT_SIZE)-$(TOOLKIT)$(TOOLKIT_VER)
endif
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
TRDPARTY = ../3rdparty

CCDEFS   = -m$(BIT_SIZE) -D_MINGW -D_ARCHITEC=$(ARCHITEC) -D_TOOLKIT=$(TOOLKIT) -D_TOOLKIT_VER=$(TOOLKIT_VER) -D_PLATFORM_VER=$(PLATFORM_VER) -D_$(BIT_SIZE)BIT -Wall -I$(TRDPARTY)/tfs -I$(TRDPARTY)/zlib -DmeVER_CN=$(meVER_CN) -DmeVER_YR=$(meVER_YR) -DmeVER_MN=$(meVER_MN) -DmeVER_DY=$(meVER_DY)
CCFLAGSR = -O3 -mfpmath=sse -Ofast -flto -march=native -funroll-loops -DNDEBUG=1 -Wno-uninitialized
CCFLAGSD = -g -D_DEBUG
LDDEFS   = -m$(BIT_SIZE)
LDFLAGSR = -O3 -mfpmath=sse -Ofast -flto -march=native -funroll-loops
LDFLAGSD = -g
LDLIBSB  = -lshell32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32
ARFLAGSR = rcs
ARFLAGSD = rcs
RCFLAGS  = --input-format rc --output-format coff -DmeVER_CN=$(meVER_CN) -DmeVER_YR=$(meVER_YR) -DmeVER_MN=$(meVER_MN) -DmeVER_DY=$(meVER_DY) -D_MINGW $(RCOPTS)

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
ifeq "$(TOOLPREF)" ""
INSTPRG  = copy
else
INSTPRG  = cp
endif
endif

ifeq "$(BCOR)" "ne"

BCOR_CDF = -D_NANOEMACS
PRGLIBS  = 
LDLIBS   = $(LDLIBSB)

else

ifeq "$(OPENSSLP)" ""
ifeq "$(BIT_SIZE)" "64"
OSSL_DIR = x64
OSSL_LIB = -x64
else
OSSL_DIR = x86
endif
ifneq "$(wildcard $(TRDPARTY)/openssl-3.1/$(OSSL_DIR)/include/openssl/ssl.h)" ""
OPENSSLP = $(TRDPARTY)/openssl-3.1/$(OSSL_DIR)
OPENSSLV = -3_1$(OSSL_LIB)
else ifneq "$(wildcard $(TRDPARTY)/openssl-3/$(OSSL_DIR)/include/openssl/ssl.h)" ""
OPENSSLP = $(TRDPARTY)/openssl-3/$(OSSL_DIR)
OPENSSLV = -3$(OSSL_LIB)
else ifneq "$(wildcard $(TRDPARTY)/openssl-1.1/$(OSSL_DIR)/include/openssl/ssl.h)" ""
OPENSSLP = $(TRDPARTY)/openssl-1.1/$(OSSL_DIR)
OPENSSLV = -1_1$(OSSL_LIB)
endif
endif
ifeq "$(OPENSSLP)" ""
$(warning WARNING: No OpenSSL support found, https support will be disabled.)
else
OPENSSLDEFS = -DMEOPT_OPENSSL=1 -I$(OPENSSLP)/include -D_OPENSSLLNM=libssl$(OPENSSLV) -D_OPENSSLCNM=libcrypto$(OPENSSLV)
OPENSSLLIBS = $(OPENSSLP)/lib/libssl.lib $(OPENSSLP)/lib/libcrypto.lib -lcrypt32
endif
BCOR     = me
BCOR_CDF = -D_SOCKET $(OPENSSLDEFS)
PRGLIBS  = $(TRDPARTY)/tfs/$(BOUTDIR)/tfs$(A) $(TRDPARTY)/zlib/$(BOUTDIR)/zlib$(A)
LDLIBS   = $(OPENSSLLIBS) -lws2_32 -lmpr $(LDLIBSB)

endif

ifeq "$(BPRF)" "1"
CCPROF = -D_ME_PROFILE -pg -no-pie
LDPROF = -pg -no-pie
STRIP  = - echo No strip - profile 
else
CCPROF = 
LDPROF = 
endif

ifeq "$(BTYP)" "cw"
BTYP_CDF = -D_ME_CONSOLE -D_CONSOLE -D_ME_WINDOW
BTYP_LDF = -Wl,-subsystem,console
else ifeq "$(BTYP)" "c"
BTYP_CDF = -D_ME_CONSOLE -D_CONSOLE
BTYP_LDF = -Wl,-subsystem,console
else
BTYP_CDF = -D_ME_WINDOW
BTYP_LDF = -Wl,-subsystem,windows
BTYP     = w
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
	   $(OUTDIR)/winterm.o $(OUTDIR)/winprint.o $(OUTDIR)/$(BCOR).coff
#
# Rules
.SUFFIXES: .c .o .rc .coff

$(OUTDIR)/%.o : %.c
	$(CC) $(CCDEFS) $(CCPROF) $(BCOR_CDF) $(BTYP_CDF) $(CCFLAGS) -c -o $@ $<

$(OUTDIR)/%.coff : %.rc
	$(RC) $(RCFLAGS) -o $@ -i $<

all: $(PRGLIBS) $(OUTDIR)/$(PRGFILE)

$(OUTDIR)/$(PRGFILE): $(OUTDIR) $(INSTDIR) $(PRGOBJS) $(PRGLIBS)
	$(RM) $@
	$(LD) $(LDDEFS) $(LDPROF) $(BTYP_LDF) $(LDFLAGS) -o $@ $(PRGOBJS) $(PRGLIBS) $(LDLIBS)
	$(STRIP) $@
ifeq "$(TOOLPREF)" ""
	$(INSTPRG) $(subst /,\\,$@ $(INSTDIR))
else
	$(INSTPRG) $@ $(INSTDIR)
endif

$(PRGOBJS): $(PRGHDRS)

$(OUTDIR):
	-mkdir $(OUTDIR)

$(INSTDIR):
ifeq "$(TOOLPREF)" ""
	-mkdir $(subst /,\\,$(INSTDIR))
else
	-mkdir $(INSTDIR)
endif

$(TRDPARTY)/zlib/$(BOUTDIR)/zlib$(A):
	cd $(TRDPARTY)/zlib && $(MK) -f $(MAKEFILE).mak BCFG=$(BCFG) BPRF=$(BPRF) BIT_SIZE=$(BIT_SIZE) TOOLPREF=$(TOOLPREF) PLATFORM_VER=$(PLATFORM_VER) MK=$(MK)

$(TRDPARTY)/tfs/$(BOUTDIR)/tfs$(A):
	cd $(TRDPARTY)/tfs && $(MK) -f $(MAKEFILE).mak BCFG=$(BCFG) BPRF=$(BPRF) BIT_SIZE=$(BIT_SIZE) TOOLPREF=$(TOOLPREF) PLATFORM_VER=$(PLATFORM_VER) MK=$(MK)

clean:
	$(RMDIR) $(OUTDIR)
	cd $(TRDPARTY)/tfs && $(MK) -f $(MAKEFILE).mak clean BCFG=$(BCFG) BPRF=$(BPRF) BIT_SIZE=$(BIT_SIZE) TOOLPREF=$(TOOLPREF) PLATFORM_VER=$(PLATFORM_VER) MK=$(MK)
	cd $(TRDPARTY)/zlib && $(MK) -f $(MAKEFILE).mak clean BCFG=$(BCFG) BPRF=$(BPRF) BIT_SIZE=$(BIT_SIZE) TOOLPREF=$(TOOLPREF) PLATFORM_VER=$(PLATFORM_VER) MK=$(MK)

spotless: clean
	$(RM) *~
	$(RM) tags
	cd $(TRDPARTY)/tfs && $(MK) -f $(MAKEFILE).mak spotless BCFG=$(BCFG) BPRF=$(BPRF) BIT_SIZE=$(BIT_SIZE) TOOLPREF=$(TOOLPREF) PLATFORM_VER=$(PLATFORM_VER) MK=$(MK)
	cd $(TRDPARTY)/zlib && $(MK) -f $(MAKEFILE).mak spotless BCFG=$(BCFG) BPRF=$(BPRF) BIT_SIZE=$(BIT_SIZE) TOOLPREF=$(TOOLPREF) PLATFORM_VER=$(PLATFORM_VER) MK=$(MK)
