##############################################################################
# This is part of the JASSPA MicroEmacs.
# Copyright (C) 2004 JASSPA (www.jasspa.com)
# See the file COPYING for copying and conditions.
#
# Created:       Thu Feb 5 21:36:24 2004
# Synopsis:      Installation Makefile
# Authors:       Jon Green
# Last Modified: <20040206.1100>
#
##############################################################################

SRCROOT=..
include $(SRCROOT)/etc/makeinc
#
# Source files
#
MSDEV6	=	memsdev6.dsw	memsdev6.dsp

#
# Released files
#
RELMSDEV6=	$(MSDEV6:%=$(MERELDIR)/msdev6/%)
RELEASES=	$(RELMSDEV6)
#
# Local rules.
#
# Build the local platform
all:	
	echo "Nothing to do"
# Install files
install:
	echo "Nothing to do"
# Build Release
release:	$(RELEASES)
# Release target files
$(RELMSDEV6): $(MSDEV6)
	$(MKDIR) $(@D)
	$(INSTALL_FIL) $(@F) $(@D)		
# Clean area.
clean:
	echo "Nothing to do"
# Remove all working files.
spotless: clean
	$(RM) *~
