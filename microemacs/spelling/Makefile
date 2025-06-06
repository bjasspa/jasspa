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
# Compressed files.
#
ZIPOBJS	=	ls_dede.zip	ls_elgr.zip	ls_engb.zip	\
		ls_enus.zip	ls_eses.zip	ls_fifi.zip	\
		ls_frfr.zip	ls_itit.zip	ls_plpl.zip	\
		ls_ptpt.zip	ls_ruru.zip
TAROBJS	=	$(ZIPOBJS:.zip=.tar.gz)
#
# Text files.
#
TXTOBJS	=	README.txt
#
# Released files
#
RELWWWSPLDIR=$(WWWRELDIR)/spelling
RELZIPS	= $(ZIPOBJS:%=$(RELWWWSPLDIR)/%)
RELTARS	= $(TAROBJS:%=$(RELWWWSPLDIR)/%)
RELTXTS	= $(TXTOBJS:%=$(JASSPLDIR)/%)
#
# Dictionary files.
#
LSDEDE	=	lsrdede.emf	lsmdede.edf	LICENSE_dede.txt	README_dede.txt
LSELGR	=	lsrelgr.emf	lsmelgr.edf	LICENSE_elgr.txt	README_elgr.txt
LSENGB	=	lsrengb.emf	lsmengb.edf	LICENSE_engb.txt	README_engb.txt
LSENUS	=	lsrenus.emf	lsmenus.edf	LICENSE_enus.txt	README_enus.txt
LSESES	=	lsreses.emf	lsmeses.edf	LICENSE_eses.txt	README_eses.txt
LSFIFI	=	lsrfifi.emf	lsmfifi.edf	LICENSE_fifi.txt	README_fifi.txt
LSFRFR	=	lsrfrfr.emf	lsmfrfr.edf	LICENSE_frfr.txt	README_frfr.txt
LSITIT	=	lsritit.emf	lsmitit.edf	LICENSE_itit.txt	README_itit.txt
LSPLPL	=	lsrplpl.emf	lsmplpl.edf	LICENSE_plpl.txt	README_plpl.txt
LSPTPT	=	lsrptpt.emf	lsmptpt.edf	LICENSE_ptpt.txt	README_ptpt.txt
LSRURU	=	lsrruru.emf	lsmruru.edf	README_ruru.txt

all:	$(TAROBJS) $(ZIPOBJS)

# German dictionary.
ls_dede.zip:	$(LSDEDE)
	$(ZIP) $@ $(LSDEDE)
ls_dede.tar.gz:	$(LSDEDE)
	$(TAR) cf - $(LSDEDE) | $(GZIP) -c > $@

# Greek
ls_elgr.zip:	$(LSELGR)
	$(ZIP) $@ $(LSELGR)
ls_elgr.tar.gz:	$(LSELGR)
	$(TAR) cf - $(LSELGR) | $(GZIP) -c > $@

# American
ls_enus.zip:	$(LSENUS)
	$(ZIP) $@ $(LSENUS)
ls_enus.tar.gz:	$(LSENUS)
	$(TAR) cf - $(LSENUS) | $(GZIP) -c > $@

# British
ls_engb.zip:	$(LSENGB)
	$(ZIP) $@ $(LSENGB)
ls_engb.tar.gz:	$(LSENGB)
	$(TAR) cf - $(LSENGB) | $(GZIP) -c > $@

# Spanish
ls_eses.zip:	$(LSESES)
	$(ZIP) $@ $(LSESES)
ls_eses.tar.gz:	$(LSESES)
	$(TAR) cf - $(LSESES) | $(GZIP) -c > $@

# French
ls_frfr.zip:	$(LSFRFR)
	$(ZIP) $@ $(LSFRFR)
ls_frfr.tar.gz:	$(LSFRFR)
	$(TAR) cf - $(LSFRFR) | $(GZIP) -c > $@

# Finnish
ls_fifi.zip:	$(LSFIFI)
	$(ZIP) $@ $(LSFIFI)
ls_fifi.tar.gz:	$(LSFIFI)
	$(TAR) cf - $(LSFIFI) | $(GZIP) -c > $@

# Italian
ls_itit.zip:	$(LSITIT)
	$(ZIP) $@ $(LSITIT)
ls_itit.tar.gz:	$(LSITIT)
	$(TAR) cf - $(LSITIT) | $(GZIP) -c > $@

# Polish
ls_plpl.zip:	$(LSPLPL)
	$(ZIP) $@ $(LSPLPL)
ls_plpl.tar.gz:	$(LSPLPL)
	$(TAR) cf - $(LSPLPL) | $(GZIP) -c > $@

# Portuguese
ls_ptpt.zip:	$(LSPTPT)
	$(ZIP) $@ $(LSPTPT)
ls_ptpt.tar.gz:	$(LSPTPT)
	$(TAR) cf - $(LSPTPT) | $(GZIP) -c > $@

# Russian - Ye & Yo versions
ls_ruru.zip:	$(LSRURU)
	$(ZIP) $@ $(LSRURU)
ls_ruru.tar.gz:	$(LSRURU)
	$(TAR) cf - $(LSRURU) | $(GZIP) -c > $@

#
# Build Release
#
release:	$(RELWWWSPLDIR)  $(JASSPLDIR)	\
		$(RELZIPS) $(RELTARS) $(RELTXTS)
# Release target files
$(RELWWWSPLDIR):
	$(MKDIR) $@
$(JASSPLDIR):
	$(MKDIR) $@
$(RELZIPS): $(ZIPOBJS)
	$(INSTALL_FIL) $(@F) $(@D)
$(RELTARS): $(TAROBJS)
	$(INSTALL_FIL) $(@F) $(@D)
$(RELTXTS): $(TXTOBJS)
	$(INSTALL_FIL) $(@F) $(@D)
#
# Clean area.
#
clean:
	$(RM) $(ZIPOBJS) $(TAROBJS)
#
# Remove all working files.
#
spotless:
	$(RM) tags
	$(RM) *~
