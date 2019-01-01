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
TRDPARTY = ..

CCDEFS   = /DWIN32 /D_WIN32 /D_WIN32_WINNT=0x500 /W3 /EHsc /Zi /D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE /I$(TRDPARTY)\zlib
CCFLAGSR = /MD /O2 /GL /DNDEBUG=1
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

LIBNAME  = tfs
LIBFILE  = $(LIBNAME)$(A)
LIBHDRS  = tfs.h $(BUILDID).mak
LIBOBJS  = $(OUTDIR)\tfs.o

PRGNAME  = tfs
PRGFILE  = $(PRGNAME)$(EXE)
PRGHDRS  = tfs.h tfsutil.h $(BUILDID).mak
PRGOBJS  = $(OUTDIR)\tfsutil.o $(OUTDIR)\tfs.o $(OUTDIR)\uappend.o $(OUTDIR)\ubuild.o $(OUTDIR)\ucopy.o \
	   $(OUTDIR)\ucreate.o $(OUTDIR)\uinfo.o $(OUTDIR)\ulist.o $(OUTDIR)\ustrip.o $(OUTDIR)\utest.o \
	   $(OUTDIR)\uxdir.o $(OUTDIR)\uxfile.o
PRGLIBS  = $(TRDPARTY)\zlib\$(OUTDIR)\zlib$(A)

.SUFFIXES: .c .o

.c{$(OUTDIR)}.o:
	$(CC) $(CCDEFS) $(CCFLAGS) /Fd"$(OUTDIR)\vc80.pdb" /c $< /Fo"$@"

all: $(PRGLIBS) $(OUTDIR)\$(LIBFILE) $(OUTDIR)\$(PRGFILE)

$(OUTDIR)\$(LIBFILE): $(OUTDIR) $(LIBOBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) /OUT:"$@" $(LIBOBJS)

$(LIBOBJS): $(LIBHDRS)

$(OUTDIR)\$(PRGFILE): $(OUTDIR) $(PRGOBJS) $(PRGLIBS)
	$(RM) $@
	$(LD) $(LDDEFS) $(LDFLAGS) /PDB:"$(OUTDIR)\$(PRGNAME).pdb" /MANIFESTFILE:"$@.intermediate.manifest" /OUT:"$@" $(PRGOBJS) $(PRGLIBS)
	$(MT) -outputresource:"$@;#2" -manifest $@.intermediate.manifest

$(PRGOBJS): $(PRGHDRS)

$(TRDPARTY)\zlib\$(OUTDIR)\zlib$(A):
	cd $(TRDPARTY)\zlib
	$(MK) -f $(BUILDID).mak BCFG=$(BCFG)
	cd $(MAKEDIR)

$(OUTDIR):
	if not exist $(OUTDIR)\ mkdir $(OUTDIR)

clean:
	if exist $(OUTDIRD)\ $(RMDIR) $(OUTDIRD)
	if exist $(OUTDIRR)\ $(RMDIR) $(OUTDIRR)

spotless: clean
	$(RM) *~
	$(RM) tags
