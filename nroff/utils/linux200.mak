#____________________________________________________________________________
#
# Last Edited <001021.1434>
#
# Utilties makefile.
#____________________________________________________________________________
#
TARGET	= linux
LOCALDIR= ..
ROOTDIR = ..
include $(ROOTDIR)/etc/linux200.mak
#____________________________________________________________________________
#
# Local definitions.
#____________________________________________________________________________
#
VPATH	= $(LOCALDIR)
INCLUDES=

SRC	=	getfiles.c utils.c error.c dir.c
HEADERS	=	utils.h
OBJ	=	$(SRC:.c=.$O)
#____________________________________________________________________________
#
# Dependencies
#____________________________________________________________________________
#
all::	libutils.$A

libutils.$A:	$(OBJ)
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $(OBJ)

error.$O:	$(HEADERS)
utils.$O:	$(HEADERS)
getfiles.$O:	$(HEADERS)
dir.$O:		$(HEADERS)

install:: libutils.$A $(HEADERS)
	$(MKDIR) $(INSTINCDIR)
	$(MKDIR) $(INSTLIBDIR)
	$(INSTALL) $(INSTINCFLAGS) $(HEADERS) $(INSTINCDIR)
	$(INSTALL) $(INSTLIBFLAGS) libutils.$A $(INSTLIBDIR)

release: libutils.a $(HEADERS)
clean::
	$(RM) libutils.a $(OBJ)
spotless:: clean
	$(RM) $(LOCALDIR)/*~
	$(RM) *~
	$(RM) tags
