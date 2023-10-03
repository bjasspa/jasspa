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
#	Run "nmake -f win32vc15.mak BTYP=c"     for console support          ./.win32vc15-release-mec/mec32.exe
#	Run "nmake -f win32vc15.mak BCOR=ne"    for ne build produces        ./.win32vc15-release-new/new32.exe
#
#	Run "nmake -f win32vc15.mak clean"      to clean source directory
#	Run "nmake -f win32vc15.mak spotless"   to clean source directory even more
#
##############################################################################
#
# Installation Directory
INSTDIR	      = c:\emacs
INSTPROGFLAGS = 
#
# Local Definitions
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

include evers.mak

TOOLKIT  = win32vc15
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
TRDPARTY = ..\3rdparty

CCDEFS   = /DmeVER_CN=$(meVER_CN) /DmeVER_YR=$(meVER_YR) /DmeVER_MN=$(meVER_MN) /DmeVER_DY=$(meVER_DY) /DWIN32 /D_WIN32 /D_WIN32_WINNT=0x0600 /W3 /Zi /D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE /I$(TRDPARTY)\tfs /I$(TRDPARTY)\zlib
CCFLAGSR = $(CCLSTT) /O2 /GL /GR- /GS- /DNDEBUG=1 /D_HAS_ITERATOR_DEBUGGING=0 /D_SECURE_SCL=0
CCFLAGSD = $(CCLSTT)d /Od /RTC1 /D_DEBUG
LDDEFS   = /INCREMENTAL:NO /MACHINE:X86 /MANIFEST $(LDLSTT)
LDFLAGSR = /OPT:REF /OPT:ICF=3 /LTCG
LDFLAGSD = /DEBUG
LDLIBSB  = shell32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib
ARFLAGSR = /LTCG
ARFLAGSD =
RCFLAGS  = /DmeVER_CN=$(meVER_CN) /DmeVER_YR=$(meVER_YR) /DmeVER_MN=$(meVER_MN) /DmeVER_DY=$(meVER_DY)

!IF "$(BCFG)" == "debug"
BOUTDIR  = $(OUTDIRD)
CCFLAGS  = $(CCFLAGSD)
LDFLAGS  = $(LDFLAGSD)
ARFLAGS  = $(ARFLAGSD)
!ELSE
BOUTDIR  = $(OUTDIRR)
CCFLAGS  = $(CCFLAGSR)
LDFLAGS  = $(LDFLAGSR)
ARFLAGS  = $(ARFLAGSR)
!ENDIF

!IF "$(BCOR)" == "ne"

BCOR_CDF = /D_NANOEMACS
PRGLIBS  = 
LDLIBS   = $(LDLIBSB)

!ELSE

!IF "$(OPENSSLP)" != ""
!ELSEIF EXISTS($(TRDPARTY)\openssl-3.1\x86\include\openssl\ssl.h)
OPENSSLP = $(TRDPARTY)\openssl-3.1\x86
OPENSSLV = -3_1
!ELSEIF EXISTS($(TRDPARTY)\openssl-3\x86\include\openssl\ssl.h)
OPENSSLP = $(TRDPARTY)\openssl-3\x86
OPENSSLV = -3
!ELSEIF EXISTS($(TRDPARTY)\openssl-1.1\x86\include\openssl\ssl.h)
OPENSSLP = $(TRDPARTY)\openssl-1.1\x86
OPENSSLV = -1_1
!ENDIF
!IF "$(OPENSSLP)" == ""
!MESSAGE WARNING: No OpenSSL support found, https support will be disabled.
!MESSAGE
!ELSE
OPENSSLDEFS = /DMEOPT_OPENSSL=1 /I$(OPENSSLP)\include /D_OPENSSLLNM=libssl$(OPENSSLV) /D_OPENSSLCNM=libcrypto$(OPENSSLV)
OPENSSLLIBS = $(OPENSSLP)\lib\libssl.lib $(OPENSSLP)\lib\libcrypto.lib crypt32.lib
!ENDIF
BCOR     = me
BCOR_CDF = /D_SOCKET $(OPENSSLDEFS)
PRGLIBS  = $(TRDPARTY)\tfs\$(BOUTDIR)\tfs.lib $(TRDPARTY)\zlib\$(BOUTDIR)\zlib.lib
LDLIBS   = $(OPENSSLLIBS) ws2_32.lib mpr.lib $(LDLIBSB)

!ENDIF

!IF "$(BTYP)" == "c"
BTYP_CDF = /D_ME_CONSOLE /D_CONSOLE
BTYP_LDF = /SUBSYSTEM:CONSOLE
!ELSE IF "$(BTYP)" == "cw"
BTYP_CDF = /D_ME_CONSOLE /D_CONSOLE /D_ME_WINDOW
BTYP_LDF = /SUBSYSTEM:CONSOLE
!ELSE
BTYP_CDF = /D_ME_WINDOW
BTYP_LDF = /SUBSYSTEM:WINDOWS
BTYP     = w
!ENDIF

OUTDIR   = $(BOUTDIR)-$(BCOR)$(BTYP)
PRGNAME  = $(BCOR)$(BTYP)32
PRGFILE  = $(PRGNAME)$(EXE)
PRGHDRS  = ebind.h edef.h eextrn.h efunc.h emain.h emode.h eprint.h esearch.h eskeys.h estruct.h eterm.h evar.h evers.h eopt.h \
	   ebind.def efunc.def eprint.def evar.def etermcap.def emode.def eskeys.def \
	   $(TOOLKIT).mak evers.mak
PRGOBJS  = $(OUTDIR)\abbrev.o $(OUTDIR)\basic.o $(OUTDIR)\bind.o $(OUTDIR)\buffer.o $(OUTDIR)\crypt.o $(OUTDIR)\dirlist.o $(OUTDIR)\display.o \
	   $(OUTDIR)\eval.o $(OUTDIR)\exec.o $(OUTDIR)\file.o $(OUTDIR)\fileio.o $(OUTDIR)\frame.o $(OUTDIR)\hash.o $(OUTDIR)\hilight.o $(OUTDIR)\history.o \
	   $(OUTDIR)\input.o $(OUTDIR)\isearch.o $(OUTDIR)\key.o $(OUTDIR)\line.o $(OUTDIR)\macro.o $(OUTDIR)\main.o $(OUTDIR)\narrow.o $(OUTDIR)\next.o \
	   $(OUTDIR)\osd.o $(OUTDIR)\print.o $(OUTDIR)\random.o $(OUTDIR)\regex.o $(OUTDIR)\region.o $(OUTDIR)\registry.o $(OUTDIR)\search.o $(OUTDIR)\sock.o \
	   $(OUTDIR)\spawn.o $(OUTDIR)\spell.o $(OUTDIR)\tag.o $(OUTDIR)\termio.o $(OUTDIR)\time.o $(OUTDIR)\undo.o $(OUTDIR)\window.o $(OUTDIR)\word.o \
	   $(OUTDIR)\winterm.o $(OUTDIR)\winprint.o $(OUTDIR)\$(BCOR).res
#
# Rules
.SUFFIXES: .c .o .rc .res

.c{$(OUTDIR)}.o:
	$(CC) $(CCDEFS) $(BCOR_CDF) $(BTYP_CDF) $(CCFLAGS) /Fd"$(OUTDIR)\vc150.pdb" /c $< /Fo"$@"

.rc{$(OUTDIR)}.res:
	$(RC) $(RCFLAGS) $(BTYP_CDF) /fo"$@" $<

all: $(PRGLIBS) $(OUTDIR)\$(PRGFILE)

$(OUTDIR)\$(PRGFILE): $(OUTDIR) $(PRGOBJS) $(PRGLIBS) $(TOOLKIT).mak
	$(RM) $@
	$(LD) $(LDDEFS) $(BTYP_LDF) $(LDFLAGS) /PDB:"$(OUTDIR)\$(PRGNAME).pdb" /OUT:"$@" $(PRGOBJS) $(PRGLIBS) $(LDLIBS)
	(echo ^<?xml version="1.0" encoding="UTF-8" standalone="yes"?^> & echo ^<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0"^> & echo ^<assemblyIdentity version="$(meVER_YR).$(meVER_MN).$(meVER_DY).0" processorArchitecture="X86" name="mecw32.exe" type="win32"/^> & echo ^<trustInfo xmlns="urn:schemas-microsoft-com:asm.v3"^> & echo ^<security^> & echo ^<requestedPrivileges^> & echo ^<requestedExecutionLevel level="asInvoker" uiAccess="false"/^> & echo ^</requestedPrivileges^> & echo ^</security^> & echo ^</trustInfo^> & echo ^</assembly^>) > $@.manifest
	$(MT) -manifest $@.manifest -validate_manifest
	$(MT) -outputresource:"$@;#1" -manifest $@.manifest

$(PRGOBJS): $(PRGHDRS)

$(OUTDIR):
	if not exist $(OUTDIR)\ mkdir $(OUTDIR)

$(TRDPARTY)\zlib\$(BOUTDIR)\zlib.lib:
	cd $(TRDPARTY)\zlib
	$(MK) -f $(TOOLKIT).mak BCFG=$(BCFG) LSTT=$(LSTT)
	cd $(MAKEDIR)

$(TRDPARTY)\tfs\$(BOUTDIR)\tfs.lib:
	cd $(TRDPARTY)\tfs
	$(MK) -f $(TOOLKIT).mak BCFG=$(BCFG) LSTT=$(LSTT)
	cd $(MAKEDIR)

clean:
	if exist $(OUTDIR)\ $(RMDIR) $(OUTDIR)
	cd $(TRDPARTY)\tfs
	$(MK) -f $(TOOLKIT).mak clean BCFG=$(BCFG) LSTT=$(LSTT)
	cd $(MAKEDIR)
	cd $(TRDPARTY)\zlib
	$(MK) -f $(TOOLKIT).mak clean BCFG=$(BCFG) LSTT=$(LSTT)
	cd $(MAKEDIR)

spotless: clean
	$(RM) *~
	$(RM) tags
	cd $(TRDPARTY)\tfs
	$(MK) -f $(TOOLKIT).mak spotless BCFG=$(BCFG) LSTT=$(LSTT)
	cd $(MAKEDIR)
	cd $(TRDPARTY)\zlib
	$(MK) -f $(TOOLKIT).mak spotless BCFG=$(BCFG) LSTT=$(LSTT)
	cd $(MAKEDIR)
