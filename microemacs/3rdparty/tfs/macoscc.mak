##############################################################################
#
#  Copyright (c) 2009 Jasspa.
# 
#  All Rights Reserved.
# 
#  This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
#  reproduced,  translated,  or  reduced to any  electronic  medium or machine
#  readable form without prior written consent from Maxexam Ltd.
#
##############################################################################

A        = .a
EXE      = 

CC       = cc
MK       = make
LD       = $(CC)
STRIP    = strip
AR       = ar
RM       = rm -f
RMDIR    = rm -rf

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
PLATFORM_VER = $(shell uname -r | cut -f 1 -d .)

MAKEFILE = $(PLATFORM)$(TOOLKIT)
ifeq "$(BPRF)" "1"
BUILDID  = $(PLATFORM)$(PLATFORM_VER)-$(ARCHITEC)$(BIT_SIZE)-$(TOOLKIT)$(TOOLKIT_VER)p
else
BUILDID  = $(PLATFORM)$(PLATFORM_VER)-$(ARCHITEC)$(BIT_SIZE)-$(TOOLKIT)$(TOOLKIT_VER)
endif
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
TRDPARTY = ..

CCDEFS   = -m$(BIT_SIZE) $(ARCFLAGS) -D_MACOS -D_$(BIT_SIZE)BIT -Wall -I$(TRDPARTY)/zlib
CCFLAGSR = -O3 -flto -DNDEBUG=1 -Wno-uninitialized
CCFLAGSD = -g
LDDEFS   = -m$(BIT_SIZE) $(ARCFLAGS)
LDFLAGSR = -O3 -flto
LDFLAGSD = -g
ARFLAGSR = rcs
ARFLAGSD = rcs

ifeq "$(BCFG)" "debug"
OUTDIR   = $(OUTDIRD)
CCFLAGS  = $(CCFLAGSD)
LDFLAGS  = $(LDFLAGSD)
ARFLAGS  = $(ARFLAGSD)
INSTDIR  = 
INSTPRG  = - echo No install - debug 
else
OUTDIR   = $(OUTDIRR)
CCFLAGS  = $(CCFLAGSR)
LDFLAGS  = $(LDFLAGSR)
ARFLAGS  = $(ARFLAGSR)
INSTDIR  = ../../bin/$(BUILDID)
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
	$(LD) $(LDDEFS) $(LDPROF) $(LDFLAGS) -o $@ $(PRGOBJS) $(PRGLIBS)
	$(STRIP) $@
	$(INSTPRG) $@ $(INSTDIR)

$(PRGOBJS): $(PRGHDRS)

$(TRDPARTY)/zlib/$(OUTDIR)/zlib$(A):
	cd $(TRDPARTY)/zlib && $(MK) -f $(MAKEFILE).mak ARCHITEC=$(ARCHITEC) BCFG=$(BCFG)

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
