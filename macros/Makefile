##############################################################################
# This is part of the JASSPA MicroEmacs.
# Copyright (C) 2004 JASSPA (www.jasspa.com)
# See the file COPYING for copying and conditions.
#
# Created:       Thu Feb 5 21:36:24 2004
# Synopsis:      Installation Makefile
# Authors:       Jon Green
# Last Modified: <20040205.2156>
#
##############################################################################

SRCROOT=..
include $(SRCROOT)/etc/makeinc
#
# Basic macros to release
#
EMFS:sh	=ls *.emf
ETFS:sh	=ls *.etf
EAFS:sh	=ls *.eaf
ERFS:sh	=ls *.erf
EDFS:sh	=ls *.edf
OTHS	=null
#
# Destination directory
#
RELEMFS	= $(EMFS:%=$(JASMACDIR)/%)
RELETFS	= $(ETFS:%=$(JASMACDIR)/%)
RELEAFS	= $(EAFS:%=$(JASMACDIR)/%)
RELERFS	= $(ERFS:%=$(JASMACDIR)/%)
RELEDFS	= $(EDFS:%=$(JASMACDIR)/%)
RELOTHS = $(OTHS:%=$(JASMACDIR)/%)

#
# Build everything
#
all:
	echo "Nothing to do for $@"
#
# Build Release
#
release:: $(RELEMFS) $(RELETFS) $(RELEAFS) $(RELERFS) $(RELEDFS) $(RELOTHS)
# Release target files
$(RELEMFS): $(EMFS) $(JASMACDIR)
	$(INSTALL) $(@F) $(@D)
$(RELETFS): $(ETFS) $(JASMACDIR)
	$(INSTALL) $(@F) $(@D)
$(RELEAFS): $(EAFS) $(JASMACDIR)
	$(INSTALL) $(@F) $(@D)
$(RELERFS): $(ERFS) $(JASMACDIR)
	$(INSTALL) $(@F) $(@D)
$(RELEDFS): $(EDFS) $(JASMACDIR)
	$(INSTALL) $(@F) $(@D)
$(RELOTHS): $(OTHERS) $(JASMACDIR)
	$(INSTALL) $(@F) $(@D)
# Release target directory.
$(JASMACDIR):
	$(MKDIR) $@
#
# Clean area.
#
clean:
#
# Remove all working files.
#
spotless:
	$(RM) tags
	$(RM) *~
