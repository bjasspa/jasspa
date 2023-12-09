##############################################################################
#
#  			Copyright 2000-2004 Jon Green
#			      All Rights Reserved
#
#
#  System        : 
#  Module        : 
#  Object Name   : $RCSfile: Makefile,v $
#  Revision      : $Revision: 1.1 $
#  Date          : $Date: 2004-02-07 17:00:31 $
#  Author        : $Author: jon $
#  Created By    : Jon Green
#  Created       : Sat Oct 21 14:08:08 2000
#  Last Modified : <231205.1249>
#
#  Description	
#
#  Notes
#
#  History
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

TOPDIR	= ..
include $(TOPDIR)/etc/makeinc

# Local definitions.
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

# Dependencies
all::		$(EXES)

$(EXES):	$(OBJS) $(INSTLIBDIR)/libutils.$(A)
		$(CC) -o $@ $(LDFLAGS) $(OBJS) $(LOCALLIBS) $(CLIBS) $(SYSLIBS)

$(INSTBINDIR):
		$(MKDIR) $(INSTBINDIR)

clean::
		$(RM) $(EXES)
		$(RM) *.$O
		$(RM) *.A

spotless:	clean
		$(RM) *~
		$(RM) tags

install::	$(INSTBINDIR) $(EXES)
		$(INSTALL_EXE) $(EXES) $(INSTBINDIR)

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
