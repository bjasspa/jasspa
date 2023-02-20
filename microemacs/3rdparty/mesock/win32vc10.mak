# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# win32vc10.mak - Make file for Windows using Microsoft MSVC v10 development kit.
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

TOOLKIT  = win32vc10
!IF "$(LSTT)" == "1"
BUILDID  = $(TOOLKIT)s
CCLSTT   = /MT
LDLSTT   = /NODEFAULTLIB:msvcrt.lib
!ELSE
BUILDID  = $(TOOLKIT)
CCLSTT   = /MD
LDLSTT   = 
!ENDIF
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
TRDPARTY = ..

!IF "$(OPENSSLP)" == ""
OPENSSLP = $(TRDPARTY)\openssl-1.1\x86
!ENDIF
!IF EXISTS($(OPENSSLP)\include)
OPENSSLDEFS = /DMEOPT_OPENSSL=1 /I$(OPENSSLP)\include /D_OPENSSLLNM=libssl-1_1 /D_OPENSSLCNM=libcrypto-1_1
OPENSSLLIBS = $(OPENSSLP)\lib\libssl.lib $(OPENSSLP)\lib\libcrypto.lib
!ELSE
!MESSAGE WARNING: No OpenSSL support found, https support will be disabled.
!MESSAGE
!ENDIF

CCDEFS   = /DWIN32 /D_WIN32 /D_WIN32_WINNT=0x0600 /W3 /Zi /D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE $(OPENSSLDEFS)
CCFLAGSR = $(CCLSTT) /O2 /GL /GR- /GS- /DNDEBUG=1 /D_HAS_ITERATOR_DEBUGGING=0 /D_SECURE_SCL=0
CCFLAGSD = $(CCLSTT)d /Od /RTC1 /D_DEBUG
LDDEFS   = /SUBSYSTEM:CONSOLE /INCREMENTAL:NO /MACHINE:X86 /MANIFEST $(LDLSTT)
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

LIBNAME  = mesock
LIBFILE  = $(LIBNAME)$(A)
LIBHDRS  = mesock.h $(TOOLKIT).mak
LIBOBJS  = $(OUTDIR)\mesock.o

PRGHDRS  = mesock.h $(TOOLKIT).mak
PRGLIBS  = $(OUTDIR)\mesock$(A) $(OPENSSLLIBS)
SYSLIBS  = crypt32.lib ws2_32.lib

PRGNAME1 = mehttptest
PRGFILE1 = $(PRGNAME1)$(EXE)
PRGOBJS1 = $(OUTDIR)\mehttptest.o

PRGNAME2 = meftptest
PRGFILE2 = $(PRGNAME2)$(EXE)
PRGOBJS2 = $(OUTDIR)\meftptest.o

.SUFFIXES: .c .o .rc .res

.c{$(OUTDIR)}.o:
	$(CC) $(CCDEFS) $(BCOR_CDF) $(BTYP_CDF) $(CCFLAGS) /Fd"$(OUTDIR)\vc100.pdb" /c $< /Fo"$@"

all: $(PRGLIBS) $(OUTDIR)\$(LIBFILE) $(OUTDIR)\$(PRGFILE1) $(OUTDIR)\$(PRGFILE2)

$(OUTDIR)\$(LIBFILE): $(OUTDIR) $(LIBOBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) /OUT:"$@" $(LIBOBJS)

$(LIBOBJS): $(LIBHDRS)

$(OUTDIR)\$(PRGFILE1): $(OUTDIR) $(PRGOBJS1) $(PRGLIBS)
	$(RM) $@
	$(LD) $(LDDEFS) $(LDFLAGS) /PDB:"$(OUTDIR)\$(PRGNAME1).pdb" /MANIFESTFILE:"$@.intermediate.manifest" /OUT:"$@" $(PRGOBJS1) $(PRGLIBS) $(SYSLIBS)
	$(MT) -outputresource:"$@;#2" -manifest $@.intermediate.manifest

$(PRGOBJS1): $(PRGHDRS)

$(OUTDIR)\$(PRGFILE2): $(OUTDIR) $(PRGOBJS2) $(PRGLIBS)
	$(RM) $@
	$(LD) $(LDDEFS) $(LDFLAGS) /PDB:"$(OUTDIR)\$(PRGNAME2).pdb" /MANIFESTFILE:"$@.intermediate.manifest" /OUT:"$@" $(PRGOBJS2) $(PRGLIBS) $(SYSLIBS)
	$(MT) -outputresource:"$@;#2" -manifest $@.intermediate.manifest

$(PRGOBJS2): $(PRGHDRS)

$(OUTDIR):
	if not exist $(OUTDIR)\ mkdir $(OUTDIR)

clean:
	if exist $(OUTDIRD)\ $(RMDIR) $(OUTDIRD)
	if exist $(OUTDIRR)\ $(RMDIR) $(OUTDIRR)

spotless: clean
	$(RM) *~
	$(RM) tags
