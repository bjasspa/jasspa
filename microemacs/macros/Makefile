##-*- makefile -*-############################################################
#
# Copyright (C) 2005-2025 JASSPA (www.jasspa.com).
#
# This is part of JASSPA's MicroEmacs, see the LICENSE file for licensing and
# copying information.
#
# Synopsis:    Installation Makefile
# Authors:     Jon Green & Steven Phillips
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
