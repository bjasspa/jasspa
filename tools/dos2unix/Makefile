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
#  Date          : $Date: 2003-08-22 23:01:10 $
#  Author        : $Author: jon $
#  Created By    : Jon Green
#  Created       : Sun Jul 27 17:56:28 2003
#  Last Modified : <030822.2355>
#
#  Description
#
#  Notes
#
#  History
#
#  $Log: not supported by cvs2svn $
#  Revision 1.1  2003/07/27 17:14:48  jon
#  Added mac conversion tools
#
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
CFLAGS	= -O3 -I.
LDFLAGS = -O3

TARGET	=	unix2dos	dos2unix

all:	$(TARGET)

unix2dos:	unix2dos.c
	$(CC) $(CCFLAGS) -o $@ $@.c
dos2unix:	dos2unix.c
	$(CC) $(CCFLAGS) -o $@ $@.c

clean:
	$(RM) ${TARGET}
	$(RM) *.o

spotless: clean
	$(RM) *~
	$(RM) core
	$(RM) tags
