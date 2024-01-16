##############################################################################
# This is part of the JASSPA MicroEmacs.
# Copyright (C) 2005-2009 JASSPA (www.jasspa.com)
# See the file COPYING for copying and conditions.
#
# Created:       Thu Feb 5 21:36:24 2004
# Synopsis:      Installation Makefile
# Authors:       Jon Green & Steven Phillips
#
##############################################################################

TOPDIR=..
include $(TOPDIR)/etc/makeinc

include filelist.mak

#
# Destination directory
#
RELEMFS	= $(EMFS:%=$(JASMACDIR)/%)
RELETFS	= $(ETFS:%=$(JASMACDIR)/%)
RELEAFS	= $(EAFS:%=$(JASMACDIR)/%)
RELERFS	= $(ERFS:%=$(JASMACDIR)/%)
RELEDFS	= $(EDFS:%=$(JASMACDIR)/%)
#
# Build everything
#
all:
	echo "Nothing to do for $@"
#
# Build Release
#
release::	$(JASMACDIR) $(RELEMFS) $(RELETFS) $(RELEAFS) \
		$(RELERFS) $(RELEDFS)
# Release target files
$(RELEMFS): $(EMFS)
	$(INSTALL_FIL) $(@F) $(@D)
$(RELETFS): $(ETFS)
	$(INSTALL_FIL) $(@F) $(@D)
$(RELEAFS): $(EAFS)
	$(INSTALL_FIL) $(@F) $(@D)
$(RELERFS): $(ERFS)
	$(INSTALL_FIL) $(@F) $(@D)
$(RELEDFS): $(EDFS)
	$(INSTALL_FIL) $(@F) $(@D)
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
