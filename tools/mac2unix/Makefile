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
#  Date          : $Date: 2003-07-27 17:14:48 $
#  Author        : $Author: jon $
#  Created By    : Jon Green
#  Created       : Sun Jul 27 17:56:28 2003
#  Last Modified : <030727.1801>
#
#  Description
#
#  Notes
#
#  History
#
#  $Log: not supported by cvs2svn $
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
RM = rm -f
CC = gcc $(CFLAGS)
LD = gcc
CFLAGS	= -O3 -I. -DTSDUMP_PLATFORM_SUNOS
LDFLAGS = -O3

TARGET	=	unix2mac	mac2unix

all:	$(TARGET)

unix2mac:	unix2mac.c
	$(CC) $(CCFLAGS) -o $@ $@.c
mac2unix:	mac2unix.c
	$(CC) $(CCFLAGS) -o $@ $@.c

clean:
	$(RM) ${TARGET}
	$(RM) *.o

spotless: clean
	$(RM) *~
	$(RM) core
	$(RM) tags
