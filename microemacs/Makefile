##############################################################################
# This is part of the JASSPA MicroEmacs.
# Copyright (C) 2004 JASSPA (www.jasspa.com)
# See the file COPYING for copying and conditions.
#
# Created:       Thu Feb 5 21:36:24 2004
# Synopsis:      Installation Makefile
# Authors:       Jon Green
# Last Modified: <20040207.1748>
#
##############################################################################

TOPDIR=.
include $(TOPDIR)/etc/makeinc
# Installable text files
TXTS	=	COPYING	license.txt change.log
#
ZIPS	=	$(WWWRELDIR)/jasspa-mesrc-20$(MEYY)$(MEMM)$(MEDD).zip	\
		$(WWWRELDIR)/jasspa-memacros-20$(MEYY)$(MEMM)$(MEDD).zip\
		$(WWWRELDIR)/jasspa-metree-20$(MEYY)$(MEMM)$(MEDD).zip
TARS	=	$(WWWRELDIR)/jasspa-mesrc-20$(MEYY)$(MEMM)$(MEDD).tar.gz\
		$(WWWRELDIR)/jasspa-memacros-20$(MEYY)$(MEMM)$(MEDD).tar.gz\
		$(WWWRELDIR)/jasspa-metree-20$(MEYY)$(MEMM)$(MEDD).tar.gz
# Automatic rules
RELTXTS	=	$(TXTS:%=$(MERELDIR)/%)	$(TXTS:%=$(WWWRELDIR)/doc/%)

all:
	cd macros;	$(MAKE) $@
	cd spelling;	$(MAKE) $@
	cd company;	$(MAKE) $@
	cd src;		$(MAKE) $@
	cd doc;		$(MAKE) $@
	cd pixmaps;	$(MAKE) $@
	cd memsdev;	$(MAKE) $@
	cd msdev6;	$(MAKE) $@

install:
	cd macros;	$(MAKE) $@
	cd spelling;	$(MAKE) $@
	cd company;	$(MAKE) $@
	cd src;		$(MAKE) $@
	cd doc;		$(MAKE) $@
	cd pixmaps;	$(MAKE) $@
	cd memsdev;	$(MAKE) $@
	cd msdev6;	$(MAKE) $@

release::
	cd macros;	$(MAKE) $@
	cd spelling;	$(MAKE) $@
	cd company;	$(MAKE) $@
	cd src;		$(MAKE) $@
	cd doc;		$(MAKE) $@
	cd pixmaps;	$(MAKE) $@
	cd memsdev;	$(MAKE) $@
	cd msdev6;	$(MAKE) $@

clean:
	cd macros;	$(MAKE) $@
	cd spelling;	$(MAKE) $@
	cd company;	$(MAKE) $@
	cd src;		$(MAKE) $@
	cd doc;		$(MAKE) $@
	cd pixmaps;	$(MAKE) $@
	cd memsdev;	$(MAKE) $@
	cd msdev6;	$(MAKE) $@
	$(RMDIR) $(RELDIR)

spotless: clean
	cd macros;	$(MAKE) $@
	cd spelling;	$(MAKE) $@
	cd company;	$(MAKE) $@
	cd src;		$(MAKE) $@
	cd doc;		$(MAKE) $@
	cd pixmaps;	$(MAKE) $@
	cd memsdev;	$(MAKE) $@
	cd msdev6;	$(MAKE) $@
	cd etc;		$(MAKE)	$@
	$(RM) *~
#
# Local Rules
#
release:: $(RELTXTS) $(ZIPS) $(TARS)
# Text file release
$(RELTXTS):	$(TXTS)
		$(MKDIR) $(@D)
		$(INSTALL_FIL) $(@F) $(@D)
# Source archives
$(WWWRELDIR)/jasspa-mesrc-20$(MEYY)$(MEMM)$(MEDD).zip: $(MERELDIR)
	(cd $(RELDIR); $(ZIP) -r - ./me$(MEYY)$(MEMM)$(MEDD)) > $@
$(WWWRELDIR)/jasspa-mesrc-20$(MEYY)$(MEMM)$(MEDD).tar.gz: $(MERELDIR)
	(cd $(RELDIR); $(TAR) cf - ./me$(MEYY)$(MEMM)$(MEDD)) | $(GZIP) -c - > $@
# Macros archives
$(WWWRELDIR)/jasspa-memacros-20$(MEYY)$(MEMM)$(MEDD).zip: $(JASMACDIR)
	(cd $(JASMACDIR); $(ZIP) -r - ./*) > $@
$(WWWRELDIR)/jasspa-memacros-20$(MEYY)$(MEMM)$(MEDD).tar.gz: $(JASMACDIR)
	(cd $(JASMACDIR); $(TAR) cf - ./*) | $(GZIP) -c - > $@
# JASSPA tree archives
$(WWWRELDIR)/jasspa-metree-20$(MEYY)$(MEMM)$(MEDD).zip: $(JASSPADIR)
	(cd $(RELDIR); $(ZIP) -r - ./jasspa) > $@
$(WWWRELDIR)/jasspa-metree-20$(MEYY)$(MEMM)$(MEDD).tar.gz: $(JASSPADIR)
	(cd $(RELDIR); $(TAR) cf - ./jasspa) | $(GZIP) -c - > $@
