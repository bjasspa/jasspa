##############################################################################
# This is part of the JASSPA MicroEmacs.
# Copyright (C) 2004 JASSPA (www.jasspa.com)
# See the file COPYING for copying and conditions.
#
# Created:       Thu Feb 5 21:36:24 2004
# Synopsis:      Installation Makefile
# Authors:       Jon Green
# Last Modified: <20040207.1750>
#
##############################################################################

TOPDIR=..
include $(TOPDIR)/etc/makeinc
#
# Source files
#
MEMSDEV	=	stdafx.h		stdafx.cpp		\
		response.h		response.cpp		\
		resource.h		readme.txt		\
		memsdev.txt		memsdev.rc		\
		memsdev.odl		memsdev.h		\
		memsdev.dsw		memsdev.dsp		\
		memsdev.def		memsdev.cpp		\
		memsdev.clw		memsdev.aps		\
		maindialog.h		maindialog.cpp		\
		dsaddin.h		dsaddin.cpp		\
		commands.h		commands.cpp

MEMSDEVRES =	res/tbarmedm.bmp	res/tbarlrge.bmp	\
		res/memsdev.rc2		res/bitmap1.bmp
#
# Released files
#
RELMEMSDEV	=	$(MEMSDEV:%=$(MERELDIR)/memsdev/%)
RELMEMSDEVRES	=	$(MEMSDEVRES:%=$(MERELDIR)/memsdev/%)

RELEASES	=	$(RELMEMSDEV)	$(RELMEMSDEVRES)
#
# Local rules.
#
# Build the local platform
#
all:
	echo "Nothing to do"
# Build Release
release:	$(RELEASES)
# Release target files
$(RELMEMSDEV): $(MEMSDEV)
	$(MKDIR) $(@D)
	$(INSTALL_FIL) $(@F) $(@D)
$(RELMEMSDEVRES): $(MEMSDEVRES)
	$(MKDIR) $(@D)
	$(INSTALL_FIL) res/$(@F) $(@D)
# Clean area.
clean:
	echo "Nothing to do"
# Install files
install:
	echo "Nothing to do"
# Remove all working files.
spotless: clean
	$(RM) tags
	$(RM) *~
