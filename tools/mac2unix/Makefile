##############################################################################
#
#			Copyright 2003 Jon Green.
#			      All Rights Reserved
#
#
#  System        :
#  Module        :
#  Object Name   : $RCSfile: Makefile,v $
#  Revision      : $Revision: 1.2 $
#  Date          : $Date: 2004-02-13 15:22:18 $
#  Author        : $Author: jon $
#  Created By    : Jon Green
#  Created       : Sun Jul 27 17:56:28 2003
#  Last Modified : <040213.1518>
#
#  Description
#
#  Notes
#
#  History
#
##############################################################################
#
#  Copyright (c) 2003 Jon Green.
#
#  All Rights Reserved.
#
#  This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
#  reproduced,  translated,  or  reduced to any  electronic  medium or machine
#  readable form without prior written consent from Jon Green.
#
##############################################################################

# Set up the defaults for the environment.
TOPDIR=..
include $(TOPDIR)/etc/makeinc
# Local definitions for compiler.
INCLUDES=$(I)./ $(I)$(INSTINCDIR)
LOCALDEFS= -DTSDUMP_PLATFORM_SUNOS

TARGET		=	unix2mac$(EXE)	mac2unix$(EXE)
INSTALL_TARGET	=	$(TARGET:%=$(INSTBINDIR)/%)

all:	$(TARGET)

unix2mac:	unix2mac.c
	$(CC) $(CCFLAGS) -o $@ $@.c
mac2unix:	mac2unix.c
	$(CC) $(CCFLAGS) -o $@ $@.c

install:	$(INSTALL_TARGET)

$(INSTBINDIR)/unix2mac$(EXE): unix2mac$(EXE)
	$(MKDIR) ${@D}
	$(INSTALL_EXE) ${@F} ${@D}
$(INSTBINDIR)/mac2unix$(EXE): mac2unix$(EXE)
	$(MKDIR) ${@D}
	$(INSTALL_EXE) ${@F} ${@D}

clean:
	$(RM) ${TARGET}
	$(RM) *.o

spotless: clean
	$(RM) *~
	$(RM) core
	$(RM) tags
