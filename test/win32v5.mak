# -!- makefile -!- ###########################################################
#
#			MicroEmacs Jasspa Distribution 1999.
#
#  System        : MicroEmacs
#  Description	 : Make file for Windows using Microsoft MSVC v5.0 development kit.
#  Created       : Sat Jan 24 00:01:40 1998
#
#  Last Modified : <991208.2037>
#
#  Notes
#       Build from the command line using nmake. 
#
#	Run "nmake -f win32v5.mak"      for optimised build produces ./me32.exe
#	Run "nmake -f win32v5.mak mec"  for console support          ./mec32.exe
#	Run "nmake -f win32v5.mak meu"  for url support              ./meu32.exe
#	Run "nmake -f win32v5.mak mecu" for console & url support    ./mecu32.exe
#	Run "nmake -f win32v5.mak med"  for debug build produces     ./med32.exe
#
#	Run "nmake -f win32v5.mak clean"      to clean source directory
#	Run "nmake -f win32v5.mak spotless"   to clean source directory even more
#
#  History
#
##############################################################################
#
# Microsoft MSCV 5.0 install directory
TOOLSDIR      = c:\Program Files\DevStudio\vc
#
# Installation Directory
INSTDIR	      = c:\emacs
INSTPROGFLAGS = 
#
# Local Definitions
RM            = erase
RC            =	rc
CC            =	cl
LD            =	link
INSTALL       =	copy
CDEBUG        =	-nologo -G5 -W3 -GX -Z7 -YX -Yd -Od
COPTIMISE     =	-nologo -G5 -YX -GX -O2 -DNDEBUG=1
CPROFILE      =	-nologo -G5 -YX -GX
CDEFS         = -D_JASSPA_ME -D_WIN32 -DWIN32 -D_WINDOWS -D_MBCS -I. "-I$(TOOLSDIR)\include"
LDFLAGS       = /NOLOGO /INCREMENTAL:no /MACHINE:IX86 /PDB:NONE "/LIBPATH:$(TOOLSDIR)\lib"
LIBS          = libc.lib user32.lib gdi32.lib winspool.lib \
		comdlg32.lib advapi32.lib shell32.lib
#
# Rules
.SUFFIXES: .c .o .od .oc .ou .ocu .rc .res

.c.o:	
	$(CC) $(COPTIMISE) $(CDEFS) -c $< -Fo$*.o

# Source files
SRC	= ftptest4.c

#
# Object files
OBJ	= $(SRC:.c=.o)	

#
# Targets
all: ftptest

install: me
	$(INSTALL) $(INSTPROGFLAGS) me $(INSTDIR)
	@echo "install done"

clean:
	$(RM) me32.exe
	$(RM) med32.exe
	$(RM) mec32.exe
	$(RM) meu32.exe
	$(RM) mecu32.exe
	$(RM) *.o
	$(RM) *.od
	$(RM) *.oc
	$(RM) *.ou 
	$(RM) *.ocu
	$(RM) me.aps
	$(RM) me.map
	$(RM) $(PLTRES)

spotless: clean
	$(RM) *.opt
	$(RM) *.pch
	$(RM) tags
	$(RM) *~

ftptest:	ftptest.exe
ftptest.exe: $(OBJ)
	$(LD) $(LDFLAGS) /SUBSYSTEM:console /out:$@ $(OBJ) ws2_32.lib $(LIBS)
