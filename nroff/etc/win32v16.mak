###########################-!- makefile -!-#################################
#
# WIN32 Common make definitions
#
############################################################################

#
# Installation directories
#
BINDIR		= bin
LIBDIR		= lib
INCDIR		= include

INSTBINDIR	= $(ROOTDIR)\$(BINDIR)
INSTLIBDIR	= $(ROOTDIR)\$(LIBDIR)\$(TARGET)
INSTINCDIR	= $(ROOTDIR)\$(INCDIR)

INSTBINFLAGS	=
INSTFLAGS	=
INSTLIBFLAGS	=
INSTINCFLAGS	=
#
# 'C' Compiler rules
#

CDEBUG        =	/W3 /D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE /MDd /Od /RTC1 /D_DEBUG
COPTIMISE     =	/W3 /D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE /MD /O2 /GL /GR- /GS- /DNDEBUG=1

CPPFLAGS        =       /D_WINDOWS /D_MBCS /D_WIN32 /DWIN32 $(LOCALDEFS) $(INCLUDES)
			
LDFLAGS		=	/SUBSYSTEM:CONSOLE /INCREMENTAL:NO /MACHINE:X86 /MANIFEST
CDEBUGFLAGS	=	/W3 /D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE /MDd /Od /RTC1 /D_DEBUG
COPTIMISEFLAGS	=	/W3 /D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE /MD /O2 /GL /GR- /GS- /DNDEBUG=1
SYSLIBS         =	user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib
MALLOC_LIB =
# -lmalloc
XLIBS		=	-lXaw -lXt -lX11 -ltermcap $(MALLOC_LIB)

LD		=	link /NOLOGO
LD_CFLAGS	=	$(CFLAGS)

CPP		=	$(CC) -E
CC		=	cl.exe /nologo
CFLAGS		=	$(CPPFLAGS)
RC		=	rc
I		=	/I
#
# YACC Flags
#
YACC		=	yacc
YFLAGS		=	-d
#
# AR flags
#
AR		=	lib
ARFLAGS		=	/SUBSYSTEM:console
#
# Tools
#
RM		=	- erase
RMDIR		=	- rmdir
INSTALL		=	install
MKDIR		=	mkdir
CD		=	cd
#
# Shell defintions
#
#SHELL = /bin/sh
#
# File extensions
#
O	=	o
A	=	lib
EXE	=	.exe
# Extended build rules.
.SUFFIXES: .od .o .c
#{..}.c{.}.o:
.c.o:
	$(CC) $(CDEBUG) $(CFLAGS) /c $*.c /Fo"$(*F).o"
#	$(CC) $(COPTIMISE) $(CFLAGS) -c $< -Fo$(*F).o
.c.od:	
	$(CC) $(CDEBUG) $(CFLAGS) /c $*.c /Fo"$(*F).od"
all::
install::
clean::
	$(RM) *.$A
	$(RM) *.$O
	$(RM) *.pch
	$(RM) *.out
	$(RM) *.err
	$(RM) *.pdb
	$(RM) *.exe
	$(RM) *.ilk

spotless:: clean
	$(RM) *.??~
	$(RM) tags

