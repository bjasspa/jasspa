# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# win32vc10.mak - Make file for Linux v5 Kernel using gcc
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

CC       = gcc-10
MK       = gmake
LD       = $(CC)
STRIP    = strip
AR       = ar
RM       = rm -f
RMDIR    = rm -rf

BUILDID  = sunosx86gcc
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
TRDPARTY = ..

ifneq (,$(OPENSSLP)) 
echo "preset"
OPENSSLLIBDIR=
# OpenIndiana has multiple SSL libraries use v3
else ifneq (,$(wildcard /usr/openssl/3/include/openssl/ssl.h) )
OPENSSLP = /usr/openssl/3/include
OPENSSLLIBDIR = /usr/openssl/3/lib
else ifneq (,$(wildcard /usr/include/openssl/ssl.h) )
OPENSSLP = /usr/include
OPENSSLLIBDIR = /usr/lib
else ifneq (, $(wildcard /usr/local/opt/openssl/include/openssl/ssl.h))
OPENSSLP = /usr/local/opt/openssl
OPENSSLLIBDIR = /usr/local/opt/openssl/lib
endif

ifeq (,$(OPENSSLP))
$(warning "WARNING: No OpenSSL support found, https support will be disabled.")
else
OPENSSLDEFS = -DMEOPT_OPENSSL=1 -I$(OPENSSLP) -D_OPENSSLLNM=libssl$(OPENSSLV).so -D_OPENSSLCNM=libcrypto$(OPENSSLV).so
endif

CCDEFS   = -D_SUNOS_X86 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -I. $(OPENSSLDEFS)
CCFLAGSR = -O3 -flto -DNDEBUG=1 -Wall -Wno-uninitialized
CCFLAGSD = -g -Wall
LDDEFS   = 
LDFLAGSR = -O3 -flto
LDFLAGSD = -g
ARFLAGSR = rcs
ARFLAGSD = rcs

#ifeq "$(BCFG)" "debug"
#OUTDIR   = $(OUTDIRD)
#CCFLAGS  = $(CCFLAGSD)
#LDFLAGS  = $(LDFLAGSD)
#ARFLAGS  = $(ARFLAGSD)
#else
OUTDIR   = $(OUTDIRR)
CCFLAGS  = $(CCFLAGSR)
LDFLAGS  = $(LDFLAGSR)
ARFLAGS  = $(ARFLAGSR)
#endif

LIBNAME  = mesock
LIBFILE  = $(LIBNAME)$(A)
LIBHDRS  = mesock.h $(BUILDID).mak
LIBOBJS  = $(OUTDIR)/mesock.o

PRGNAME  = mehttptest
PRGFILE  = $(PRGNAME)$(EXE)
PRGHDRS  = mesock.h $(BUILDID).mak
PRGOBJS  = $(OUTDIR)/mehttptest.o
PRGLIBS  = $(OUTDIR)/mesock$(A) -lsocket -lnsl

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
	-mkdir $(OUTDIR)

clean:
	$(RMDIR) .$(BUILDID)-*
	# $(RMDIR) $(OUTDIRD)
	# $(RMDIR) $(OUTDIRR)

spotless: clean
	$(RM) *~
	$(RM) tags
