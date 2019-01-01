# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# win32vc8.mak - Make file for Windows using Microsoft MSVC v8.0 development kit.
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
# Synopsis:    Make file for Windows using Microsoft MSVC v8.0 development kit.
# Notes:
#     Run .\build.bat to compile, build.bat -h for more information.
#
#     To build from the command line using nmake & makefile. 
#
#	Run "nmake -f win32vc8.mak"            for optimised build produces ./.win32vc8-release/meicons.exe
#	Run "nmake -f win32vc8.mak BCFG=debug" for debug build produces     ./.win32vc8-debug/meicons.exe
#
#	Run "nmake -f win32vc8.mak clean"      to clean source directory
#	Run "nmake -f win32vc8.mak spotless"   to clean source directory even more
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
RC       = rc.exe /nologo
MT       = mt.exe -nologo
MK       = nmake.exe /NOLOGO
LD       = link /NOLOGO
AR       = lib /NOLOGO
RM       = del /F /Q
RMDIR    = rd /S /Q

BUILDID  = win32vc8
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug

CCDEFS   = /DWIN32 /D_ME_WINDOW /D_WIN32 /D_WIN32_WINNT=0x500 /W3 /EHsc /Zi /D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE
CCFLAGSR = /MD /O2 /GL /DNDEBUG=1
CCFLAGSD = /MDd /Od /RTC1 /D_DEBUG
LDDEFS   = /SUBSYSTEM:WINDOWS /INCREMENTAL:NO /MACHINE:X86 /MANIFEST
LDFLAGSR = /OPT:REF /OPT:ICF /LTCG
LDFLAGSD = /DEBUG
LDLIBS   = wininet.lib shell32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib
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

PRGNAME  = meicons
PRGFILE  = $(PRGNAME)$(EXE)
PRGHDRS  = $(PRGNAME).h $(BUILDID).mak
PRGOBJS  = $(OUTDIR)/$(PRGNAME).o $(OUTDIR)/$(PRGNAME).res
ICONS	 =	me32.ico	c.ico		cpp.ico		h.ico \
		def.ico		emc.ico		empty.ico	e_grey.ico \
		e_magent.ico	e_blue.ico	e_black.ico	e_cyan.ico \
		e_green.ico	e_red.ico	e_yellow.ico	ehf.ico \
		make.ico	doc.ico		txt.ico		nroff0.ico \
		nroff1.ico	nroff2.ico	nroff3.ico	nroff4.ico \
		nroff5.ico	nroff6.ico	nroff7.ico	nroff8.ico \
		nroff9.ico	nroffso.ico	nrofftni.ico	nroffnrs.ico \
		man.ico		erblue.ico	erblue2.ico	erbrown.ico \
		ercyan.ico	ergreen.ico	ergreen2.ico	eraqua.ico \
		erblack.ico	ergrey.ico	ermagent.ico	erred.ico \
		eryellow.ico	abr.ico		dic.ico		hash.ico \
		twiddle.ico	y.ico		l.ico		p.ico \
		etf.ico		eaf.ico		edf.ico		esf.ico	\
		emf.ico		awk.ico		i.ico		rc.ico \
		rul.ico		log.ico		err.ico		lbn.ico \
		lib.ico		htm.ico		htp.ico		sm.ico \
		pso.ico		asm.ico		erf.ico		jst.ico \
		bat.ico		cfg.ico		asp.ico		vb.ico
#
# Rules
.SUFFIXES: .c .o .rc .res

.c{$(OUTDIR)}.o:
	$(CC) $(CCDEFS) $(CCFLAGS) /Fd"$(OUTDIR)\vc80.pdb" /c $< /Fo"$@"

.rc{$(OUTDIR)}.res:
	$(RC) $(RCFLAGS) /fo"$@" $<

all: $(OUTDIR)\$(PRGFILE)

$(OUTDIR)\$(PRGFILE): $(OUTDIR) $(PRGOBJS)
	$(RM) $@
	$(LD) $(LDDEFS) $(LDFLAGS) /PDB:"$(OUTDIR)\$(PRGNAME).pdb" /MANIFESTFILE:"$@.intermediate.manifest" /OUT:"$@" $(PRGOBJS) $(LDLIBS)
	$(MT) -outputresource:"$@;#2" -manifest $@.intermediate.manifest

$(PRGOBJS): $(PRGHDRS) $(ICONS)

$(OUTDIR):
	if not exist $(OUTDIR)\ mkdir $(OUTDIR)

clean:
	if exist $(OUTDIR)\ $(RMDIR) $(OUTDIR)

spotless: clean
	$(RM) *~
	$(RM) tags
