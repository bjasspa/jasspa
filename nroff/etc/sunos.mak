##############################################################################
#
#  			Copyright 2004 Jon Green.
#			      All Rights Reserved
#
#
#  System        : 
#  Module        : 
#  Object Name   : $RCSfile: sunos.mak,v $
#  Revision      : $Revision: 1.1 $
#  Date          : $Date: 2004-01-06 00:52:42 $
#  Author        : $Author: jon $
#  Created By    : Jon Green
#  Created       : Sat Jan 3 19:39:07 2004
#  Last Modified : <040103.1940>
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
#  Copyright (c) 2004 Jon Green.
# 
#  All Rights Reserved.
# 
#  This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
#  reproduced,  translated,  or  reduced to any  electronic  medium or machine
#  readable form without prior written consent from Jon Green.
#
##############################################################################

#
# Installation directories
#
BINDIR		= bin
LIBDIR		= lib
INCDIR		= include

INSTBINDIR	= $(ROOTDIR)/$(BINDIR)
INSTLIBDIR	= $(ROOTDIR)/$(LIBDIR)/linux
INSTINCDIR	= $(ROOTDIR)/$(INCDIR)

INSTBINFLAGS	= -o jon -g users -m 0775
INSTFLAGS	= -o jon -g users -m 0644
INSTLIBFLAGS	= -o jon -g users -m 0644
INSTINCFLAGS	= -o jon -g users -m 0644
#
# 'C' Compiler rules
#
CPPFLAGS        =       -D_SUNOS -D_UNIX $(LOCALDEFS) $(INCLUDES)
CDEBUGFLAGS	=	-g  
CPROFILEFLAGS	=	-p1
COPTIMISEFLAGS	=	-O -DNDEBUG=1 -Wall
CNONOPTIMISEFLAGS=	-O -DNDEBUG=1
SYSLIBS         =	-lm

MALLOC_LIB =
# -lmalloc
XLIBS		=	-lXaw -lXt -lX11 -ltermcap $(MALLOC_LIB)

LD		=	cc
LD_CFLAGS	=	$(CFLAGS)

CPP		=	$(CC) -E
CC		=	cc
CFLAGS = 	$(CPPFLAGS) $(CDEBUGFLAGS)
I		=	-I
#
# YACC Flags
#
YACC		=	yacc
YFLAGS		=	-d
#
# AR flags
#
AR		=	ar
ARFLAGS		=	-rc
#
# Tools
#
RM		=	rm -f
RMDIR		=	rmdir
INSTALL		=	ginstall
MKDIR		=	mkdir -p
CD		=	cd
#
# RCS
#
% :: RCS/%,v
	$(CO) $(COFLAGS) $< $(@F)
#
# Shell defintions
#
SHELL = /bin/sh
#
# File extensions
#
O	=	o
A	=	a

all::
install::
clean::
	$(RM) *.$A
	$(RM) *.$O
	$(RM) *.pch
	$(RM) *.out
	$(RM) *.err
	$(RM) *.pdb
	$(RM) *.exe
	$(RM) *.ilk

spotless:: clean
	$(RM) *~
	$(RM) tags


