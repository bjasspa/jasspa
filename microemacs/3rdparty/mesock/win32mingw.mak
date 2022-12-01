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

BUILDID  = win32mingw
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
TRDPARTY = ..
ifeq "$(OPENSSLP)" ""
OPENSSLP = $(TRDPARTY)/openssl-1.1/x86
endif
ifeq "$(wildcard $(OPENSSLP)/include/.)" ""
$(warning WARNING: No OpenSSL support found, https support will be disabled.)
else
OPENSSLDEFS = -DMEOPT_OPENSSL=1 -I$(OPENSSLP)/include -D_OPENSSLLNM=libssl-1_1 -D_OPENSSLCNM=libcrypto-1_1
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

LIBNAME  = mesock
LIBFILE  = $(LIBNAME)$(A)
LIBHDRS  = mesock.h $(BUILDID).mak
LIBOBJS  = $(OUTDIR)/mesock.o

PRGHDRS  = mesock.h $(BUILDID).mak
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
	$(CC) $(CCDEFS) $(CCFLAGS) -c -o $@ $<

all: $(PRGLIBS) $(OUTDIR)/$(LIBFILE) $(OUTDIR)/$(PRGFILE1) $(OUTDIR)/$(PRGFILE2)

$(OUTDIR)/$(LIBFILE): $(OUTDIR) $(LIBOBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $(LIBOBJS)

$(LIBOBJS): $(LIBHDRS)

$(OUTDIR)/$(PRGFILE1): $(OUTDIR) $(PRGOBJS1) $(PRGLIBS)
	$(RM) $@
	$(LD) $(LDFLAGS) -o $@ $(PRGOBJS1) $(PRGLIBS) $(SYSLIBS)

$(PRGOBJS1): $(PRGHDRS)

$(OUTDIR)/$(PRGFILE2): $(OUTDIR) $(PRGOBJS2) $(PRGLIBS)
	$(RM) $@
	$(LD) $(LDFLAGS) -o $@ $(PRGOBJS2) $(PRGLIBS) $(SYSLIBS)

$(PRGOBJS2): $(PRGHDRS)

$(TRDPARTY)/zlib/$(OUTDIR)/zlib$(A):
	cd $(TRDPARTY)/zlib && $(MK) -f $(BUILDID).mak BCFG=$(BCFG)

$(OUTDIR):
	if not exist $(OUTDIR)\ mkdir $(OUTDIR)

clean:
	$(RMDIR) $(OUTDIRD)
	$(RMDIR) $(OUTDIRR)

spotless: clean
	$(RM) *~
	$(RM) tags
