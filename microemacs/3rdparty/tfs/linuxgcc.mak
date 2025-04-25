# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# linuxgcc.mak - Make file for Linux (v3+ Kernel) using gcc
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
EXE      = 
CC       = gcc
MK       = make
LD       = $(CC)
STRIP    = strip
AR       = ar
RM       = rm -f
RMDIR    = rm -rf

TOOLKIT  = gcc
TOOLKIT_VER = $(shell $(CC) -dumpversion)

ifneq "$(ARCHITEC)" ""
else ifeq "$(shell uname -m | cut -c 1-3)" "arm"
ARCHITEC = arm
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

CCDEFS   = $(BIT_OPT) -D_LINUX -D_$(BIT_SIZE)BIT -DZ7_ST -Wall -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -I.
CCFLAGSR = -O3 -flto -DNDEBUG=1 -Wno-uninitialized
CCFLAGSD = -g
LDDEFS   = $(BIT_OPT)
LDFLAGSR = -O3 -flto
LDFLAGSD = -g
ARFLAGSR = rcs
ARFLAGSD = rcs

ifeq "$(BCFG)" "debug"
OUTDIR   = $(OUTDIRD)
CCFLAGS  = $(CCFLAGSD)
LDFLAGS  = $(LDFLAGSD)
ARFLAGS  = $(ARFLAGSD)
STRIP    = - echo No strip - debug 
INSTDIR	 = 
INSTPRG  = - echo No install - debug 
else
OUTDIR   = $(OUTDIRR)
CCFLAGS  = $(CCFLAGSR)
LDFLAGS  = $(LDFLAGSR)
ARFLAGS  = $(ARFLAGSR)
INSTDIR	 = ../../bin/$(BUILDID)
INSTPRG  = cp
endif

ifeq "$(BPRF)" "1"
CCPROF = -D_ME_PROFILE -pg
LDPROF = -pg
STRIP  = - echo No strip - profile 
else
CCPROF = 
LDPROF = 
endif

LIBNAME  = tfs
LIBFILE  = $(LIBNAME)$(A)
LIBHDRS  = tfs.h $(MAKEFILE).mak
LIBOBJS  = $(OUTDIR)/tfs.o $(OUTDIR)/Lzma2Dec.o $(OUTDIR)/LzmaDec.o

PRGNAME  = tfs
PRGFILE  = $(PRGNAME)$(EXE)
PRGHDRS  = tfs.h 7zTypes.h 7zWindows.h Compiler.h CpuArch.h LzFind.h LzHash.h LzmaDec.h LzmaEnc.h Precomp.h \
	   $(MAKEFILE).mak
PRGOBJS  = $(OUTDIR)/tfsutil.o $(OUTDIR)/LzFind.o $(OUTDIR)/Lzma2Enc.o $(OUTDIR)/LzmaEnc.o $(OUTDIR)/CpuArch.o
PRGLIBS  = 

TSTNAME  = tfstest
TSTFILE  = $(TSTNAME)$(EXE)
TSTHDRS  = tfs.h $(MAKEFILE).mak
TSTOBJS  = $(OUTDIR)/tfstest.o
TSTLIBS  = $(OUTDIR)/tfs$(A)

.SUFFIXES: .c .o

$(OUTDIR)/%.o : %.c
	$(CC) $(CCDEFS) $(CCPROF) $(CCFLAGS) -c -o $@ $<

all: $(OUTDIR)/$(LIBFILE) $(OUTDIR)/$(PRGFILE) $(OUTDIR)/$(TSTFILE)

$(OUTDIR)/$(LIBFILE): $(OUTDIR) $(LIBOBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $(LIBOBJS)

$(LIBOBJS): $(LIBHDRS)

$(OUTDIR)/$(PRGFILE): $(OUTDIR) $(INSTDIR) $(PRGOBJS) $(PRGLIBS)
	$(RM) $@
	$(LD) $(LDDEFS) $(LDPROF) $(LDFLAGS) -o $@ $(PRGOBJS) $(PRGLIBS)
	$(STRIP) $@
	$(INSTPRG) $@ $(INSTDIR)

$(PRGOBJS): $(PRGHDRS)

$(OUTDIR)/$(TSTFILE): $(OUTDIR) $(INSTDIR) $(TSTOBJS) $(TSTLIBS)
	$(RM) $@
	$(LD) $(LDDEFS) $(LDPROF) $(LDFLAGS) -o $@ $(TSTOBJS) $(TSTLIBS)
	$(STRIP) $@

$(TSTOBJS): $(TSTHDRS)

$(OUTDIR):
	-mkdir $(OUTDIR)

$(INSTDIR):
	-mkdir $(INSTDIR)

clean:
	$(RMDIR) $(OUTDIRD)
	$(RMDIR) $(OUTDIRR)

spotless: clean
	$(RM) *~
	$(RM) tags
