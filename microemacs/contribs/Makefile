##############################################################################
# This is part of the JASSPA MicroEmacs.
# Copyright (C) 2004 JASSPA (www.jasspa.com)
# See the file COPYING for copying and conditions.
#
# Created:       Thu Feb 5 21:36:24 2004
# Synopsis:      Installation Makefile
# Authors:       Jon Green
# Last Modified: <20241217.1553>
#
##############################################################################

TOPDIR=..
include $(TOPDIR)/etc/makeinc
#
# Basic macros to release
#
EMFS=	user.emf myipipe.emf bol_eol.emf \
	sunkeys.emf cygkeys.emf \
	hkgroovy.emf
#
# Windows registry files.
#
REGS=	ME_4_all.reg	mesetup.reg
#
# Documentation
#
DOCS=	readme.txt  \
	jasspa-microemacs.desktop

#
# Destination directory
#
RELEMFS	= $(EMFS:%=$(JASCONDIR)/%)
RELDOCS	= $(DOCS:%=$(JASCONDIR)/%)
RELREGS	= $(REGS:%=$(JASCONDIR)/%)

#
# Build everything
#
all:
	echo "Nothing to do for $@"
#
# Build Release
#
release::	$(JASCONDIR) $(RELEMFS)	$(RELDOCS) $(RELREGS)
# Release target files
$(RELEMFS): $(EMFS)
	$(INSTALL_FIL) $(@F) $(@D)
$(RELDOCS): $(EMFS)
	$(INSTALL_FIL) $(@F) $(@D)
$(RELREGS): $(EMFS)
	$(INSTALL_FIL) $(@F) $(@D)
# Release target directory.
$(JASCONDIR):
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

cheatsheet:
	 pandoc cheatsheet.md -o cheatsheet.html --css mini.css -s \
	 	--metadata title="Jasspa MicroEmacs Cheatsheet" --metadata author="Detlef Groth, University of Potsdam"  \
	 	--syntax-definition /home/dgroth/workspace/microemacs/docs/emf.xml
