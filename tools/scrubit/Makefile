##############################################################################
#
#			Copyright 2003 Jon Green.
#			      All Rights Reserved
#
#
#  System        :
#  Module        :
#  Object Name   : $RCSfile: Makefile,v $
#  Revision      : $Revision: 1.1 $
#  Date          : $Date: 2004-02-13 15:21:41 $
#  Author        : $Author: jon $
#  Created By    : Jon Green
#  Created       : Sun Jul 27 17:56:28 2003
#  Last Modified : <040213.1520>
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

TARGET		=	scrubit$(EXE)
INSTALL_TARGET	=	$(TARGET:%=$(INSTBINDIR)/%)

all:	$(TARGET)

scrubit:	scrubit.c
	$(CC) $(CCFLAGS) -o $@ $@.c

install:	$(INSTALL_TARGET)

$(INSTBINDIR)/scrubit$(EXE): scrubit$(EXE)
	$(MKDIR) ${@D}
	$(INSTALL_EXE) ${@F} ${@D}

clean:
	$(RM) ${TARGET}
	$(RM) *.o

spotless: clean
	$(RM) *~
	$(RM) core
	$(RM) tags
