##-*- makefile -*-############################################################
#
#  			     Copyright 2004 JASSPA.
#			      All Rights Reserved
#
#
#  System        : 
#  Module        : 
#  Object Name   : $RCSfile: Makefile,v $
#  Revision      : $Revision: 1.1 $
#  Date          : $Date: 2005-03-10 01:00:54 $
#  Author        : $Author: jon $
#  Created By    : Jon Green
#  Created       : Mon Oct 25 22:00:44 2004
#  Last Modified : <041025.2207>
#
#  Description	
#
#  Notes
#
#  History
#	
##############################################################################
#
#  Copyright (c) 2004 JASSPA.
# 
#  All Rights Reserved.
# 
#  This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
#  reproduced,  translated,  or  reduced to any  electronic  medium or machine
#  readable form without prior written consent from JASSPA.
#
##############################################################################

TOPDIR=..
include $(TOPDIR)/etc/makeinc

#
# Define the standard tools
#
BIBTEX	= bibtex
PDFLATEX= pdflatex
THUMBPDF= thumbpdf

#
# Suffic rules for the conversion of
#
.SUFFIXES: .pdf .tex
.tex.pdf:
	- $(RM) $*.aux
	- $(RM) $*.log
	- $(RM) $*.lot
	- $(RM) $*.out
	- $(RM) $*.pdg
	- $(RM) $*.doc
	- $(PDFLATEX) $<
	- $(BIBTEX)   $*
	- $(PDFLATEX) $<
	- $(PDFLATEX) $<
	- $(THUMBPDF) $*
	- $(PDFLATEX) $<
	- $(RM) $*.aux
	- $(RM) $*.lot
	- $(RM) $*.out
	- $(RM) $*.pdg
	- $(RM) $*.doc
	- $(RM) thumb???.png
	- $(RM) $*.out
	- $(RM) $*.doc
	- $(RM) $*.tpt
	- $(RM) thumbpdf.log
	- $(RM) thumbpdf.pdf

# sid dependencies
ME_DEPS=	jasspame.tex logo.png
ME_BASE=	jasspame.pdf
#
# Build everything.
#
all:		$(ME_BASE)
$(ME_BASE):	$(ME_DEPS)

#
# Special build instructions.
#
# sitest_2sided_bwlinks.pdf: sitest_2sided_bwlinks.tex
# sitest_2sided_bwlinks.tex: sitest.tex $(SITEST_DEPS)
# 	cat sitest.tex | \
# 	sed -e "s/documentclass\[/documentclass\[twoside,openright,/" | \
# 	sed -e "s/{Colorlinks}{true}/{Colorlinks}{false}/" > $@
#
#sitest_2sided_collinks.pdf: sitest_2sided_collinks.tex
#sitest_2sided_collinks.tex: sitest.tex $(SITEST_DEPS)
#	cat sitest.tex | \
#	sed -e "s/documentclass\[/documentclass\[twoside,openright,/" > $@
#
#sitest_bwlinks.pdf: sitest_bwlinks.tex
#sitest_bwlinks.tex: sitest.tex $(SITEST_DEPS)
#	cat sitest.tex | \
#	sed -e "s/{Colorlinks}{true}/{Colorlinks}{false}/" > $@
#
#
# Delete everything expect the generated output
#
clean:
	$(RM) *.aux
	$(RM) *.log
	$(RM) *.lot
	$(RM) thumb???.png
	$(RM) *.out
	$(RM) *.doc
	$(RM) thumbpdf.log
	$(RM) thumbpdf.pdf
	$(RM) *.blg
	$(RM) *.bbl
	$(RM) *.toc
	$(RM) *.tpt
#
# Delete everything that is generated.
#
spotless: clean
	$(RM) *.pdf
	$(RM) *~
	$(RM) tags
	$(RM) $(ME_BASE)


