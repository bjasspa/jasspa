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
CC       = clang
SWIFTC   = swiftc
MK       = make
LD       = $(CC)
STRIP    = strip
AR       = ar
CP       = cp
RM       = rm -f
RMDIR    = rm -r -f

TOPDIR=..
include $(TOPDIR)/etc/makeinc.ver

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
SDK_PATH = $(shell xcrun --show-sdk-path)
SWLIBDIR := $(SDK_PATH)/usr/lib/swift

PLATFORM = macos
PLATFORM_VER = $(shell sw_vers -productVersion | cut -f 1 -d .)

BTYP     = nw
MAKEFILE = $(PLATFORM)$(BTYP)
ifeq "$(BPRF)" "1"
BUILDID  = $(PLATFORM)$(PLATFORM_VER)-$(ARCHITEC)$(BIT_SIZE)-$(TOOLKIT)$(TOOLKIT_VER)p
else
BUILDID  = $(PLATFORM)$(PLATFORM_VER)-$(ARCHITEC)$(BIT_SIZE)-$(TOOLKIT)$(TOOLKIT_VER)
endif
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
TRDPARTY = ../3rdparty
NWDIR    = $(PLATFORM)nw

CCDEFS   = -m$(BIT_SIZE) $(ARCFLAGS) -D_MACOS -D_ARCHITEC=$(ARCHITEC) -D_TOOLKIT=$(TOOLKIT) -D_TOOLKIT_VER=$(TOOLKIT_VER) -D_PLATFORM_VER=$(PLATFORM_VER) -D_$(BIT_SIZE)BIT -Wall -I$(TRDPARTY)/tfs -DmeVER_CN=$(meVER_CN) -DmeVER_YR=$(meVER_YR) -DmeVER_MN=$(meVER_MN) -DmeVER_DY=$(meVER_DY)
CCFLAGSR = -O3 -flto -DNDEBUG=1 -Wno-uninitialized
CCFLAGSD = -g -O0
SWDEFS   = -sdk $(SDK_PATH) -import-objc-header $(NWDIR)/ME-Bridging-Header.h -module-name MicroEmacs
SWLDEFS  = -framework Cocoa -framework CoreText -framework CoreGraphics -framework CoreFoundation -L$(SWLIBDIR) -Wl,-rpath,/usr/lib/swift -lpthread
LDDEFS   = -m$(BIT_SIZE) $(ARCFLAGS)
LDFLAGSR = -O3 -flto=auto
LDFLAGSD = -g -O0
LDLIBS   = 
ARFLAGSR = rcs
ARFLAGSD = rcs

ifeq "$(BCFG)" "debug"
BOUTDIR  = $(OUTDIRD)
CCFLAGS  = $(CCFLAGSD)
SWFLAGS  = -g -Onone
LDFLAGS  = $(LDFLAGSD)
ARFLAGS  = $(ARFLAGSD)
STRIP    = - echo No strip - debug 
INSTDIR  = 
INSTPRG  = - echo No install - debug 
DCP      = $(CP)
else
BOUTDIR  = $(OUTDIRR)
CCFLAGS  = $(CCFLAGSR)
SWFLAGS  = -O
LDFLAGS  = $(LDFLAGSR)
ARFLAGS  = $(ARFLAGSR)
INSTDIR  = ../bin/$(BUILDID)
INSTPRG  = cp
DCP      = - echo No cp - release
endif

ifneq "$(DMALLOC)" ""
CCDEFS  += -DDMALLOC -DDMALLOC_FUNC_CHECK -I$(TRDPARTY)/dmalloc 
LDLIBS  += -L$(TRDPARTY)/dmalloc -ldmalloc
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

BTYP_CDF = $(WINDOW_DEFS) -D_ME_WINDOW -D_ME_MACOSNW -I./macosnw/
BTYP_LIB = $(WINDOW_LIBS) -lswiftCore -lswiftFoundation

OUTDIR   = $(BOUTDIR)-$(BCOR)$(BTYP)
PRGNAME  = $(BCOR)$(BTYP)
PRGFILE  = $(PRGNAME)$(EXE)
APPNAME  = MicroEmacs
APPPATH  = $(APPNAME).app
# TODO mak name wrong
PRGHDRS  = ebind.h edef.h eextrn.h efunc.h emain.h emode.h eprint.h esearch.h eskeys.h estruct.h evar.h evers.h eopt.h \
	   ebind.def efunc.def eprint.def evar.def etermcap.def emode.def eskeys.def macosnw.h $(MAKEFILE).mak $(TOPDIR)/etc/makeinc.ver
PRGOBJS  = $(OUTDIR)/abbrev.o $(OUTDIR)/basic.o $(OUTDIR)/bind.o $(OUTDIR)/buffer.o $(OUTDIR)/crypt.o $(OUTDIR)/dirlist.o $(OUTDIR)/display.o \
	   $(OUTDIR)/eval.o $(OUTDIR)/exec.o $(OUTDIR)/file.o $(OUTDIR)/fileio.o $(OUTDIR)/frame.o $(OUTDIR)/hash.o $(OUTDIR)/hilight.o $(OUTDIR)/history.o \
	   $(OUTDIR)/input.o $(OUTDIR)/isearch.o $(OUTDIR)/key.o $(OUTDIR)/line.o $(OUTDIR)/macro.o $(OUTDIR)/main.o $(OUTDIR)/narrow.o $(OUTDIR)/next.o \
	   $(OUTDIR)/osd.o $(OUTDIR)/print.o $(OUTDIR)/random.o $(OUTDIR)/regex.o $(OUTDIR)/region.o $(OUTDIR)/registry.o $(OUTDIR)/search.o $(OUTDIR)/sock.o \
	   $(OUTDIR)/spawn.o $(OUTDIR)/spell.o $(OUTDIR)/tag.o $(OUTDIR)/termio.o $(OUTDIR)/time.o $(OUTDIR)/undo.o $(OUTDIR)/window.o $(OUTDIR)/word.o \
	   $(OUTDIR)/macosnw.o
SWIFTSRC = $(NWDIR)/MEApplication.swift $(NWDIR)/MEAppDelegate.swift $(NWDIR)/MEWindowController.swift $(NWDIR)/MEView.swift $(NWDIR)/MEClipboard.swift
SWIFTOBJ = $(OUTDIR)/swift_me.o
#
# Rules
.SUFFIXES: .c .o

$(OUTDIR)/%.o : %.c
	$(CC) $(CCDEFS) $(BCOR_CDF) $(BTYP_CDF) $(CCFLAGS) -c -o $@ $<


all: $(PRGLIBS) $(OUTDIR)/$(APPPATH)/Contents/MacOS/$(APPNAME)

$(OUTDIR)/$(APPPATH)/Contents/MacOS/$(APPNAME): $(OUTDIR)/$(PRGFILE) $(OUTDIR)/$(APPPATH)/Contents/MacOS $(OUTDIR)/$(APPPATH)/Contents/Info.plist $(OUTDIR)/$(APPPATH)/Contents/Resources/$(APPNAME).icns
	$(RM) $@
	$(CP) $(OUTDIR)/$(PRGFILE) $@
	$(DCP) $(OUTDIR)/$(APPNAME).swiftmodule $(OUTDIR)/$(APPPATH)/Contents/MacOS/$(APPNAME).swiftmodule
	
$(OUTDIR)/$(APPPATH)/Contents/Info.plist: $(NWDIR)/Info.plist $(OUTDIR)/$(APPPATH)/Contents
	$(RM) $@
	$(CP) $(NWDIR)/Info.plist $@

$(OUTDIR)/$(APPPATH)/Contents/Resources/$(APPNAME).icns: ../icons/logo/$(APPNAME).icns $(OUTDIR)/$(APPPATH)/Contents/Resources
	$(RM) $@
	$(CP) ../icons/logo/$(APPNAME).icns $@

$(OUTDIR)/$(PRGFILE): $(OUTDIR) $(INSTDIR) $(PRGOBJS) $(SWIFTOBJ) $(PRGLIBS)
	$(RM) $@
	$(LD) $(LDDEFS) $(LDFLAGS) $(SWLDEFS) -o $@ $(PRGOBJS) $(SWIFTOBJ) $(PRGLIBS) $(BTYP_LIB) $(LDLIBS)
	$(STRIP) $@
	$(INSTPRG) $@ $(INSTDIR)

$(PRGOBJS): $(PRGHDRS)

$(SWIFTOBJ): $(SWIFTSRC) $(NWDIR)/ME-Bridging-Header.h
	$(SWIFTC) $(SWDEFS) $(SWFLAGS) -emit-object -whole-module-optimization -emit-module \
	-emit-module-path $(OUTDIR)/$(APPNAME).swiftmodule -pch-output-dir $(OUTDIR) -o $@ $(SWIFTSRC)

$(OUTDIR)/$(APPPATH)/Contents/MacOS: $(OUTDIR)/$(APPPATH)/Contents
	-mkdir $@

$(OUTDIR)/$(APPPATH)/Contents/Resources: $(OUTDIR)/$(APPPATH)/Contents
	-mkdir $@

$(OUTDIR)/$(APPPATH)/Contents: $(OUTDIR)/$(APPPATH)
	-mkdir $@

$(OUTDIR)/$(APPPATH): $(OUTDIR)
	-mkdir $@

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
