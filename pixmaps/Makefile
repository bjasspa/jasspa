##############################################################################
# This is part of the JASSPA MicroEmacs.
# Copyright (C) 2004 JASSPA (www.jasspa.com)
# See the file COPYING for copying and conditions.
#
# Created:       Thu Feb 5 21:36:24 2004
# Synopsis:      Installation Makefile
# Authors:       Jon Green
# Last Modified: <20060917.1117>
#
##############################################################################

TOPDIR=..
include $(TOPDIR)/etc/makeinc
#
# Pixmap files
#
PIXMAPS	=	me.l.pm		me.m.pm		me.t.pm	\
		me.xpm		mini-me.xpm	ne.xpm	\
		me_l.png	me_m.png	me_s.png \
		ne_l.png	ne_m.png	ne_s.png \
		me_gnome_48.png	me_128.png

RELPIXMAPS=	$(PIXMAPS:%=$(JASSPADIR)/pixmaps/%)

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
		$(INSTALL_FIL) $(@F) $(@D)
