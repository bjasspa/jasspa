#____________________________-!- Makefile -!- ______________________________
#
# Last Edited <001021.1329>
#
# Utilties makefile.
#____________________________________________________________________________
#
TARGET	= win32
ROOTDIR = ..
include ..\etc\win32v6.mak
#____________________________________________________________________________
#
# Local definitions.
#____________________________________________________________________________
#
INCLUDES=	$(I).
UTILSRC	=	getfiles.c	utils.c		error.c		dir.c \
		getopt.c
#UTILOBJ	=	getfiles.o	utils.o		error.o		dir.o
UTILOBJ		= $(UTILSRC:.c=.o)

HEADERS	=	utils.h		getopt.h
#____________________________________________________________________________
#
# Dependencies
#____________________________________________________________________________
#
all::	libutils.$(A)

#$(UTILOBJ):	$(HEADERS)
libutils.$A:	$(UTILOBJ) $(HEADERS)
	$(RM) $@
	$(AR) $(ARFLAGS) /out:$@ $(UTILOBJ)


install:: libutils.$A $(HEADERS)
	- $(MKDIR) $(INSTINCDIR)
	$(INSTALL) $(HEADERS) $(INSTINCDIR)
	- $(MKDIR) $(ROOTDIR)\$(LIBDIR)
	- $(MKDIR) $(INSTLIBDIR)
	$(INSTALL) libutils.$A $(INSTLIBDIR)
	
