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

TOPDIR=..
include $(TOPDIR)/etc/makeinc
#
# Text files.
#
TXTOBJS	=	README.txt
#
# Released files
#
RELTXTS	= $(TXTOBJS:%=$(JASCOMDIR)/%)
#
# Build Release
#
release: $(RELTXTS)
# Release target files
$(JASCOMDIR):
	$(MKDIR) $@
$(RELTXTS): $(TXTOBJS) $(JASCOMDIR)
	$(INSTALL_FIL) $(@F) $(@D)
#
# Clean area.
#
clean:
	echo "Done."
#
# Remove all working files.
#
spotless:
	$(RM) tags
	$(RM) *~
