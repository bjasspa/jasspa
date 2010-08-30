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

BUILDID  = win32vs6
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
A        = .lib
EXE      = .exe
CC       = cl
CCDEFS   = -nologo -DWIN32 -D_WIN32 -D_WIN32_WINNT=0x500 
LD       = link
LDDEFS   = -nologo
AR       = lib
ARFLAGS  = -nologo
RC       = rc.exe
RCFLAGS  = 
RM       = rm.exe -f
RMDIR    = rm.exe -f -r

!IF "$(BCFG)" == "release"
OUTDIR   = $(OUTDIRR)
CCFLAGS  = $(CCDEFS) -MD -GX -YX -O2 -W3 -DNDEBUG=1 -Fp$(OUTDIRR)/vc50.pch
LDFLAGS  = $(LDDEFS)
LDLIBS   = 
!ELSE
OUTDIR   = $(OUTDIRD)
CCFLAGS  = $(CCDEFS) -MDd -Z7 -GZ -GX -Yd -YX -Od -W3 -D_DEBUG -Fp$(OUTDIRD)/vc50.pch
LDFLAGS  = $(LDDEFS)
LDLIBS   = 
!ENDIF

LIBNAME  = zlib 
LIBFILE  = $(LIBNAME)$(A)
LIBHDRS  = zutil.h zlib.h zconf.in.h zconf.h trees.h inftrees.h inflate.h \
	   inffixed.h inffast.h deflate.h crc32.h win32vs6.mak
LIBOBJS  = $(OUTDIR)/adler32.o $(OUTDIR)/compress.o $(OUTDIR)/crc32.o $(OUTDIR)/gzio.o $(OUTDIR)/uncompr.o \
	   $(OUTDIR)/deflate.o $(OUTDIR)/trees.o $(OUTDIR)/zutil.o $(OUTDIR)/inflate.o $(OUTDIR)/infback.o \
	   $(OUTDIR)/inftrees.o $(OUTDIR)/inffast.o

.SUFFIXES: .c .o

.c{$(OUTDIR)}.o:
	$(CC) $(CCDEFS) $(CCFLAGS) -c $< -Fo$@

all: $(OUTDIR)/$(LIBFILE)

$(OUTDIR)/$(LIBFILE): $(OUTDIR) $(LIBOBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) -out:$@ $(LIBOBJS)

$(LIBOBJS): $(LIBHDRS)

$(OUTDIR):
	if not exist $(OUTDIR)/ mkdir $(OUTDIR)

clean:
	$(RMDIR) $(OUTDIRR) $(OUTDIRD)

spotless:	clean
	$(RM) *~
	$(RM) tags
