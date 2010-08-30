# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# win32v6.mak - Make file for Windows using Microsoft MSVC v6.0 development kit.
#
# Copyright (C) 2001-2009 JASSPA (www.jasspa.com)
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

BUILDID  = win32cygwin
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
A	 = .a
EXE	 = .exe
CC       = gcc
CCDEFS   = -D_CYGWIN -I../zlib
LD       = $(CC)
LDDEFS   = 
AR       = ar
ARFLAGS  = rcs
STRIP    = strip
RM       = rm -f
RMDIR    = rm -f -r

ifeq "$(BCFG)" "release"
OUTDIR   = $(OUTDIRR)
CCFLAGS  = $(CCDEFS) -O3 -DNDEBUG=1 -Wall -Wno-uninitialized
LDFLAGS  = $(LDDEFS) -L../zlib/$(OUTDIRR)
LDLIBS   = -lzlib
else
OUTDIR   = $(OUTDIRD)
CCFLAGS  = $(CCDEFS) -g -Wall
LDFLAGS  = $(LDDEFS) -L../zlib/$(OUTDIRD)
LDLIBS   = -lzlib
endif

LIBNAME  = tfs
LIBFILE  = lib$(LIBNAME)$(A)
LIBHDRS  = tfs.h win32cygwin.mak
LIBOBJS  = $(OUTDIR)/tfs.o

PRGNAME  = tfs
PRGFILE  = $(PRGNAME)
PRGHDRS  = tfs.h tfsutil.h win32cygwin.mak
PRGOBJS  = $(OUTDIR)/tfsutil.o $(OUTDIR)/tfs.o $(OUTDIR)/uappend.o $(OUTDIR)/ubuild.o $(OUTDIR)/ucopy.o \
	   $(OUTDIR)/ucreate.o $(OUTDIR)/uinfo.o $(OUTDIR)/ulist.o $(OUTDIR)/ustrip.o $(OUTDIR)/utest.o \
	   $(OUTDIR)/uxdir.o $(OUTDIR)/uxfile.o

$(OUTDIR)/%.o : %.c
	$(CC) $(CCFLAGS) -o $@ -c $<

all: $(OUTDIR)/$(LIBFILE) $(OUTDIR)/$(PRGFILE)

$(OUTDIR)/$(LIBFILE): $(OUTDIR) $(LIBOBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $(LIBOBJS)

$(LIBOBJS): $(LIBHDRS)

$(OUTDIR)/$(PRGFILE): $(OUTDIR) $(PRGOBJS)
	$(RM) $@$(EXE)
	$(LD) $(LDFLAGS) -o $@ $(PRGOBJS) $(LDLIBS)

$(PRGOBJS): $(PRGHDRS)

$(OUTDIR):
	@mkdir $(OUTDIR)

clean:
	$(RMDIR) $(OUTDIRR) $(OUTDIRD)

spotless:	clean
	$(RM) *~
	$(RM) tags
