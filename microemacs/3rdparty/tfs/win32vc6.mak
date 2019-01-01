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
MK       = nmake.exe /NOLOGO
LD       = link /NOLOGO
AR       = lib /NOLOGO
RM       = del /F /Q
RMDIR    = rd /S /Q

BUILDID  = win32vc6
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
TRDPARTY = ..

CCDEFS   = /DWIN32 /D_WIN32 /D_WIN32_WINNT=0x500 /W3 /GX /I$(TRDPARTY)\zlib
CCFLAGSR = /MD /O2 /DNDEBUG=1
CCFLAGSD = /MDd /Od /Zi /GZ /D_DEBUG
LDDEFS   = /SUBSYSTEM:CONSOLE /INCREMENTAL:NO
LDFLAGSR = /OPT:REF /OPT:ICF
LDFLAGSD = /DEBUG
ARFLAGSR =
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
	$(CC) $(CCDEFS) $(CCFLAGS) /Fd"$(OUTDIR)\vc60.pdb" /c $< /Fo"$@"

all: $(PRGLIBS) $(OUTDIR)\$(LIBFILE) $(OUTDIR)\$(PRGFILE)

$(OUTDIR)\$(LIBFILE): $(OUTDIR) $(LIBOBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) /OUT:"$@" $(LIBOBJS)

$(LIBOBJS): $(LIBHDRS)

$(OUTDIR)\$(PRGFILE): $(OUTDIR) $(PRGOBJS) $(PRGLIBS)
	$(RM) $@
	$(LD) $(LDDEFS) $(LDFLAGS) /PDB:"$(OUTDIR)\$(PRGNAME).pdb" /OUT:"$@" $(PRGOBJS) $(PRGLIBS)

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
