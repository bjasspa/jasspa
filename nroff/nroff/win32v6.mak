#############################################################################
#
# Utilities Makefile
#
#############################################################################
#
#  System        : Nroff
#  Module        : All
#  Object Name   : $RCSfile: win32v6.mak,v $
#  Revision      : $Revision: 1.2 $
#  Date	         : $Date: 2002-03-10 18:36:43 $
#  Author        : $Author: jon $
#  Last Modified : <020310.1432>
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
TARGET	= win32
LOCALDIR= ..
ROOTDIR = ..
include ..\etc\win32v6.mak
#____________________________________________________________________________
#
# Local definitions.
#____________________________________________________________________________
#
INCLUDES=	$(I). $(I)$(ROOTDIR)\utils
UTILSDIR=	$(ROOTDIR)\utils

#fname32$(EXE)
HEADERS	=
LOCALLIBS=	$(INSTLIBDIR)\libutils.$A libnroff.$A

INCLUDES=       $(I). $(I)$(INSTINCDIR)
CLIBS	=	libnroff.$A
# -lmalloc
# -DDEBUG

HEADERS	=	nroff.def	_nroff.h	nroff.h

UTILS_H		= $(INSTINCDIR)\utils.h

RTFEXES	=	nr2rtf$(EXE)
TEXEXES	=	nr2tex$(EXE)
HTMEXES	=	nr2html$(EXE)	htmlc$(EXE)
MANEXES	=	superman$(EXE)	droff$(EXE)
TOLEXES	=	nrsearch$(EXE)	nrinfo$(EXE)	nrcheck$(EXE)	ntags$(EXE) \
		nrorder$(EXE)	difftags$(EXE)	nrar$(EXE)	hts2html$(EXE)\
		hts$(EXE)	nr2ehf$(EXE)
LOCLEXES=
#	table$(EXE)
MODULES =	$(RTFEXES) $(HTMEXES) $(MANEXES) $(TOLEXES) \
		$(CANEXES) $(TEXEXES) $(LOCLEXES)

EXEOBJ	=	nr2rtf.$O	nr2html.$O	htmlc.$O	superman.$O \
		droff.$O	nrsearch.$O	nrinfo.$O	nrcheck.$O \
		ntags.$O	nrorder.$O	difftags.$O	nrar.$O \
		hts2html.$O	hts.$O		nr2ehf.$O
#____________________________________________________________________________
#
# Dependencies
#____________________________________________________________________________
#
all::		$(MODULES)

$(EXEOBJ):	$(HEADERS) $(UTILS_H) html.h rtf.h

$(MODULES):	$(INSTLIBDIR)\libutils.$A libnroff.$A $*.$O
		$(LD) $(LDFLAGS) /out:$@ $*.$O $(LOCALLIBS) $(SYSLIBS)
#____________________________________________________________________________
#
# Build the nroff library.
#____________________________________________________________________________
#
LIBOBJ	=	nroff.$O dtags.$O catman.$O nlibrary.$O

libnroff.$A:	$(LIBOBJ)
		$(RM) $@
		$(AR) $(ARFLAGS) /out:$@ $(LIBOBJ)

$(LIBOBJ):	$(HEADERS) $*.c

#____________________________________________________________________________
#
# Install the files.
#____________________________________________________________________________
#
install::	$(MODULES)
		- $(MKDIR) $(INSTBINDIR)
		$(INSTALL) $(INSTBINFLAGS) $(MODULES) $(INSTBINDIR)

