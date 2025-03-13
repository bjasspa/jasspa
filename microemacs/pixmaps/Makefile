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
