#____________________________________________________________________________
#
# Author:	Jon Green
#
# Last Modified <001021.1450>
#
# Description:	The dudev component describes the utilities library. This
#		includes the file searching functions etc.
#
#____________________________________________________________________________
#
# Define the modules to build to make.
#
RM	=	erase
INSTALL	=	$(RUNBINDIR)\install
MKDIR	=	mkdir
COPY	=	copy
#
# Documentation components
#
DOCBIN	=	
DOCDIR	=	

SUPERMAN=	$(DOCBIN)superman -b
NR2RTF	=	$(DOCBIN)nr2rtf
NRAR	=	$(DOCBIN)nrar
NR2HTML	=	$(DOCBIN)nr2html
NR2EHF	=	$(DOCBIN)nr2ehf
NRORDER	=	$(DOCBIN)nrorder
NRSEARCH=	$(DOCBIN)nrsearch
HTS	=	$(DOCBIN)hts
HC31	=	hc31
#hc31
#____________________________________________________________________________
#
# Define the modules to build to make.
#____________________________________________________________________________
#
HTSS	=	manrun.hts
HLPS	=	manrun.hlp
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

all:: $(HLPS) $(HTSS)
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
	$(NR2HTML) -x -M manrun manrun.sm manrun.1	\
	           -c JASSPA                            \
		   -H jasspa				\
		   -p toollogo.gif			\
		   -L. -lmanrun -lforward		\
		   -thome:manrun			\
		   -tbuild:build

manrun.hlp: manrun.sm manrun.1 manrun.lbn forward.lbn
	$(NR2RTF) -x -M manrun manrun.sm manrun.1	\
	           -c JASSPA                            \
		   -H jasspa				\
		   -p toollogo.bmp			\
		   -L. -lmanrun -lforward		\
		   -thome:manrun			\
		   -tbuild:build
	$(HC31) manrun
	$(RM) manrun.rtf
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
	- attrib -H manrun.gid
	- $(RM) manrun.gid
	- $(RM) *.hlp
	- $(RM) *.err

spotless:: clean
	- $(RM) *.??~
	- $(RM) tags
#____________________________________________________________________________
#
# Standard make rules here.
#____________________________________________________________________________
#
#include $(MAKEBODY)
