# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# win32vc6.mak - Make file for Windows using Microsoft MSVC v6.0 development kit.
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
# Synopsis:    Make file for Windows using Microsoft MSVC v6.0 development kit.
# Notes:
#     Run .\build.bat to compile, build.bat -h for more information.
#
#     To build from the command line using nmake & makefile. 
#
#	Run "nmake -f win32vc6.mak"            for optimised build produces ./.win32vc6-release-mew/mew32.exe
#	Run "nmake -f win32vc6.mak BCFG=debug" for debug build produces     ./.win32vc6-debug-mew/mew32.exe
#	Run "nmake -f win32vc6.mak BTYP=c"     for console support          ./.win32vc6-release-mec/mec32.exe
#	Run "nmake -f win32vc6.mak BCOR=ne"    for ne build produces        ./.win32vc6-release-new/new32.exe
#
#	Run "nmake -f win32vc6.mak clean"      to clean source directory
#	Run "nmake -f win32vc6.mak spotless"   to clean source directory even more
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

BUILDID  = win32vc6
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
TRDPARTY = ..\3rdparty

CCDEFS   = /DWIN32 /D_WIN32 /D_WIN32_WINNT=0x500 /W3 /GX /I$(TRDPARTY)\tfs /I$(TRDPARTY)\zlib
CCFLAGSR = /MD /O2 /DNDEBUG=1
CCFLAGSD = /MDd /Od /Zi /GZ /D_DEBUG
LDDEFS   = /INCREMENTAL:NO
LDFLAGSR = /OPT:REF /OPT:ICF
LDFLAGSD = /DEBUG
LDLIBSB  = shell32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib
ARFLAGSR = /LTCG
ARFLAGSD =
RCFLAGS  = /d USE_UNCMP_PNG=1

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
BCOR     = me
BCOR_CDF = /D_SOCKET
PRGLIBS  = $(TRDPARTY)\tfs\$(BOUTDIR)\tfs.lib $(TRDPARTY)\zlib\$(BOUTDIR)\zlib.lib
LDLIBS   = wsock32.lib $(LDLIBSB)
!ENDIF

!IF "$(BTYP)" == "c"
BTYP_CDF = /D_ME_CONSOLE /D_CONSOLE
BTYP_LDF = /SUBSYSTEM:CONSOLE
!ELSE IF "$(BTYP)" == "cw"
BTYP_CDF = /D_ME_CONSOLE /D_CONSOLE /D_ME_WINDOW
BTYP_LDF = /SUBSYSTEM:CONSOLE
!ELSE
BTYP     = w
BTYP_CDF = /D_ME_WINDOW
BTYP_LDF = /SUBSYSTEM:WINDOWS
!ENDIF

OUTDIR   = $(BOUTDIR)-$(BCOR)$(BTYP)
PRGNAME  = $(BCOR)$(BTYP)32
PRGFILE  = $(PRGNAME)$(EXE)
PRGHDRS  = ebind.h edef.h eextrn.h efunc.h emain.h emode.h eprint.h esearch.h eskeys.h estruct.h eterm.h evar.h evers.h eopt.h \
	   ebind.def efunc.def eprint.def evar.def etermcap.def emode.def eskeys.def \
	   $(BUILDID).mak
PRGOBJS  = $(OUTDIR)\abbrev.o $(OUTDIR)\basic.o $(OUTDIR)\bind.o $(OUTDIR)\buffer.o $(OUTDIR)\crypt.o $(OUTDIR)\dirlist.o $(OUTDIR)\display.o \
	   $(OUTDIR)\eval.o $(OUTDIR)\exec.o $(OUTDIR)\file.o $(OUTDIR)\fileio.o $(OUTDIR)\frame.o $(OUTDIR)\hilight.o $(OUTDIR)\history.o $(OUTDIR)\input.o \
	   $(OUTDIR)\isearch.o $(OUTDIR)\key.o $(OUTDIR)\line.o $(OUTDIR)\macro.o $(OUTDIR)\main.o $(OUTDIR)\narrow.o $(OUTDIR)\next.o $(OUTDIR)\osd.o \
	   $(OUTDIR)\print.o $(OUTDIR)\random.o $(OUTDIR)\regex.o $(OUTDIR)\region.o $(OUTDIR)\registry.o $(OUTDIR)\search.o $(OUTDIR)\spawn.o \
	   $(OUTDIR)\spell.o $(OUTDIR)\tag.o $(OUTDIR)\termio.o $(OUTDIR)\time.o $(OUTDIR)\undo.o $(OUTDIR)\window.o $(OUTDIR)\word.o \
	   $(OUTDIR)\winterm.o $(OUTDIR)\winprint.o $(OUTDIR)\$(BCOR).res
#
# Rules
.SUFFIXES: .c .o .rc .res

.c{$(OUTDIR)}.o:
	$(CC) $(CCDEFS) $(BCOR_CDF) $(BTYP_CDF) $(CCFLAGS) /Fd"$(OUTDIR)\vc60.pdb" /c $< /Fo"$@"

.rc{$(OUTDIR)}.res:
	$(RC) $(RCFLAGS) $(BTYP_CDF) /fo"$@" $<

all: $(PRGLIBS) $(OUTDIR)\$(PRGFILE)

$(OUTDIR)\$(PRGFILE): $(OUTDIR) $(PRGOBJS) $(PRGLIBS)
	$(RM) $@
	$(LD) $(LDDEFS) $(BTYP_LDF) $(LDFLAGS) /PDB:"$(OUTDIR)\$(PRGNAME).pdb" /OUT:"$@" $(PRGOBJS) $(PRGLIBS) $(LDLIBS)

$(PRGOBJS): $(PRGHDRS)

$(OUTDIR):
	if not exist $(OUTDIR)\ mkdir $(OUTDIR)

$(TRDPARTY)\zlib\$(BOUTDIR)\zlib.lib:
	cd $(TRDPARTY)\zlib
	$(MK) -f $(BUILDID).mak BCFG=$(BCFG)
	cd $(MAKEDIR)

$(TRDPARTY)\tfs\$(BOUTDIR)\tfs.lib:
	cd $(TRDPARTY)\tfs
	$(MK) -f $(BUILDID).mak BCFG=$(BCFG)
	cd $(MAKEDIR)

clean:
	if exist $(OUTDIR)\ $(RMDIR) $(OUTDIR)
	cd $(TRDPARTY)\tfs
	$(MK) -f $(BUILDID).mak clean
	cd $(MAKEDIR)
	cd $(TRDPARTY)\zlib
	$(MK) -f $(BUILDID).mak clean
	cd $(MAKEDIR)

spotless: clean
	$(RM) *~
	$(RM) tags
	cd $(TRDPARTY)\tfs
	$(MK) -f $(BUILDID).mak spotless
	cd $(MAKEDIR)
	cd $(TRDPARTY)\zlib
	$(MK) -f $(BUILDID).mak spotless
	cd $(MAKEDIR)
