##-*- makefile -*-############################################################
#
# Copyright (C) 2004-2025 JASSPA (www.jasspa.com).
#
# This is part of JASSPA's MicroEmacs, see the LICENSE file for licensing and
# copying information.
#
# Synopsis:      Installation Makefile
# Authors:       Jon Green
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
