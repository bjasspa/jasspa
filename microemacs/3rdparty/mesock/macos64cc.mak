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
# If the program ends up trying to load the default system ssl libraries it is likely to trigger a runtime '<prg> is loading libcrypto in an unsafe way' abort!!
# Avoid this by trying to use a versioned library, i.e. libssl.3.dylib as this will lead to a relatively safe 'lib not found' (no crash)
# But better still set the DYLD_FALLBACK_LIBRARY_PATH appropriately!
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

CCDEFS   = -D_MACOS -D_64BIT -m64 -Wall $(OPENSSLDEFS)
CCFLAGSR = -O3 -flto -DNDEBUG=1 -Wno-uninitialized
CCFLAGSD = -g
LDDEFS   = -m64
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

LIBNAME  = mesock
LIBFILE  = $(LIBNAME)$(A)
LIBHDRS  = mesock.h $(BUILDID).mak
LIBOBJS  = $(OUTDIR)/mesock.o

PRGHDRS  = mesock.h $(BUILDID).mak
PRGLIBS  = $(OUTDIR)/mesock$(A)

PRGNAME1 = mehttptest
PRGFILE1 = $(PRGNAME1)$(EXE)
PRGOBJS1 = $(OUTDIR)/mehttptest.o

PRGNAME2 = meftptest
PRGFILE2 = $(PRGNAME2)$(EXE)
PRGOBJS2 = $(OUTDIR)/meftptest.o

.SUFFIXES: .c .o

$(OUTDIR)/%.o : %.c
	$(CC) $(CCDEFS) $(CCFLAGS) -c -o $@ $<

all: $(PRGLIBS) $(OUTDIR)/$(LIBFILE) $(OUTDIR)/$(PRGFILE1) $(OUTDIR)/$(PRGFILE2)

$(OUTDIR)/$(LIBFILE): $(OUTDIR) $(LIBOBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $(LIBOBJS)

$(LIBOBJS): $(LIBHDRS)

$(OUTDIR)/$(PRGFILE1): $(OUTDIR) $(PRGOBJS1) $(PRGLIBS)
	$(RM) $@
	$(LD) $(LDDEFS) $(LDFLAGS) -o $@ $(PRGOBJS1) $(PRGLIBS)

$(PRGOBJS1): $(PRGHDRS)

$(OUTDIR)/$(PRGFILE2): $(OUTDIR) $(PRGOBJS2) $(PRGLIBS)
	$(RM) $@
	$(LD) $(LDDEFS) $(LDFLAGS) -o $@ $(PRGOBJS2) $(PRGLIBS)

$(PRGOBJS2): $(PRGHDRS)

$(OUTDIR):
	[ -d $(OUTDIR) ] || mkdir $(OUTDIR)

clean:
	$(RMDIR) $(OUTDIRD)
	$(RMDIR) $(OUTDIRR)

spotless: clean
	$(RM) *~
	$(RM) tags
