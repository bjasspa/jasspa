##############################################################################
#
#  			Copyright 2000-2004 Jon Green.
#			      All Rights Reserved
#
#
#  System        : 
#  Module        : 
#  Object Name   : $RCSfile: Makefile,v $
#  Revision      : $Revision: 1.1 $
#  Date          : $Date: 2004-02-07 16:59:52 $
#  Author        : $Author: jon $
#  Created By    : Jon Green
#  Created       : Sat Feb 7 16:08:59 2004
#  Last Modified : <040207.1616>
#
#  Description	
#
#  Notes
#
#  History
#	
##############################################################################
#
#  Copyright (c) 2000-2004 Jon Green.
# 
#  All Rights Reserved.
# 
#  This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
#  reproduced,  translated,  or  reduced to any  electronic  medium or machine
#  readable form without prior written consent from Jon Green.
#
##############################################################################
#
TOPDIR	= ..
include $(TOPDIR)/etc/makeinc
#
# Define the modules to build to make.
#
# Documentation components
#
SUPERMAN=	$(INSTBINDIR)/superman -b
NR2RTF	=	$(INSTBINDIR)/nr2rtf
NR2TEX	=	$(INSTBINDIR)/nr2tex
NRAR	=	$(INSTBINDIR)/nrar
NR2HTML	=	$(INSTBINDIR)/nr2html
NR2EHF	=	$(INSTBINDIR)/nr2ehf
NRORDER	=	$(INSTBINDIR)/nrorder
NRSEARCH=	$(INSTBINDIR)/nrsearch
HTS	=	$(INSTBINDIR)/hts
#hc31
#____________________________________________________________________________
#
# Define the modules to build to make.
#____________________________________________________________________________
#
HTSS	=	manrun.hts
HLPS	=	manrun.rtf
SMSS	=	manrun.sm
LIBS	=	manrun.lbn
PSOS	=	manrun.pso

MAN1	=	difftags.1	droff.1		hc.1		htmlc.1 \
		hts2html.1	idc.1		mantools.1	nr2html.1 \
		nr2rtf.1	nrar.1		nrcheck.1	nrinfo.1 \
		nrorder.1	nrsearch.1	ntags.1		sm2cat.1 \
		superman.1

MAN4	=	htp.4		hts.4		idc.4		lbn.4 \
		sm.4

MAN7	=	mad.7

TNIS	=	opt_e.tni	opt_h.tni	opt_hh.tni	opt_i.tni \
		opt_l.tni	opt_ll.tni	opt_m.tni	opt_n.tni \
		opt_o.tni	opt_p.tni	opt_q.tni	opt_s.tni \
		opt_t.tni	opt_v.tni	opt_x.tni

MANRUN	=	$(MAN1) $(MAN4) $(MAN7) $(TNIS)

TEX1	=	$(MAN1:.1=.tex)
TEX4	=	$(MAN1:.4=.tex)
TEX7	=	$(MAN1:.7=.tex)

all:: $(HLPS) $(HTSS)
install:: all
#____________________________________________________________________________
#
# Define the local build rules
#____________________________________________________________________________
#
# Build the local index page.
#
manrun.1:: manrun.sm manrun.1s
	$(NRSEARCH) -o $@ -n manrun.1s manrun.sm
manrun.lbn: manrun.sm manrun.1
	$(NRAR) -M manrun manrun.1 manrun.sm
clean::
	$(RM) manrun.1
	$(RM) manrun.lbn
#
# Build the main components.
#
manrun.sm:: $(MANRUN)
	$(SUPERMAN) -o $@ $(MANRUN)

manrun.hts: manrun.sm manrun.1 manrun.lbn forward.lbn
	$(NR2HTML) -x -M manrun	\
	           -c JASSPA \
		   -H jasspa \
		   -p toollogo.gif \
		   -L. -lmanrun -lforward \
		   -thome:manrun \
		   -tbuild:build \
		   manrun.sm manrun.1
manrun.rtf: manrun.sm manrun.1 manrun.lbn forward.lbn
	$(NR2RTF) -x -M manrun \
	          -c JASSPA \
		  -H jasspa \
		  -p toollogo.bmp \
		  -L. -lmanrun -lforward \
		  -thome:manrun \
		  -tbuild:build \
		  manrun.sm manrun.1
$(TEX1):	  manrun.sm manrun.1 manrun.lbn forward.lbn
	$(NR2TEX) -x -M manrun \
		  -c JASSPA \
		  -L. -lmanrun -lforward \
		  manrun.sm manrun.1
#
# Build the postscript print job ordering files.
#
manrun.pso:	manrun.sm
		$(NRORDER) $(NROFLAGS) -z -o $@ -i mantools manrun.sm

clean::
	- $(RM) *.sm
	- $(RM) *.hts
	- $(RM) *.pso
	- $(RM) *.log
	- $(RM) manrun.lbn
	- $(RM) manrun.fts
	- $(RM) manrun.ehf
	- $(RM) manrun.rtf
	- $(RM) manrun.gid
	- $(RM) *.hlp
	- $(RM) *.err

spotless:: clean
	- $(RM) *~
	- $(RM) tags
