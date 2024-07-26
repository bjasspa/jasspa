##############################################################################
#
## JASSPA MicroEmacs - www.jasspa.com
# win32mingw.mak - Make file for Windows using MinGW development kit.
#
# Copyright (C) 2007-2022 JASSPA (www.jasspa.com)
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
CC       = $(TOOLPREF)gcc
RC       = $(TOOLPREF)windres
MK       = mingw32-make
LD       = $(CC)
STRIP    = $(TOOLPREF)strip
AR       = $(TOOLPREF)ar
RM       = rm -f
RMDIR    = rm -rf

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

MAKEFILE = win$(TOOLKIT)
ifeq "$(BPRF)" "1"
BUILDID  = $(PLATFORM)$(PLATFORM_VER)-$(ARCHITEC)$(BIT_SIZE)-$(TOOLKIT)$(TOOLKIT_VER)p
else
BUILDID  = $(PLATFORM)$(PLATFORM_VER)-$(ARCHITEC)$(BIT_SIZE)-$(TOOLKIT)$(TOOLKIT_VER)
endif
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
TRDPARTY = ..

CCDEFS   = -m$(BIT_SIZE) -D_MINGW -D_$(BIT_SIZE)BIT -Wall -I$(TRDPARTY)/zlib
CCFLAGSR = -O3 -DNDEBUG=1 -Wno-uninitialized
CCFLAGSD = -g
LDDEFS   = -m$(BIT_SIZE) 
LDFLAGSR = -O3 -flto
LDFLAGSD = -g
ARFLAGSR = rcs
ARFLAGSD = rcs
RCFLAGS  =

ifeq "$(BCFG)" "debug"
OUTDIR   = $(OUTDIRD)
CCFLAGS  = $(CCFLAGSD)
LDFLAGS  = $(LDFLAGSD)
ARFLAGS  = $(ARFLAGSD)
STRIP    = - echo No strip - debug 
INSTDIR  = 
INSTPRG  = - echo No install - debug 
else
OUTDIR   = $(OUTDIRR)
CCFLAGS  = $(CCFLAGSR)
LDFLAGS  = $(LDFLAGSR)
ARFLAGS  = $(ARFLAGSR)
INSTDIR  = ../../bin/$(BUILDID)
ifeq "$(TOOLPREF)" ""
INSTPRG  = copy
else
INSTPRG  = cp
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

LIBNAME  = tfs
LIBFILE  = $(LIBNAME)$(A)
LIBHDRS  = tfs.h $(MAKEFILE).mak
LIBOBJS  = $(OUTDIR)/tfs.o

PRGNAME  = tfs
PRGFILE  = $(PRGNAME)$(EXE)
PRGHDRS  = tfs.h tfsutil.h $(MAKEFILE).mak
PRGOBJS  = $(OUTDIR)/tfsutil.o $(OUTDIR)/tfs.o $(OUTDIR)/uappend.o $(OUTDIR)/ubuild.o $(OUTDIR)/ucopy.o \
	   $(OUTDIR)/ucreate.o $(OUTDIR)/uinfo.o $(OUTDIR)/ulist.o $(OUTDIR)/ustrip.o $(OUTDIR)/utest.o \
	   $(OUTDIR)/uxdir.o $(OUTDIR)/uxfile.o
PRGLIBS  = $(TRDPARTY)/zlib/$(OUTDIR)/zlib$(A)

.SUFFIXES: .c .o

$(OUTDIR)/%.o : %.c
	$(CC) $(CCDEFS) $(CCPROF) $(CCFLAGS) -c -o $@ $<

all: $(PRGLIBS) $(OUTDIR)/$(LIBFILE) $(OUTDIR)/$(PRGFILE)

$(OUTDIR)/$(LIBFILE): $(OUTDIR) $(LIBOBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $(LIBOBJS)

$(LIBOBJS): $(LIBHDRS)

$(OUTDIR)/$(PRGFILE): $(OUTDIR) $(INSTDIR) $(PRGOBJS) $(PRGLIBS)
	$(RM) $@
	$(LD) $(LDFLAGS) $(LDPROF) -o $@ $(PRGOBJS) $(PRGLIBS)
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

$(TRDPARTY)/zlib/$(OUTDIR)/zlib$(A):
	cd $(TRDPARTY)/zlib && $(MK) -f $(MAKEFILE).mak BCFG=$(BCFG) BPRF=$(BPRF) BIT_SIZE=$(BIT_SIZE) PLATFORM_VER=$(PLATFORM_VER)


clean:
	$(RMDIR) $(OUTDIRD)
	$(RMDIR) $(OUTDIRR)

spotless: clean
	$(RM) *~
	$(RM) tags
