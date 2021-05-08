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

BUILDID  = macos64cc
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
TRDPARTY = ..
OPENSSLP = /usr/local/opt/openssl@1.1

CCDEFS   = -D_OSX -m64 -Wall -I$(OPENSSLP)/include
CCFLAGSR = -O3 -DNDEBUG=1 -Wno-uninitialized
CCFLAGSD = -g
LDDEFS   = -m64
LDFLAGSR = -O3
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

LIBNAME  = messl
LIBFILE  = $(LIBNAME)$(A)
LIBHDRS  = messl.h $(BUILDID).mak
LIBOBJS  = $(OUTDIR)/messl.o

PRGNAME  = messltest
PRGFILE  = $(PRGNAME)$(EXE)
PRGHDRS  = messl.h $(BUILDID).mak
PRGOBJS  = $(OUTDIR)/messltest.o $(OUTDIR)/messl.o

.SUFFIXES: .c .o

$(OUTDIR)/%.o : %.c
	$(CC) $(CCDEFS) $(CCFLAGS) -c -o $@ $<

all: $(PRGLIBS) $(OUTDIR)/$(LIBFILE) $(OUTDIR)/$(PRGFILE)

$(OUTDIR)/$(LIBFILE): $(OUTDIR) $(LIBOBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $(LIBOBJS)

$(LIBOBJS): $(LIBHDRS)

$(OUTDIR)/$(PRGFILE): $(OUTDIR) $(PRGOBJS) $(PRGLIBS)
	$(RM) $@
	$(LD) $(LDDEFS) $(LDFLAGS) -o $@ $(PRGOBJS) $(PRGLIBS)

$(PRGOBJS): $(PRGHDRS)

$(OUTDIR):
	[ -d $(OUTDIR) ] || mkdir $(OUTDIR)

clean:
	$(RMDIR) $(OUTDIRD)
	$(RMDIR) $(OUTDIRR)

spotless: clean
	$(RM) *~
	$(RM) tags
