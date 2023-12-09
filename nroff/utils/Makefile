#____________________________________________________________________________
#
# Last Edited <231205.1247>
#
# Utilties makefile.
#____________________________________________________________________________
#
TOPDIR	= ..
include $(TOPDIR)/etc/makeinc
# Local definitions.
INCLUDES=

SRC	=	getfiles.c utils.c error.c dir.c
HEADERS	=	utils.h
OBJ	=	$(SRC:.c=.$O)
# Dependencies
all::	libutils.$A

libutils.$A:	$(OBJ)
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $(OBJ)

error.$O:	$(HEADERS)
utils.$O:	$(HEADERS)
getfiles.$O:	$(HEADERS)
dir.$O:		$(HEADERS)

$(INSTINCDIR):
	$(MKDIR) $(INSTINCDIR)

$(INSTLIBDIR):
	$(MKDIR) $(INSTLIBDIR)

install:: $(INSTINCDIR) $(INSTLIBDIR) libutils.$A $(HEADERS)
	$(INSTALL_FIL) $(HEADERS) $(INSTINCDIR)
	$(INSTALL_FIL) libutils.$A $(INSTLIBDIR)

release: libutils.a $(HEADERS)
clean::
	$(RM) libutils.a $(OBJ)
spotless:: clean
	$(RM) *~
	$(RM) tags
