# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# winmsvc.mak - Make file for Windows using Microsoft MSVC development kit.
#
# Copyright (C) 2007-2024 JASSPA (www.jasspa.com)
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
# Synopsis:    Make file for Windows using Microsoft MSVC compiler.
# Notes:
#     Run .\build.bat to compile, build.bat -h for more information.
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

TOOLKIT  = msvc
!IF "$(TOOLKIT_VER)" != ""
!ELSEIF "$(VISUALSTUDIOVERSION:.0=)" != ""
TOOLKIT_VER=$(VISUALSTUDIOVERSION:.0=)
!ELSEIF "$(VCINSTALLDIR:Visual Studio 10.0=)" != "$(VCINSTALLDIR)"
TOOLKIT_VER=10
!ELSEIF "$(VCINSTALLDIR:Visual Studio 9.0=)" != "$(VCINSTALLDIR)"
TOOLKIT_VER=9
!ELSEIF "$(VCINSTALLDIR:Visual Studio 8.0=)" != "$(VCINSTALLDIR)"
TOOLKIT_VER=8
!ELSE
!ERROR Failed to determine version of MSVC
!ENDIF

!IF "$(ARCHITEC)" != ""
!ELSE
# TODO auto-detect arm ARCHITEC
ARCHITEC = intel
!ENDIF

!IF "$(BIT_SIZE)" != ""
!ELSEIF "$(PLATFORM)" == "x64"
BIT_SIZE = 64
!ELSE
BIT_SIZE = 32
!ENDIF

PLATFORM = windows
!IF "$(PLATFORM_VER)" == ""
PLATFORM_VER = -1
!IF [ver | findstr Version > .windows-ver.tmp] == 0
WIN_VER = \
!INCLUDE .windows-ver.tmp
!IF [del .windows-ver.tmp] == 0
!ENDIF
!IF "$(WIN_VER:Version 10.0=)" != "$(WIN_VER)"
PLATFORM_VER = 100
!ELSEIF "$(WIN_VER:Version 6.3=)" != "$(WIN_VER)"
PLATFORM_VER = 63
!ELSEIF "$(WIN_VER:Version 6.2=)" != "$(WIN_VER)"
PLATFORM_VER = 62
!ELSEIF "$(WIN_VER:Version 6.1=)" != "$(WIN_VER)"
PLATFORM_VER = 61
!ELSEIF "$(WIN_VER:Version 6.=)" != "$(WIN_VER)"
PLATFORM_VER = 60
!ELSEIF "$(WIN_VER:Version 5.=)" != "$(WIN_VER)"
PLATFORM_VER = 51
!ENDIF
!ENDIF
!ENDIF

include evers.mak

MAKEFILE = win$(TOOLKIT)
!IF "$(LSTT)" == "1"
BUILDID  = $(PLATFORM)$(PLATFORM_VER)-$(ARCHITEC)$(BIT_SIZE)-$(TOOLKIT)$(TOOLKIT_VER)s
CCLSTT   = /MT
LDLSTT   = /NODEFAULTLIB:msvcrt.lib
!ELSE
BUILDID  = $(PLATFORM)$(PLATFORM_VER)-$(ARCHITEC)$(BIT_SIZE)-$(TOOLKIT)$(TOOLKIT_VER)
CCLSTT   = /MD
LDLSTT   = 
!ENDIF
!IF "$(ASAN)" == "1"
BUILDID  = $(BUILDID)a
!ENDIF
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
INSTDIR  = ..\bin\$(BUILDID)
TRDPARTY = ..\3rdparty

CCDEFS   = /D_ARCHITEC=$(ARCHITEC) /D_TOOLKIT=$(TOOLKIT) /D_TOOLKIT_VER=$(TOOLKIT_VER) /D_PLATFORM_VER=$(PLATFORM_VER) /D_WIN32 /D_WIN32_WINNT=0x0602 /DWINVER=0x0602 /D_$(BIT_SIZE)BIT /D_WIN$(BIT_SIZE) /W3 /EHs-c- /D_HAS_EXCEPTIONS=0 /D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE /I$(TRDPARTY)\tfs /DmeVER_CN=$(meVER_CN) /DmeVER_YR=$(meVER_YR) /DmeVER_MN=$(meVER_MN) /DmeVER_DY=$(meVER_DY)
CCFLAGSR = $(CCLSTT) /O2 /GL /GR- /GS- /DNDEBUG=1 /D_HAS_ITERATOR_DEBUGGING=0 /D_SECURE_SCL=0
CCFLAGSD = $(CCLSTT)d /Od /RTC1 /Zi /D_DEBUG
!IF "$(ARCHITEC)" == "arm"
LDDEFS   = /INCREMENTAL:NO /MACHINE:ARM64 /MANIFEST $(LDLSTT)
MANPARCH = ARM64
!ELSEIF "$(BIT_SIZE)" == "64"
LDDEFS   = /INCREMENTAL:NO /MACHINE:X64 /MANIFEST $(LDLSTT)
MANPARCH = X86
!ELSE
LDDEFS   = /INCREMENTAL:NO /MACHINE:X86 /MANIFEST $(LDLSTT)
MANPARCH = X86
!ENDIF
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
INSTPRG  = - echo No install - debug 
!ELSE
BOUTDIR  = $(OUTDIRR)
CCFLAGS  = $(CCFLAGSR)
LDFLAGS  = $(LDFLAGSR)
ARFLAGS  = $(ARFLAGSR)
INSTPRG  = copy
!ENDIF
!IF "$(ASAN)" == "1"
CCFLAGS  = $(CCFLAGS) /fsanitize=address
!ENDIF

!IF "$(BCOR)" == "ne"

BCOR_CDF = /D_NANOEMACS
PRGLIBS  = 
LDLIBS   = $(LDLIBSB)

!ELSE

!IF "$(OPENSSLP)" == ""
!IF "$(ARCHITEC)" == "arm"
OSSL_DIR = arm64
OSSL_LIB = -arm64
!ELSEIF "$(BIT_SIZE)" == "64"
OSSL_DIR = x64
OSSL_LIB = -x64
!ELSE
OSSL_DIR = x86
!ENDIF
!IF EXISTS($(TRDPARTY)\openssl-3.5\$(OSSL_DIR)\include\openssl\ssl.h)
OPENSSLP = $(TRDPARTY)\openssl-3.5\$(OSSL_DIR)
OPENSSLV = -3$(OSSL_LIB)
!ELSEIF EXISTS($(TRDPARTY)\openssl-3.3\$(OSSL_DIR)\include\openssl\ssl.h)
OPENSSLP = $(TRDPARTY)\openssl-3.3\$(OSSL_DIR)
OPENSSLV = -3$(OSSL_LIB)
!ELSEIF EXISTS($(TRDPARTY)\openssl-3.2\$(OSSL_DIR)\include\openssl\ssl.h)
OPENSSLP = $(TRDPARTY)\openssl-3.2\$(OSSL_DIR)
OPENSSLV = -3$(OSSL_LIB)
!ELSEIF EXISTS($(TRDPARTY)\openssl-3.1\$(OSSL_DIR)\include\openssl\ssl.h)
OPENSSLP = $(TRDPARTY)\openssl-3.1\$(OSSL_DIR)
OPENSSLV = -3$(OSSL_LIB)
!ELSEIF EXISTS($(TRDPARTY)\openssl-3.0\$(OSSL_DIR)\include\openssl\ssl.h)
OPENSSLP = $(TRDPARTY)\openssl-3.0\$(OSSL_DIR)
OPENSSLV = -3$(OSSL_LIB)
!ELSEIF EXISTS($(TRDPARTY)\openssl-1.1\$(OSSL_DIR)\include\openssl\ssl.h)
OPENSSLP = $(TRDPARTY)\openssl-1.1\$(OSSL_DIR)
OPENSSLV = -1_1$(OSSL_LIB)
!ENDIF
!ENDIF
!IF "$(OPENSSLP)" == ""
!MESSAGE WARNING: No OpenSSL support found, https support will be disabled.
!MESSAGE
!ELSE
OPENSSLDEFS = /DMEOPT_OPENSSL=1 /I$(OPENSSLP)\include /D_OPENSSLLNM=libssl$(OPENSSLV).dll /D_OPENSSLCNM=libcrypto$(OPENSSLV).dll
OPENSSLLIBS = $(OPENSSLP)\lib\libssl.lib $(OPENSSLP)\lib\libcrypto.lib crypt32.lib
!ENDIF
BCOR     = me
BCOR_CDF = /D_SOCKET $(OPENSSLDEFS)
PRGLIBS  = $(TRDPARTY)\tfs\$(BOUTDIR)\tfs.lib
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
PRGNAME  = $(BCOR)$(BTYP)
PRGFILE  = $(PRGNAME)$(EXE)
PRGHDRS  = ebind.h edef.h eextrn.h efunc.h emain.h emode.h eprint.h esearch.h eskeys.h estruct.h eterm.h evar.h evers.h eopt.h \
	   ebind.def efunc.def eprint.def evar.def etermcap.def emode.def eskeys.def \
	   $(MAKEFILE).mak evers.mak
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
	$(CC) $(CCDEFS) $(BCOR_CDF) $(BTYP_CDF) $(CCFLAGS) /Fd"$(OUTDIR)\vc$(TOOLKIT_VER)0.pdb" /c $< /Fo"$@"

.rc{$(OUTDIR)}.res:
	$(RC) $(RCFLAGS) $(BTYP_CDF) /fo"$@" $<

all: $(PRGLIBS) $(OUTDIR)\$(PRGFILE)

$(OUTDIR)\$(PRGFILE): $(OUTDIR) $(INSTDIR) $(PRGOBJS) $(PRGLIBS)
	$(RM) $@
	$(LD) $(LDDEFS) $(BTYP_LDF) $(LDFLAGS) /PDB:"$(OUTDIR)\$(PRGNAME).pdb" /OUT:"$@" $(PRGOBJS) $(PRGLIBS) $(LDLIBS)
	(echo ^<?xml version="1.0" encoding="UTF-8" standalone="yes"?^> & echo ^<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0"^> & echo ^<assemblyIdentity version="$(meVER_YR).$(meVER_MN).$(meVER_DY).0" processorArchitecture="$(MANPARCH)" name="$(PRGNAME).exe" type="win32"/^> & echo ^<trustInfo xmlns="urn:schemas-microsoft-com:asm.v3"^> & echo ^<security^> & echo ^<requestedPrivileges^> & echo ^<requestedExecutionLevel level="asInvoker" uiAccess="false"/^> & echo ^</requestedPrivileges^> & echo ^</security^> & echo ^</trustInfo^> & echo ^</assembly^>) > $@.manifest
	$(MT) -manifest $@.manifest -validate_manifest
	$(MT) -outputresource:"$@;#1" -manifest $@.manifest
	$(INSTPRG) $@ $(INSTDIR)

$(PRGOBJS): $(PRGHDRS)

$(OUTDIR):
	if not exist $(OUTDIR)\ mkdir $(OUTDIR)

$(INSTDIR):
	if not exist $(INSTDIR)\ mkdir $(INSTDIR)

$(TRDPARTY)\tfs\$(BOUTDIR)\tfs.lib:
	cd $(TRDPARTY)\tfs
	$(MK) -f $(MAKEFILE).mak ASAN=$(ASAN) BCFG=$(BCFG) LSTT=$(LSTT) BPRF=$(BPRF) ARCHITEC=$(ARCHITEC) BIT_SIZE=$(BIT_SIZE)
	cd $(MAKEDIR)

clean:
	if exist $(OUTDIR)\ $(RMDIR) $(OUTDIR)
	cd $(TRDPARTY)\tfs
	$(MK) -f $(MAKEFILE).mak clean ASAN=$(ASAN) BCFG=$(BCFG) LSTT=$(LSTT) BPRF=$(BPRF) ARCHITEC=$(ARCHITEC) BIT_SIZE=$(BIT_SIZE)
	cd $(MAKEDIR)

spotless: clean
	$(RM) *~
	$(RM) tags
	cd $(TRDPARTY)\tfs
	$(MK) -f $(MAKEFILE).mak spotless ASAN=$(ASAN) BCFG=$(BCFG) LSTT=$(LSTT) BPRF=$(BPRF) ARCHITEC=$(ARCHITEC) BIT_SIZE=$(BIT_SIZE)
	cd $(MAKEDIR)
