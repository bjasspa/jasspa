# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
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

CC       = gcc
RC       = windres
MK       = mingw32-make
LD       = $(CC)
STRIP    = strip
AR       = ar
RM       = rm -f
RMDIR    = rm -rf

TOOLKIT  = win32mingw
ifeq "$(BPRF)" "1"
BUILDID  = $(TOOLKIT)p
else
BUILDID  = $(TOOLKIT)
endif
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
TRDPARTY = ..
ifneq "$(OPENSSLP)" ""
else ifneq "$(wildcard $(TRDPARTY)/openssl-3.1/x86/include/openssl/ssl.h)" ""
OPENSSLP = $(TRDPARTY)/openssl-3.1/x86
OPENSSLV = -3_1
else ifneq "$(wildcard $(TRDPARTY)/openssl-3/x86/include/openssl/ssl.h)" ""
OPENSSLP = $(TRDPARTY)/openssl-3/x86
OPENSSLV = -3
else ifneq "$(wildcard $(TRDPARTY)/openssl-1.1/x86/include/openssl/ssl.h)" ""
OPENSSLP = $(TRDPARTY)/openssl-1.1/x86
OPENSSLV = -1_1
endif
ifeq "$(OPENSSLP)" ""
$(warning WARNING: No OpenSSL support found, https support will be disabled.)
else
OPENSSLDEFS = -DMEOPT_OPENSSL=1 -I$(OPENSSLP)/include -D_OPENSSLLNM=libssl$(OPENSSLV) -D_OPENSSLCNM=libcrypto$(OPENSSLV)
OPENSSLLIBS = $(OPENSSLP)/lib/libssl.lib $(OPENSSLP)/lib/libcrypto.lib
endif

CCDEFS   = -D_MINGW -Wall $(OPENSSLDEFS)
CCFLAGSR = -O3 -DNDEBUG=1 -Wno-uninitialized
CCFLAGSD = -g
LDDEFS   = 
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

ifeq "$(BPRF)" "1"
CCPROF = -D_ME_PROFILE -pg -no-pie
LDPROF = -pg -no-pie
STRIP  = - echo No strip - profile 
else
CCPROF = 
LDPROF = 
endif

LIBNAME  = mesock
LIBFILE  = $(LIBNAME)$(A)
LIBHDRS  = mesock.h $(TOOLKIT).mak
LIBOBJS  = $(OUTDIR)/mesock.o

PRGHDRS  = mesock.h $(TOOLKIT).mak
PRGLIBS  = $(OUTDIR)/mesock$(A) $(OPENSSLLIBS)
SYSLIBS  = -lcrypt32 -lws2_32

PRGNAME1 = mehttptest
PRGFILE1 = $(PRGNAME1)$(EXE)
PRGOBJS1 = $(OUTDIR)/mehttptest.o

PRGNAME2 = meftptest
PRGFILE2 = $(PRGNAME2)$(EXE)
PRGOBJS2 = $(OUTDIR)/meftptest.o

.SUFFIXES: .c .o

$(OUTDIR)/%.o : %.c
	$(CC) $(CCDEFS) $(CCPROF) $(CCFLAGS) -c -o $@ $<

all: $(PRGLIBS) $(OUTDIR)/$(LIBFILE) $(OUTDIR)/$(PRGFILE1) $(OUTDIR)/$(PRGFILE2)

$(OUTDIR)/$(LIBFILE): $(OUTDIR) $(LIBOBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $(LIBOBJS)

$(LIBOBJS): $(LIBHDRS)

$(OUTDIR)/$(PRGFILE1): $(OUTDIR) $(PRGOBJS1) $(PRGLIBS)
	$(RM) $@
	$(LD) $(LDFLAGS) $(LDPROF) -o $@ $(PRGOBJS1) $(PRGLIBS) $(SYSLIBS)

$(PRGOBJS1): $(PRGHDRS)

$(OUTDIR)/$(PRGFILE2): $(OUTDIR) $(PRGOBJS2) $(PRGLIBS)
	$(RM) $@
	$(LD) $(LDFLAGS) $(LDPROF) -o $@ $(PRGOBJS2) $(PRGLIBS) $(SYSLIBS)

$(PRGOBJS2): $(PRGHDRS)

$(OUTDIR):
	-mkdir $(OUTDIR)

clean:
	$(RMDIR) $(OUTDIRD)
	$(RMDIR) $(OUTDIRR)

spotless: clean
	$(RM) *~
	$(RM) tags
