#____________________________________________________________________________
#
# Make file for MicroEmacs using the Microsoft MSCV 1.5 development kit.
#
# Author      : Jon Green
# Created     :  020197.1002
# Last Edited : <040199.2240>
#
# History     : JG  020197 Original  Constructed from MS envoronment and
#				     UNIX makefile.
#
# Details     : THIS IS BUILT WITH MSVC 1.5 THE 16-BIT DOS COMPILER. 
#               THIS BUILDS THE WIN32s 16-BIT THUNKING CODE THAT WAITS
#               FOR A COMMAND TO EXECUTE.
#
#               Build from the command line using nmake.
#
#               nmake              - builds locally.
#		nmake clean        - cleans emacs directory
#		nmake spotless     - cleans some more !!
#		nmake install	   - builds and copies to c:\emacs.
#____________________________________________________________________________
#
#
#
# TOOLS Environment - change to location of your tools.
#
TOOLSDIR=       c:\msvc
INCLUDE=	$(TOOLSDIR)\include
LIBRARY=	$(TOOLSDIR)\lib
TOOLSBIN=	$(TOOLSDIR)\bin

#
# Standard Compilation tools and flags
#
CC	=	$(TOOLSBIN)\cl
RC	=	$(TOOLSBIN)\rc
RM	=	erase
LD	=	$(TOOLSBIN)\link
INSTALL	=	copy
MKDIR	=	mkdir

# Standard File Extensions
I	=	-I
O	=	.obj
EXE	=	.exe

# Extended build rules.
.SUFFIXES: .c .obj
.c.obj:
	$(CC) $(CFLAGS) -c $< -Fo$*.obj

#
# Standard include stuff
#
CINCLUDES	=	$(I). $(I)$(INCLUDE)
CDEFSD		=	$(CDEBUGFLAGS) -D_WIN32s -DWIN32 -D_WINDOWS -D_MBCS
CDEFS		=	$(DEBUGFLAGS) -D_WIN32s -DWIN32 -D_WINDOWS -D_MBCS
CFLAGS		=	$(CINCLUDES) $(CDEFS)  -c -GA -Os -W3 -Zpe /DWINVER=0x0300
LDFLAGS		=	/LI /CO /MAP /align:16

DLLLIBS		=	.\W32SUT16.LIB \
			$(LIBRARY)\libw.lib \
			$(LIBRARY)\oldnames.lib \
			$(LIBRARY)\slibcew.lib \
			$(LIBRARY)\slibce.lib \
			$(LIBRARY)\TOOLHELP.LIB

all: methnk16.dll

methnk16.dll: methnk16$(O)
	echo >>NUL @<<methnk16.crf
methnk16$(O)
methnk16.dll
methnk16.map
$(DLLLIBS)
methnk16.def
<<
	$(LD) $(LDFLAGS) @methnk16.crf
	- $(RM) methnk16.map
	- $(RM) methnk16.lib
	- $(RM) methnk16.crf

spotless:: clean
	- $(RM) *.??~
	- $(RM) methnk16.dll
clean::
	- $(RM) *.pch
	- $(RM) *.obj
	- $(RM) *.exe
	- $(RM) *.res
	- $(RM) methnk16.lib
	- $(RM) methnk16.map
