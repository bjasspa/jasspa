##############################################################################
# This is part of the JASSPA MicroEmacs.
# Copyright (C) 2004 JASSPA (www.jasspa.com)
# See the file COPYING for copying and conditions.
#
# Created:       Thu Feb 5 21:36:24 2004
# Synopsis:      Installation Makefile
# Authors:       Jon Green
# Last Modified: <20040205.2239>
#
##############################################################################

SRCROOT=..
include $(SRCROOT)/etc/makeinc
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
	$(INSTALL) $(@F) $(@D)
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
