##############################################################################
#
#  			Copyright 2000-2004 Jon Green.
#			      All Rights Reserved
#
#
#  System        : 
#  Module        : 
#  Object Name   : $RCSfile: sunos.mak,v $
#  Revision      : $Revision: 1.1 $
#  Date          : $Date: 2004-01-06 00:53:09 $
#  Author        : $Author: jon $
#  Created By    : Jon Green
#  Created       : Sat Oct 21 14:08:08 2000
#  Last Modified : <040104.0000>
#
#  Description	
#
#  Notes
#
#  History
#	
#
##############################################################################
#
#  Copyright (c) 2000-2004 Jon Green.
# 
#  All Rights Reserved.
# 
#  This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
#  reproduced,  translated,  or  reduced to any  electronic  medium or machine
#  readable form without prior written consent from Jon Green.
#
##############################################################################


#____________________________________________________________________________
#
# Last Edited <001021.1342>
#
# Bug makefile.
#____________________________________________________________________________
#

TARGET	=	linux
ROOTDIR	=	..
include		$(ROOTDIR)/etc/sunos.mak

#____________________________________________________________________________
#
# Local definitions.
#____________________________________________________________________________
#
LOCALLIBS=	$(INSTLIBDIR)/libutils.$A
HEADERS	=	idc.h		list.h		idc.def
UTILS_H	=	$(INSTINCDIR)/utils.h

INCLUDES=	$(I). $(I)$(INSTINCDIR)
EXES	=	idc$(EXE)

SRCS	=	idc.c		line.c		name.c		list.c \
		tek.c		rel.c		pat.c		bug.c \
		faq.c		new.c		inf.c		html.c \
		hml.c		rtf.c
OBJS	=	$(SRCS:.c=.o)

#____________________________________________________________________________
#
# Dependencies
#____________________________________________________________________________
#

all::	$(EXES)

$(EXES):	$(OBJS) $(INSTLIBDIR)/libutils.$(A)
		$(CC) -o $@ $(LDFLAGS) $(OBJS) $(LOCALLIBS) $(CLIBS) $(SYSLIBS)

clean::
		$(RM) $(EXES)

install::	$(EXES)
		$(MKDIR) $(INSTBINDIR)
		$(INSTALL) $(INSTBINFLAGS) $(EXES) $(INSTBINDIR)

idc.$O:		idc.h
line.$O:	idc.h idc.def
hash.$O:	hash.h
name.$O:	idc.h
list.$O:	list.h
tek.$O:		idc.h
rel.$O:		idc.h
pat.$O:		idc.h
bug.$O:		idc.h
faq.$O:		idc.h
new.$O:		idc.h
inf.$O:		idc.h
idc.h:		$(INSTINCDIR)/utils.h list.h hml.h
