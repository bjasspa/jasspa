##############################################################################
# This is part of the JASSPA MicroEmacs.
# Copyright (C) 2004 JASSPA (www.jasspa.com)
# See the file COPYING for copying and conditions.
#
# Created:       Thu Feb 5 21:36:24 2004
# Synopsis:      Installation Makefile
# Authors:       Jon Green
# Last Modified: <20040206.1002>
#
##############################################################################

SRCROOT=..
include $(SRCROOT)/etc/makeinc
#
# Pixmap files
#
PIXMAPS	=	me.l.pm		me.m.pm		me.t.pm	\
		me.xpm		mini-me.xpm	ne.xpm
RELPIXMAPS=	$(PIXMAPS:%=$(MERELDIR)/pixmaps%)

all:
	echo "Nothing to do"
clean:
	echo "Nothing to do"
install:
	echo "Nothing to do"		
release: $(RELPIXMAPS)

spotless: clean
	$(RM) *~
#
# Local Rules
#
$(RELPIXMAPS):	$(PIXMAPS)
		$(MKDIR) $(@D)
		$(INSTALL) $(@F) $(@D)
