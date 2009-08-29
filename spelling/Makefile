##############################################################################
# This is part of the JASSPA MicroEmacs.
# Copyright (C) 2004-2009 JASSPA (www.jasspa.com)
# See the file COPYING for copying and conditions.
#
# Created:       Thu Feb 5 21:36:24 2004
# Synopsis:      Installation Makefile
# Authors:       Jon Green
# Last Modified: <20090829.1807>
#
##############################################################################

TOPDIR=..
include $(TOPDIR)/etc/makeinc
#
# Compressed files.
#
ZIPOBJS	=	ls_dede.zip	ls_engb.zip	ls_enus.zip	\
		ls_eses.zip	ls_frfr.zip	ls_itit.zip	\
		ls_plpl.zip	ls_ptpt.zip	ls_fifi.zip	\
		ls_ruye.zip	ls_ruyo.zip
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
LSDEDE	=	lsrdede.emf	lsdmdede.edf	lsdxdede.edf
LSENGB	=	lsrengb.emf	lsdmengb.edf	lsdxengb.edf
LSENUS	=	lsrenus.emf	lsdmenus.edf	lsdxenus.edf
LSESES	=	lsreses.emf	lsdmeses.edf	COPYING
LSFIFI	=	lsrfifi.emf	lsdmfifi.edf
LSFRFR	=	lsrfrfr.emf	lsdmfrfr.edf	COPYING
LSITIT	=	lsritit.emf	lsdmitit.edf	COPYING
LSPLPL	=	lsrplpl.emf	lsdmplpl.edf
LSPTPT	=	lsrptpt.emf	lsdmptpt.edf	COPYING
LSRUYE	=	lsrruye.emf	lsdmruye.edf
LSRUYO	=	lsrruyo.emf	lsdmruyo.edf

all:	$(TAROBJS) $(ZIPOBJS)

# German dictionary.
ls_dede.zip:	$(LSDEDE)
	$(ZIP) $@ $(LSDEDE)
ls_dede.tar.gz:	$(LSDEDE)
	$(TAR) cf - $(LSDEDE) | $(GZIP) -c > $@

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
ls_ruye.zip:	$(LSRUYE)
	$(ZIP) $@ $(LSRUYE)
ls_ruye.tar.gz:	$(LSRUYE)
	$(TAR) cf - $(LSRUYE) | $(GZIP) -c > $@
ls_ruyo.zip:	$(LSRUYO)
	$(ZIP) $@ $(LSRUYO)
ls_ruyo.tar.gz:	$(LSRUYO)
	$(TAR) cf - $(LSRUYO) | $(GZIP) -c > $@

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
