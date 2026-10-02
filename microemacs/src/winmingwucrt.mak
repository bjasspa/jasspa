# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# winmingwucrt.mak - Make file for Windows using MinGW-w64 UCRT/MSYS2.
#
# This makefile builds true Windows GUI applications for MicroEmacs 26
# using the MinGW-w64 toolchain from MSYS2. It supports three build
# distributions controlled by BDIST:
#   BDIST=ucrt64     - Native Windows UCRT64 executables (default when MSYSTEM=UCRT64)
#   BDIST=mingw64    - Native Windows MINGW64 executables
#   BDIST=msys2      - MSYS2 executables (requires msys-2.0.dll at runtime)
#
# For building from the command line using make & makefile:
#
#   Native Windows builds (UCRT64 or MINGW64):
#     make -f winmingwucrt.mak                        release GUI build
#     make -f winmingwucrt.mak BCFG=debug             debug GUI build
#     make -f winmingwucrt.mak BTYP=c                 release console build
#     make -f winmingwucrt.mak BCOR=ne                release ne build
#
#   MSYS2 builds:
#     make -f winmingwucrt.mak BDIST=msys2            release GUI build
#     make -f winmingwucrt.mak BDIST=msys2 BTYP=c     release console build
#
#     make -f winmingwucrt.mak clean                  to clean source directory
#     make -f winmingwucrt.mak spotless               to clean source directory even more
#
# Output directories:
#   UCRT64 builds:      ./.ucrt64win-{release,debug}-me{c,w}/
#   MINGW64 builds:     ./.mingw64win-{release,debug}-me{c,w}/
#   MSYS2 builds:       ./.msys2win-{release,debug}-me{c,w}/
#
# Copyright (C) 2007-2026 JASSPA (www.jasspa.com)
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

A        = .a
EXE      = .exe

# BDIST controls build distribution:
#   ucrt64   - Native Windows UCRT64 executables (default when MSYSTEM=UCRT64)
#   mingw64  - Native Windows MINGW64 executables
#   msys2    - MSYS2 executables (requires msys-2.0.dll at runtime)
BDIST    ?= $(AUTO_BDIST)

# Auto-detect MSYSTEM and set AUTO_BDIST + PKGPFX accordingly
ifdef MSYSTEM
ifeq "$(MSYSTEM)" "UCRT64"
AUTO_BDIST = ucrt64
PKGPFX   = mingw-w64-ucrt-x86_64
else ifeq "$(MSYSTEM)" "MINGW64"
AUTO_BDIST = mingw64
PKGPFX   = mingw-w64-x86_64
else ifeq "$(MSYSTEM)" "MINGW32"
AUTO_BDIST = mingw32
PKGPFX   = mingw-w64-i686
else
AUTO_BDIST = msys2
PKGPFX   = gcc
endif
else
# MSYSTEM not set (non-MSYS2 build)
endif

# Tool chain
CC       = gcc
RC       = windres
STRIP    = strip
AR       = ar
MK       = make
LD       = $(CC)
RM       = rm -f
RMDIR    = rm -r -f

# Native Windows builds need a MinGW-w64 gcc; when the toolchain directory
# (e.g. /ucrt64/bin) is empty the MSYS2 POSIX gcc (/usr/bin/gcc) is picked up
# silently and fails later with missing Windows headers like <direct.h>.
ifneq "$(BDIST)" "msys2"
CCMACHINE := $(shell $(CC) -dumpmachine 2>/dev/null)
ifeq "$(findstring w64-mingw32,$(CCMACHINE))" ""
$(error MinGW-w64 gcc not found (CC=gcc dumpmachine='$(CCMACHINE)'). Install the toolchain, e.g. pacman -S mingw-w64-ucrt-x86_64-toolchain, or build MSYS2-native with BDIST=msys2)
endif
ifeq "$(MSYSTEM)" "UCRT64"
ifeq "$(findstring /ucrt64/,$(shell command -v $(CC) 2>/dev/null))" ""
$(warning WARNING: MSYSTEM=UCRT64 but gcc resolves to '$(shell command -v $(CC) 2>/dev/null)' - install mingw-w64-ucrt-x86_64-toolchain for true UCRT64 builds)
endif
endif
endif

TOPDIR=..
include $(TOPDIR)/etc/makeinc.ver

TOOLKIT  = winmingwucrt
TOOLKIT_VER = $(subst -win32,,$(word 1,$(subst ., ,$(shell $(CC) -dumpversion))))

ARCHITEC = intel
ifneq "$(BIT_SIZE)" ""
else ifeq "$(PLATFORM)" "x64"
BIT_SIZE = 64
else ifeq "$(MSYSTEM_CARCH)" "x86_64"
BIT_SIZE = 64
else
BIT_SIZE = 32
endif

PLATFORM = windows
ifneq "$(PLATFORM_VER)" ""
else ifeq "$(TOOLPREF)" ""
ifeq "$(UNX_SHLL)" "0"
WINDOWS_VER := $(subst ., ,$(shell ver))
else
WINDOWS_VER := $(subst ., ,$(shell MSYS2_ARG_CONV_EXCL='*' cmd.exe /c ver))
endif
WINDOWS_MNV = $(word 5,$(WINDOWS_VER))
$(eval WINDOWS_MNR := $$$(WINDOWS_MNV))
PLATFORM_VER = $(word 4,$(WINDOWS_VER))$(subst $(WINDOWS_MNR),,$(WINDOWS_MNV))
else
PLATFORM_VER = 0
endif

# Set OUTTAG based on BDIST and MSYSTEM
# Format: .{env}{subsystem}-release-{type}
ifeq "$(BDIST)" "ucrt64"
ifdef MSYSTEM
ifeq "$(MSYSTEM)" "UCRT64"
OUTTAG   = ucrt64win
else
OUTTAG   = ucrt64win
endif
else
OUTTAG   = ucrt64win
endif
LDLIBSB  = -lshell32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -Wl,-Bstatic -lz -Wl,-Bdynamic
else ifeq "$(BDIST)" "mingw64"
ifdef MSYSTEM
ifeq "$(MSYSTEM)" "MINGW64"
OUTTAG   = mingw64win
else
OUTTAG   = mingw64win
endif
else
OUTTAG   = mingw64win
endif
LDLIBSB  = -lshell32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -Wl,-Bstatic -lz -Wl,-Bdynamic
else ifeq "$(BDIST)" "mingw32"
ifdef MSYSTEM
ifeq "$(MSYSTEM)" "MINGW32"
OUTTAG   = mingw32win
else
OUTTAG   = mingw32win
endif
else
OUTTAG   = mingw32win
endif
LDLIBSB  = -lshell32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -Wl,-Bstatic -lz -Wl,-Bdynamic
else
# Default MSYS2 build (dynamic zlib)
ifdef MSYSTEM
ifeq "$(MSYSTEM)" "UCRT64"
OUTTAG   = ucrt64msys
else ifeq "$(MSYSTEM)" "MINGW64"
OUTTAG   = mingw64msys
else ifeq "$(MSYSTEM)" "MINGW32"
OUTTAG   = mingw32msys
else
OUTTAG   = msys2msys
endif
else
OUTTAG   = msys2msys
endif
LDLIBSB  = -lshell32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lz
endif

ifneq "$(TOOLPREF)" ""
UNX_SHLL = 1
else ifeq (,$(MSYSTEM))
UNX_SHLL = 0
else
UNX_SHLL = 1
endif

ifeq "$(BPRF)" "1"
BUILDID  = $(OUTTAG)p
else
BUILDID  = $(OUTTAG)
endif
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
TRDPARTY = ../3rdparty
MAKEFILE = $(TOOLKIT)

# Compiler defines: native Windows builds use -D_WIN32 -D_MINGW
# MSYS2 builds also use these for Windows GUI apps
CCDEFS   = -m$(BIT_SIZE) -D_WIN32 -D_MINGW -D_ARCHITEC=$(ARCHITEC) -D_TOOLKIT=$(TOOLKIT) -D_TOOLKIT_VER=$(TOOLKIT_VER) -D_PLATFORM_VER=$(PLATFORM_VER) -D_$(BIT_SIZE)BIT -Wall -I$(TRDPARTY)/tfs -DmeVER_CN=$(meVER_CN) -DmeVER_YR=$(meVER_YR) -DmeVER_MN=$(meVER_MN) -DmeVER_DY=$(meVER_DY)
CCFLAGSR = -O3 -mfpmath=sse -Ofast -flto -funroll-loops -DNDEBUG=1 -Wno-uninitialized
CCFLAGSD = -g -D_DEBUG
LDDEFS   = -m$(BIT_SIZE)
LDFLAGSR = -O3 -mfpmath=sse -Ofast -flto -funroll-loops
LDFLAGSD = -g

# When building native Windows from MSYS2, the native gcc cannot resolve
# MSYS2 paths in TEMP/TMP environment variables. Override them with proper
# Windows paths so cc1.exe and the assembler can create temp files.
# Also ensure the mingw bin directory is on PATH so cc1.exe and as.exe can
# find their dependent DLLs (libisl, libmpc, libmpfr, etc.).
ifneq "$(BDIST)" "msys2"
WINTEMP  := $(shell cygpath -w /tmp 2>/dev/null || echo C:\\msys64\\tmp)
export TEMP  = $(WINTEMP)
export TMP   = $(WINTEMP)
ifdef MSYSTEM
ifneq "$(MSYSTEM)" "UCRT64"
export PATH := /mingw64/bin:$(PATH)
else
export PATH := /ucrt64/bin:$(PATH)
endif
endif
endif

ARFLAGSR = rcs
ARFLAGSD = rcs
RCFLAGS  = --input-format rc --output-format coff -DmeVER_CN=$(meVER_CN) -DmeVER_YR=$(meVER_YR) -DmeVER_MN=$(meVER_MN) -DmeVER_DY=$(meVER_DY) -D_WIN32 -D_MINGW $(RCOPTS)

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
ifeq "$(UNX_SHLL)" "0"
INSTPRG  = copy
else
INSTPRG  = cp
endif
endif

ifeq "$(BCOR)" "ne"

BCOR_CDF = -D_NANOEMACS
PRGLIBS  = 
LDLIBS   = $(LDLIBSB)
ifneq "$(BDIST)" "msys2"
LDLIBS  += -Wl,-Bstatic -lwinpthread -Wl,-Bdynamic
endif

else

ifneq "$(OPENSSLP)" ""
ifeq "$(OPENSSLV)" ""
ifeq "$(BIT_SIZE)" ""
OSSL_LIB = -x64
else ifeq "$(BIT_SIZE)" "64"
OSSL_LIB = -x64
endif
OPENSSLV = -3$(OSSL_LIB)
endif
else
ifneq "$(BIT_SIZE)" ""
ifeq "$(BIT_SIZE)" "64"
OSSL_DIR = x64
OSSL_LIB = -x64
else
OSSL_DIR = x86
endif
else
OSSL_DIR = x64
OSSL_LIB = -x64
endif
ifneq "$(wildcard $(TRDPARTY)/openssl-3.5/$(OSSL_DIR)/include/openssl/ssl.h)" ""
OPENSSLP = $(TRDPARTY)/openssl-3.5/$(OSSL_DIR)
OPENSSLV = -3$(OSSL_LIB)
else ifneq "$(wildcard $(TRDPARTY)/openssl-3.3/$(OSSL_DIR)/include/openssl/ssl.h)" ""
OPENSSLP = $(TRDPARTY)/openssl-3.3/$(OSSL_DIR)
OPENSSLV = -3$(OSSL_LIB)
else ifneq "$(wildcard $(TRDPARTY)/openssl-3.2/$(OSSL_DIR)/include/openssl/ssl.h)" ""
OPENSSLP = $(TRDPARTY)/openssl-3.2/$(OSSL_DIR)
OPENSSLV = -3$(OSSL_LIB)
else ifneq "$(wildcard $(TRDPARTY)/openssl-3.1/$(OSSL_DIR)/include/openssl/ssl.h)" ""
OPENSSLP = $(TRDPARTY)/openssl-3.1/$(OSSL_DIR)
OPENSSLV = -3$(OSSL_LIB)
else ifneq "$(wildcard $(TRDPARTY)/openssl-3.0/$(OSSL_DIR)/include/openssl/ssl.h)" ""
OPENSSLP = $(TRDPARTY)/openssl-3.0/$(OSSL_DIR)
OPENSSLV = -3$(OSSL_LIB)
else ifneq "$(wildcard $(TRDPARTY)/openssl-1.1/$(OSSL_DIR)/include/openssl/ssl.h)" ""
OPENSSLP = $(TRDPARTY)/openssl-1.1/$(OSSL_DIR)
OPENSSLV = -1_1$(OSSL_LIB)
endif
ifeq "$(OPENSSLP)" ""
ifeq "$(UNX_SHLL)" "1"
ifneq (0,$(shell pkg-config --exists openssl 2>/dev/null; echo $$?))
else ifeq (0,$(shell pkg-config --modversion openssl 2>/dev/null | grep -c "^3\."))
else
ifeq "$(BDIST)" "msys2"
OSSL_SYSFX = /usr
OPENSSLV = -3
else ifeq "$(BDIST)" "mingw32"
OSSL_SYSFX = /mingw32
OPENSSLV = -3$(OSSL_LIB)
else ifeq "$(BDIST)" "mingw64"
OSSL_SYSFX = /mingw64
OPENSSLV = -3$(OSSL_LIB)
else
OSSL_SYSFX = /ucrt64
OPENSSLV = -3$(OSSL_LIB)
endif
OSSL_SYSPREFIX := $(shell pkg-config --variable=prefix openssl 2>/dev/null)
ifneq "$(findstring $(OSSL_SYSFX),$(OSSL_SYSPREFIX))" ""
OPENSSLP = sys
endif
endif
endif
endif
endif
ifeq "$(OPENSSLP)" ""
$(warning WARNING: No OpenSSL support found, https support will be disabled.)
else ifeq "$(OPENSSLP)" "sys"
ifeq "$(BDIST)" "msys2"
OPENSSLDEFS = -DMEOPT_OPENSSL=1 -D_OPENSSLLNM=msys-ssl$(OPENSSLV).dll -D_OPENSSLCNM=msys-crypto$(OPENSSLV).dll
else
OPENSSLDEFS = -DMEOPT_OPENSSL=1 -D_OPENSSLLNM=libssl$(OPENSSLV).dll -D_OPENSSLCNM=libcrypto$(OPENSSLV).dll
endif
OPENSSLLIBS = -lssl -lcrypto -lcrypt32
else
OPENSSLDEFS = -DMEOPT_OPENSSL=1 -I$(OPENSSLP)/include -D_OPENSSLLNM=libssl$(OPENSSLV).dll -D_OPENSSLCNM=libcrypto$(OPENSSLV).dll
OPENSSLLIBS = $(OPENSSLP)/lib/libssl.lib $(OPENSSLP)/lib/libcrypto.lib -lcrypt32
endif
BCOR     = me
BCOR_CDF = -D_SOCKET $(OPENSSLDEFS)
PRGLIBS  = $(TRDPARTY)/tfs/$(BOUTDIR)/tfs$(A)
LDLIBS   = $(OPENSSLLIBS) -lws2_32 -lmpr $(LDLIBSB)
ifneq "$(BDIST)" "msys2"
LDLIBS  += -Wl,-Bstatic -lwinpthread -Wl,-Bdynamic
endif

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
	   $(MAKEFILE).mak $(TOPDIR)/etc/makeinc.ver
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
	-$(RM) $@
	$(LD) $(LDDEFS) $(LDPROF) $(BTYP_LDF) $(LDFLAGS) -o $@ $(PRGOBJS) $(PRGLIBS) $(LDLIBS)
	$(STRIP) $@
ifeq "$(UNX_SHLL)" "0"
	$(INSTPRG) $(subst /,\\,$@ $(INSTDIR))
else
	$(INSTPRG) $@ $(INSTDIR)
endif
ifneq "$(INSTDIR)" ""
ifeq "$(OPENSSLP)" "sys"
ifeq "$(UNX_SHLL)" "0"
	copy $(subst /,\\,$(OSSL_SYSPREFIX)/bin/libssl$(OPENSSLV).dll $(OSSL_SYSPREFIX)/bin/libcrypto$(OPENSSLV).dll $(INSTDIR))
else
	cp $(OSSL_SYSPREFIX)/bin/libssl$(OPENSSLV).dll $(OSSL_SYSPREFIX)/bin/libcrypto$(OPENSSLV).dll $(INSTDIR)
endif
endif
endif

$(PRGOBJS): $(PRGHDRS)

$(OUTDIR):
	-mkdir $(OUTDIR)

$(INSTDIR):
ifeq "$(UNX_SHLL)" "0"
	-mkdir $(subst /,\\,$(INSTDIR))
else
	-mkdir $(INSTDIR)
endif

$(TRDPARTY)/tfs/$(BOUTDIR)/tfs$(A):
	cd $(TRDPARTY)/tfs && $(MK) -f $(MAKEFILE).mak BCFG=$(BCFG) BPRF=$(BPRF) BIT_SIZE=$(BIT_SIZE) TOOLPREF=$(TOOLPREF) PLATFORM_VER=$(PLATFORM_VER) MK=$(MK) BUILDID=$(BUILDID)

clean:
	$(RMDIR) $(OUTDIR)
	cd $(TRDPARTY)/tfs && $(MK) -f $(MAKEFILE).mak clean BCFG=$(BCFG) BPRF=$(BPRF) BIT_SIZE=$(BIT_SIZE) TOOLPREF=$(TOOLPREF) PLATFORM_VER=$(PLATFORM_VER) MK=$(MK) BUILDID=$(BUILDID)

spotless: clean
	$(RM) *~
	$(RM) tags
	cd $(TRDPARTY)/tfs && $(MK) -f $(MAKEFILE).mak spotless BCFG=$(BCFG) BPRF=$(BPRF) BIT_SIZE=$(BIT_SIZE) TOOLPREF=$(TOOLPREF) PLATFORM_VER=$(PLATFORM_VER) MK=$(MK) BUILDID=$(BUILDID)

print-info:
	@echo "MSYSTEM=$(MSYSTEM)"
	@echo "BDIST=$(BDIST)"
	@echo "OUTTAG=$(OUTTAG)"
	@echo "PKGPFX=$(PKGPFX)"
	@echo "OUTDIR=$(OUTDIR)"
	@echo "CC=$(CC)"
	@echo "CCDEFS=$(CCDEFS)"
	@echo "BCOR_CDF=$(BCOR_CDF)"
	@echo "OPENSSLP=$(OPENSSLP)"
	@echo "LDLIBSB=$(LDLIBSB)"
