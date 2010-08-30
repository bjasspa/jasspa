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

BUILDID  = win32vs6
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
A        = .lib
EXE      = .exe
CC       = cl
CCDEFS   = -nologo -DWIN32 -D_WIN32 -D_WIN32_WINNT=0x500 -I../zlib
LD       = link
LDDEFS   = -nologo -subsystem:console -incremental:no -machine:I386
AR       = lib
ARFLAGS  = -nologo
RC       = rc
RCFLAGS  = 
RM       = rm.exe -f
RMDIR    = rm.exe -f -r

!IF "$(BCFG)" == "release"
OUTDIR   = $(OUTDIRR)
CCFLAGS  = $(CCDEFS) -MD -GX -YX -O2 -W3 -DNDEBUG=1 -Fp$(OUTDIRR)/vc50.pch
LDFLAGS  = $(LDDEFS) -libpath:../zlib/$(OUTDIRR)
LDLIBS   = zlib.lib
!ELSE
OUTDIR   = $(OUTDIRD)
CCFLAGS  = $(CCDEFS) -MDd -Z7 -GZ -GX -Yd -YX -Od -W3 -D_DEBUG -Fp$(OUTDIRD)/vc50.pch
LDFLAGS  = $(LDDEFS) -libpath:../zlib/$(OUTDIRD)
LDLIBS   = zlib.lib
!ENDIF

LIBNAME  = tfs
LIBFILE  = $(LIBNAME)$(A)
LIBHDRS  = tfs.h win32vs6.mak
LIBOBJS  = $(OUTDIR)/tfs.o

PRGNAME  = tfs
PRGFILE  = $(PRGNAME)$(EXE)
PRGHDRS  = tfs.h tfsutil.h win32vs6.mak
PRGOBJS  = $(OUTDIR)/tfsutil.o $(OUTDIR)/tfs.o $(OUTDIR)/uappend.o $(OUTDIR)/ubuild.o $(OUTDIR)/ucopy.o \
	   $(OUTDIR)/ucreate.o $(OUTDIR)/uinfo.o $(OUTDIR)/ulist.o $(OUTDIR)/ustrip.o $(OUTDIR)/utest.o \
	   $(OUTDIR)/uxdir.o $(OUTDIR)/uxfile.o

.SUFFIXES: .c .o .rc .res

.c{$(OUTDIR)}.o:
	$(CC) $(CCFLAGS) -c $< -Fo$@

.rc{$(OUTDIR)}.res:
	$(RC) $(RCFLAGS) -Fo $@ $<

all: $(OUTDIR)/$(LIBFILE) $(OUTDIR)/$(PRGFILE)

$(OUTDIR)/$(LIBFILE): $(OUTDIR) $(LIBOBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) -out:$@ $(LIBOBJS)

$(LIBOBJS): $(LIBHDRS)

$(OUTDIR)/$(PRGFILE): $(OUTDIR) $(PRGOBJS)
	$(RM) $@
	$(LD) $(LDFLAGS) -out:$@ $(PRGOBJS) $(LDLIBS)

$(PRGOBJS): $(PRGHDRS)

$(OUTDIR):
	if not exist $(OUTDIR)/ mkdir $(OUTDIR)

clean:
	$(RMDIR) $(OUTDIRR) $(OUTDIRD)

spotless:	clean
	$(RM) *~
	$(RM) tags
