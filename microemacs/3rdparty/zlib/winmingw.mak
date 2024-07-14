##############################################################################
#
#  Copyright (c) 2009 Maxexam Ltd.
# 
#  All Rights Reserved.
# 
#  This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
#  reproduced,  translated,  or  reduced to any  electronic  medium or machine
#  readable form without prior written consent from Maxexam Ltd.
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

ifneq "$(BIT_SIZE)" ""
else ifeq "$(PLATFORM)" "x64"
BIT_SIZE = 64
else
BIT_SIZE = 32
endif

TOOLKIT_VER = $(subst -win32,,$(word 1,$(subst ., ,$(shell $(CC) -dumpversion))))

PLATFORM = windows
TOOLKIT  = mingw
ARCHITEC = intel
MAKEFILE = win$(TOOLKIT)
ifeq "$(BPRF)" "1"
BUILDID  = $(PLATFORM)-$(ARCHITEC)$(BIT_SIZE)$(TOOLKIT)$(TOOLKIT_VER)p
else
BUILDID  = $(PLATFORM)-$(ARCHITEC)$(BIT_SIZE)$(TOOLKIT)$(TOOLKIT_VER)
endif
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
TRDPARTY = ..

CCDEFS   = -m$(BIT_SIZE) -D_MINGW -D_$(BIT_SIZE)BIT -Wall
CCFLAGSR = -O3 -flto -DNDEBUG=1 -Wno-uninitialized
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
else
OUTDIR   = $(OUTDIRR)
CCFLAGS  = $(CCFLAGSR)
LDFLAGS  = $(LDFLAGSR)
ARFLAGS  = $(ARFLAGSR)
endif

ifeq "$(BPRF)" "1"
CCPROF = -D_ME_PROFILE -pg -no-pie
LDPROF = -pg -no-pie
STRIP  = - echo No strip - profile 
else
CCPROF = 
LDPROF = 
endif

LIBNAME  = zlib
LIBFILE  = $(LIBNAME)$(A)
LIBHDRS  = zutil.h zlib.h zconf.in.h zconf.h trees.h inftrees.h inflate.h \
	   inffixed.h inffast.h deflate.h crc32.h $(MAKEFILE).mak
LIBOBJS  = $(OUTDIR)/adler32.o $(OUTDIR)/compress.o $(OUTDIR)/crc32.o $(OUTDIR)/gzio.o $(OUTDIR)/uncompr.o \
	   $(OUTDIR)/deflate.o $(OUTDIR)/trees.o $(OUTDIR)/zutil.o $(OUTDIR)/inflate.o $(OUTDIR)/infback.o \
	   $(OUTDIR)/inftrees.o $(OUTDIR)/inffast.o

.SUFFIXES: .c .o

$(OUTDIR)/%.o : %.c
	$(CC) $(CCDEFS) $(CCFLAGS) $(CCPROF) -c -o $@ $<

all: $(OUTDIR)/$(LIBFILE)

$(OUTDIR)/$(LIBFILE): $(OUTDIR) $(LIBOBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $(LIBOBJS)

$(LIBOBJS): $(LIBHDRS)

$(OUTDIR):
	-mkdir $(OUTDIR)

clean:
	$(RMDIR) $(OUTDIRD)
	$(RMDIR) $(OUTDIRR)

spotless: clean
	$(RM) *~
	$(RM) tags
