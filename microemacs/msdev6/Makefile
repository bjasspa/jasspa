##############################################################################
# This is part of the JASSPA MicroEmacs.
# Copyright (C) 2004 JASSPA (www.jasspa.com)
# See the file COPYING for copying and conditions.
#
# Created:       Thu Feb 5 21:36:24 2004
# Synopsis:      Installation Makefile
# Authors:       Jon Green
# Last Modified: <20040207.1751>
#
##############################################################################

TOPDIR=..
include $(TOPDIR)/etc/makeinc
#
# Source files
#
SRCFIL	=	memsdev6.dsp	memsdev6.dsw
#
# Released files
#
RELSRCFIL	=	$(SRCFIL:%=$(MERELDIR)/msdev6/%)
RELEASES	=	$(RELSRCFIL)
#
# Local rules.
#
# Build the local platform
#
all:	
	echo "Nothing to do"
#
# Build Release
#
release:	$(RELEASES)

# Release target files
$(RELSRCFIL): $(SRCFIL)
	$(MKDIR) $(@D)
	$(INSTALL_FIL) $(@F) $(@D)
#
# Clean area.
#
clean:
#
# Remove all working files.
#
spotless: clean
	$(RM) *~
