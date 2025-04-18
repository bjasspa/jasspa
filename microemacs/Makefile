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

TOPDIR=.
include $(TOPDIR)/etc/makeinc
# Installable text files
TXTS	=	LICENSE change.log
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
	cd contribs;	$(MAKE) $@
	cd macros;	$(MAKE) $@
	cd spelling;	$(MAKE) $@
	cd company;	$(MAKE) $@
	cd src;		$(MAKE) $@
	cd doc;		$(MAKE) $@
	cd pixmaps;	$(MAKE) $@
	cd memsdev;	$(MAKE) $@
	cd msdev6;	$(MAKE) $@

install:
	cd contribs;	$(MAKE) $@
	cd macros;	$(MAKE) $@
	cd spelling;	$(MAKE) $@
	cd company;	$(MAKE) $@
	cd src;		$(MAKE) $@
	cd doc;		$(MAKE) $@
	cd pixmaps;	$(MAKE) $@
	cd memsdev;	$(MAKE) $@
	cd msdev6;	$(MAKE) $@

release::
	cd contribs;	$(MAKE) $@
	cd macros;	$(MAKE) $@
	cd spelling;	$(MAKE) $@
	cd company;	$(MAKE) $@
	cd src;		$(MAKE) $@
	cd doc;		$(MAKE) $@
	cd pixmaps;	$(MAKE) $@
	cd memsdev;	$(MAKE) $@
	cd msdev6;	$(MAKE) $@

clean:
	cd contribs;	$(MAKE) $@
	cd macros;	$(MAKE) $@
	cd spelling;	$(MAKE) $@
	cd company;	$(MAKE) $@
	cd src;		$(MAKE) $@
	cd doc;		$(MAKE) $@
	cd pixmaps;	$(MAKE) $@
	cd memsdev;	$(MAKE) $@
	cd msdev6;	$(MAKE) $@
	cd packages;	$(MAKE)	$@
	$(RMDIR) $(RELDIR)

spotless: clean
	cd contribs;	$(MAKE) $@
	cd macros;	$(MAKE) $@
	cd spelling;	$(MAKE) $@
	cd company;	$(MAKE) $@
	cd src;		$(MAKE) $@
	cd doc;		$(MAKE) $@
	cd pixmaps;	$(MAKE) $@
	cd memsdev;	$(MAKE) $@
	cd msdev6;	$(MAKE) $@
	cd etc;		$(MAKE)	$@
	cd packages;	$(MAKE)	$@
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
