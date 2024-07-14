##############################################################################
#
#  Copyright (c) 2009 Jasspa
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

ifeq "$(BIT_SIZE)" ""
BIT_SIZE = $(shell getconf LONG_BIT)
endif

PLATFORM = macos
TOOLKIT  = cc
ifneq "$(ARCHITEC)" ""
else ifeq "$(shell uname -p)" "i386"
ARCHITEC = intel
else
ARCHITEC = apple
endif
MAKEFILE = $(PLATFORM)$(TOOLKIT)
ifeq "$(BPRF)" "1"
BUILDID  = $(PLATFORM)-$(ARCHITEC)$(BIT_SIZE)$(TOOLKIT)p
else
BUILDID  = $(PLATFORM)-$(ARCHITEC)$(BIT_SIZE)$(TOOLKIT)
endif
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
TRDPARTY = ..

CCDEFS   = -m$(BIT_SIZE) -D_MACOS -D_$(BIT_SIZE)BIT -Wall
CCFLAGSR = -O3 -flto -DNDEBUG=1 -Wno-uninitialized
CCFLAGSD = -g
LDDEFS   = -m$(BIT_SIZE)
LDFLAGSR = -O3 -flto
LDFLAGSD = -g
ARFLAGSR = rcs
ARFLAGSD = rcs

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

LIBNAME  = zlib
LIBFILE  = $(LIBNAME)$(A)
LIBHDRS  = zutil.h zlib.h zconf.in.h zconf.h trees.h inftrees.h inflate.h \
	   inffixed.h inffast.h deflate.h crc32.h $(MAKEFILE).mak
LIBOBJS  = $(OUTDIR)/adler32.o $(OUTDIR)/compress.o $(OUTDIR)/crc32.o $(OUTDIR)/gzio.o $(OUTDIR)/uncompr.o \
	   $(OUTDIR)/deflate.o $(OUTDIR)/trees.o $(OUTDIR)/zutil.o $(OUTDIR)/inflate.o $(OUTDIR)/infback.o \
	   $(OUTDIR)/inftrees.o $(OUTDIR)/inffast.o

.SUFFIXES: .c .o

$(OUTDIR)/%.o : %.c
	$(CC) $(CCDEFS) $(CCFLAGS) -c -o $@ $<

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
