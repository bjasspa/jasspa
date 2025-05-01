##############################################################################
#
#  Copyright (c) 2009 Maxexam Ltd.
# 
#  All Rights Reserved.
# 
#  This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
#  reproduced,  translated,  or  reduced to any  electronic  medium or machine
#  readable form without prior written consent from Maxexam Ltd.
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

TOOLKIT  = msvc
!IF "$(TOOLKIT_VER)" != ""
!ELSEIF "$(VISUALSTUDIOVERSION:.0=)" != ""
TOOLKIT_VER=$(VISUALSTUDIOVERSION:.0=)
!ELSEIF "$(VCINSTALLDIR:Visual Studio 10.0=)" != "$(VCINSTALLDIR)"
TOOLKIT_VER=10
!ELSE
!ERROR Failed to determine version of MSVC
!ENDIF

ARCHITEC = intel
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
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
INSTDIR  = ..\..\bin\$(BUILDID)
TRDPARTY = ..

CCDEFS   = /DWIN32 /D_WIN32 /D_WIN32_WINNT=0x0600 /D_$(BIT_SIZE)BIT /DZ7_ST /W3 /Zi /EHs-c- /D_HAS_EXCEPTIONS=0 /D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE /I.
CCFLAGSR = $(CCLSTT) /O2 /GL /GR- /GS- /DNDEBUG=1 /D_HAS_ITERATOR_DEBUGGING=0 /D_SECURE_SCL=0
CCFLAGSD = $(CCLSTT)d /Od /RTC1 /D_DEBUG
!IF "$(ARCHITEC)" == "arm"
LDDEFS   = /SUBSYSTEM:CONSOLE /INCREMENTAL:NO /MACHINE:ARM64 /MANIFEST $(LDLSTT)
!ELSEIF "$(BIT_SIZE)" == "64"
LDDEFS   = /SUBSYSTEM:CONSOLE /INCREMENTAL:NO /MACHINE:X64 /MANIFEST $(LDLSTT)
!ELSE
LDDEFS   = /SUBSYSTEM:CONSOLE /INCREMENTAL:NO /MACHINE:X86 /MANIFEST $(LDLSTT)
!ENDIF
LDFLAGSR = /OPT:REF /OPT:ICF=3 /LTCG
LDFLAGSD = /DEBUG
ARFLAGSR = /LTCG
ARFLAGSD =
RCFLAGS  = 

!IF "$(BCFG)" == "debug"
OUTDIR   = $(OUTDIRD)
CCFLAGS  = $(CCFLAGSD)
LDFLAGS  = $(LDFLAGSD)
ARFLAGS  = $(ARFLAGSD)
INSTPRG  = - echo No install - debug 
!ELSE
OUTDIR   = $(OUTDIRR)
CCFLAGS  = $(CCFLAGSR)
LDFLAGS  = $(LDFLAGSR)
ARFLAGS  = $(ARFLAGSR)
INSTPRG  = copy
!ENDIF

LIBNAME  = tfs
LIBFILE  = $(LIBNAME)$(A)
LIBHDRS  = tfs.h $(MAKEFILE).mak
LIBOBJS  = $(OUTDIR)\tfs.o $(OUTDIR)\Lzma2Dec.o $(OUTDIR)\LzmaDec.o

PRGNAME  = tfs
PRGFILE  = $(PRGNAME)$(EXE)
PRGHDRS  = tfs.h 7zTypes.h 7zWindows.h Compiler.h CpuArch.h LzFind.h LzHash.h LzmaDec.h LzmaEnc.h Precomp.h \
	   $(MAKEFILE).mak
PRGOBJS  = $(OUTDIR)\tfsutil.o $(OUTDIR)\LzFind.o $(OUTDIR)\Lzma2Enc.o $(OUTDIR)\LzmaEnc.o $(OUTDIR)\CpuArch.o
PRGLIBS  = 

TSTNAME  = tfstest
TSTFILE  = $(TSTNAME)$(EXE)
TSTHDRS  = tfs.h $(MAKEFILE).mak
TSTOBJS  = $(OUTDIR)\tfstest.o
TSTLIBS  = $(OUTDIR)\tfs$(A)

.SUFFIXES: .c .o

.c{$(OUTDIR)}.o:
	$(CC) $(CCDEFS) $(CCFLAGS) /Fd"$(OUTDIR)\vc$(TOOLKIT_VER)0.pdb" /c $< /Fo"$@"

all: $(PRGLIBS) $(OUTDIR)\$(LIBFILE) $(OUTDIR)\$(PRGFILE) $(OUTDIR)\$(TSTFILE)

$(OUTDIR)\$(LIBFILE): $(OUTDIR) $(LIBOBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) /OUT:"$@" $(LIBOBJS)

$(LIBOBJS): $(LIBHDRS)

$(OUTDIR)\$(PRGFILE): $(OUTDIR) $(INSTDIR) $(PRGOBJS) $(PRGLIBS)
	$(RM) $@
	$(LD) $(LDDEFS) $(LDFLAGS) /PDB:"$(OUTDIR)\$(PRGNAME).pdb" /MANIFESTFILE:"$@.intermediate.manifest" /OUT:"$@" $(PRGOBJS) $(PRGLIBS)
	$(MT) -outputresource:"$@;#2" -manifest $@.intermediate.manifest
	$(INSTPRG) $@ $(INSTDIR)

$(PRGOBJS): $(PRGHDRS)

$(OUTDIR)\$(TSTFILE): $(OUTDIR) $(INSTDIR) $(TSTOBJS) $(TSTLIBS)
	$(RM) $@
	$(LD) $(LDDEFS) $(LDFLAGS) /PDB:"$(OUTDIR)\$(TSTNAME).pdb" /MANIFESTFILE:"$@.intermediate.manifest" /OUT:"$@" $(TSTOBJS) $(TSTLIBS)
	$(MT) -outputresource:"$@;#2" -manifest $@.intermediate.manifest

$(TSTOBJS): $(TSTHDRS)

$(OUTDIR):
	if not exist $(OUTDIR)\ mkdir $(OUTDIR)

$(INSTDIR):
	if not exist $(INSTDIR)\ mkdir $(INSTDIR)

clean:
	if exist $(OUTDIRD)\ $(RMDIR) $(OUTDIRD)
	if exist $(OUTDIRR)\ $(RMDIR) $(OUTDIRR)

spotless: clean
	$(RM) *~
	$(RM) tags
