# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# win32vc10.mak - Make file for Windows using Microsoft MSVC v10.0 development kit.
#
# Copyright (C) 2007-2009 JASSPA (www.jasspa.com)
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
#
# Created:     Sat Jan 24 1998
# Synopsis:    Make file for Windows using Microsoft MSVC v10.0 development kit.
# Notes:
#     Run .\build.bat to compile, build.bat -h for more information.
#
#     To build from the command line using nmake & makefile. 
#
#	Run "nmake -f win32vc15.mak"            for optimised build produces ./.win32vc15-release-mew/mew32.exe
#	Run "nmake -f win32vc15.mak BCFG=debug" for debug build produces     ./.win32vc15-debug-mew/mew32.exe
#	Run "nmake -f win32vc15.mak BCOR=ne"    for ne build produces        ./.win32vc15-release-new/new32.exe
#
#	Run "nmake -f win32vc15.mak clean"      to clean source directory
#	Run "nmake -f win32vc15.mak spotless"   to clean source directory even more
#
# See https://wiki.openssl.org/index.php/Binaries for OpenSSL download
#
#   Windows: Used FireDaemon (https://kb.firedaemon.com/support/solutions/articles/4000121705)
#
##############################################################################
#

A        = .lib
EXE      = .exe

CC       = cl.exe /nologo
RC       = rc.exe
MT       = mt.exe -nologo
MK       = nmake.exe /NOLOGO
LD       = link /NOLOGO
AR       = lib /NOLOGO
RM       = del /F /Q
RMDIR    = rd /S /Q

BUILDID  = win32vc15
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
TRDPARTY = ..
OPENSSLP = $(TRDPARTY)\openssl-1.1\x86
OPENSSLL = libssl-1_1
OPENSSLC = libcrypto-1_1

CCDEFS   = /DWIN32 /D_WIN32 /D_WIN32_WINNT=0x500 /W3 /Zi /D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE /I$(OPENSSLP)\include /D_OPENSSLLNM=$(OPENSSLL) /D_OPENSSLCNM=$(OPENSSLC)
CCFLAGSR = /MD /O2 /GL /GS- /DNDEBUG=1 /D_HAS_ITERATOR_DEBUGGING=0 /D_SECURE_SCL=0
CCFLAGSD = /MDd /Od /RTC1 /D_DEBUG
LDDEFS   = /SUBSYSTEM:CONSOLE /INCREMENTAL:NO /MACHINE:X86 /MANIFEST
LDFLAGSR = /OPT:REF /OPT:ICF /LTCG
LDFLAGSD = /DEBUG
ARFLAGSR = /LTCG
ARFLAGSD =
RCFLAGS  =

!IF "$(BCFG)" == "debug"
OUTDIR   = $(OUTDIRD)
CCFLAGS  = $(CCFLAGSD)
LDFLAGS  = $(LDFLAGSD)
ARFLAGS  = $(ARFLAGSD)
!ELSE
OUTDIR   = $(OUTDIRR)
CCFLAGS  = $(CCFLAGSR)
LDFLAGS  = $(LDFLAGSR)
ARFLAGS  = $(ARFLAGSR)
!ENDIF

LIBNAME  = messl
LIBFILE  = $(LIBNAME)$(A)
LIBHDRS  = messl.h $(BUILDID).mak
LIBOBJS  = $(OUTDIR)\messl.o

PRGNAME  = messltest
PRGFILE  = $(PRGNAME)$(EXE)
PRGHDRS  = messl.h $(BUILDID).mak
PRGOBJS  = $(OUTDIR)\messltest.o
PRGLIBS  = $(OUTDIR)\messl$(A) $(OPENSSLP)\lib\libssl.lib $(OPENSSLP)\lib\libcrypto.lib
SYSLIBS  = crypt32.lib ws2_32.lib

.SUFFIXES: .c .o .rc .res

.c{$(OUTDIR)}.o:
	$(CC) $(CCDEFS) $(BCOR_CDF) $(BTYP_CDF) $(CCFLAGS) /Fd"$(OUTDIR)\vc150.pdb" /c $< /Fo"$@"

all: $(PRGLIBS) $(OUTDIR)\$(LIBFILE) $(OUTDIR)\$(PRGFILE)

$(OUTDIR)\$(LIBFILE): $(OUTDIR) $(LIBOBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) /OUT:"$@" $(LIBOBJS)

$(LIBOBJS): $(LIBHDRS)

$(OUTDIR)\$(PRGFILE): $(OUTDIR) $(PRGOBJS) $(PRGLIBS)
	$(RM) $@
	$(LD) $(LDDEFS) $(LDFLAGS) /PDB:"$(OUTDIR)\$(PRGNAME).pdb" /MANIFESTFILE:"$@.intermediate.manifest" /OUT:"$@" $(PRGOBJS) $(PRGLIBS) $(SYSLIBS)
	$(MT) -outputresource:"$@;#2" -manifest $@.intermediate.manifest

$(PRGOBJS): $(PRGHDRS)

$(OUTDIR):
	if not exist $(OUTDIR)\ mkdir $(OUTDIR)

clean:
	if exist $(OUTDIRD)\ $(RMDIR) $(OUTDIRD)
	if exist $(OUTDIRR)\ $(RMDIR) $(OUTDIRR)

spotless: clean
	$(RM) *~
	$(RM) tags
