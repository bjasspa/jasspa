#____________________________________________________________________________
#
# Last Edited <240398.1422>
#
# Utilties makefile.
#____________________________________________________________________________
#

TARGET	= dos
LOCALDIR= ..
ROOTDIR = ../..
include $(ROOTDIR)/etc/$(TARGET)/makeinc.mk
VPATH	= $(LOCALDIR)

#____________________________________________________________________________
#
# Local definitions.
#____________________________________________________________________________
#

OBJ	=	getopt.$O getfiles.$O utils.$O error.$O dir.$O
SRC	=	getfiles.c utils.c error.c dir.c
HEADERS	=	$(LOCALDIR)/utils.h $(LOCALDIR)/getopt.h

#____________________________________________________________________________
#
# Dependencies
#____________________________________________________________________________
#

all::	libutils.a

libutils.a:	$(OBJ)
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $(OBJ)

error.$O::	$(LOCALDIR)/utils.h
utils.$O::	$(LOCALDIR)/utils.h
getfiles.$O::	$(LOCALDIR)/utils.h
getopt.$O::	$(LOCALDIR)/getopt.h
dir.$O::	$(LOCALDIR)/utils.h

install:: libutils.a $(HEADERS)
	$(INSTALL) $(INCDIR) $(HEADERS)
	$(MKDIR) $(ROOTDIR)/lib/dos
	$(INSTALL) $(LIBDIR) libutils.a

clean::
	$(RM) libutils.a $(OBJ)
