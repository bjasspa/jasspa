###########################-!- makefile -!-#################################
#
# LINUX Common make definitions
#
############################################################################

#
# Installation directories
#
BINDIR		= bin
LIBDIR		= lib
INCDIR		= include

INSTBINDIR	= $(ROOTDIR)/$(BINDIR)
INSTLIBDIR	= $(ROOTDIR)/$(LIBDIR)
INSTINCDIR	= $(ROOTDIR)/$(INCDIR)

INSTBINFLAGS	= -o jon -g users -m 0775
INSTFLAGS	= -o jon -g users -m 0644
INSTLIBFLAGS	= -o jon -g users -m 0644
INSTINCFLAGS	= -o jon -g users -m 0644
#
# 'C' Compiler rules
#
CPPFLAGS        =       -D_LINUX -D_UNIX $(LOCALDEFS) $(INCLUDES)
CDEBUGFLAGS	=	-g  -Wall
CPROFILEFLAGS	=	-p1
COPTIMISEFLAGS	=	-O3 -DNDEBUG=1 -Wall
CNONOPTIMISEFLAGS=	-O0 -DNDEBUG=1
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
INSTALL		=	install
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
