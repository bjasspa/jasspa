##############################################################################
# This is part of the JASSPA MicroEmacs.
# Copyright (C) 2004 JASSPA (www.jasspa.com)
# See the file COPYING for copying and conditions.
#
# Created:       Thu Feb 5 21:36:24 2004
# Synopsis:      Installation Makefile
# Authors:       Jon Green
# Last Modified: <20040323.2047>
#
##############################################################################

TOPDIR=..
include $(TOPDIR)/etc/makeinc
#
# Basic macros to release
#
# EMFS:sh=ls *.emf
EMFS=	abbrev.emf	abbrlist.emf	aix.emf		au3.eaf		\
	au3.etf		\
	benchmrk.emf	bookmark.emf	buffinit.emf	buffstp.emf	\
	calc.emf	charset.emf	charsutl.emf	cmacros.emf	\
	collapse.emf	comment.emf	crosswd.emf	ctags.emf	\
	cvs.emf		cygwin.emf	\
	dmf.emf		docmacro.emf	docutl.emf	dos.emf		\
	dos2unix.emf	draw.emf	\
	ehftools.emf	emftags.emf	etfinsrt.emf	\
	fattrib.emf	favorite.emf	fileopen.emf	find.emf	\
	findbuff.emf	fold.emf	format.emf	freebsd.emf	\
	ftp.emf		\
	games.emf	gdiff.emf	gentags.emf	\
	hkada.emf	hkasmx86.emf	hkasn1.emf	hkasp.emf	\
	hkau3.emf	hkawk.emf	\
	hkbibtex.emf	hkbinary.emf	hkblist.emf	hkbnf.emf	\
	hkc.emf		hkcobol.emf	hkcpp.emf	hkcss.emf	\
	hkcygwin.emf	\
	hkdiff.emf	hkdirlst.emf	hkdirtre.emf	hkdoc.emf	\
	hkdos.emf	\
	hkeaf.emf	hkehf.emf	hkemf.emf	hkerf.emf	\
	hkeuphor.emf	\
	hkf90.emf	hkfvwm.emf	\
	hkhtml.emf	\
	hkidl.emf	hkimake.emf	hkinfo.emf	hkini.emf	\
	hkipipe.emf	\
	hkjava.emf	hkjs.emf	hkjsp.emf	hkjst.emf	\
	hklatex.emf	hklhtml.emf	hklisp.emf	hklists.emf	\
	hklua.emf	\
	hkm4.emf	hkmake.emf	hkman.emf	hkmeta.emf	\
	hkmhg.emf	\
	hknroff.emf	\
	hkpascal.emf	hkperl.emf	hkphp.emf	hkphps.emf	\
	hkpls.emf	hkpseudo.emf	hkpsf.emf	hkpython.emf	\
	hkreg.emf	hkrul.emf	\
	hkscheme.emf	hksgml.emf	hkshell.emf	hkslang.emf	\
	hkspec.emf	hksql.emf	\
	hktcl.emf	hktexi.emf	hktxt.emf	\
	hkvb.emf	hkvhdl.emf	hkvrml.emf	\
	hkxml.emf	\
	hkyacc.emf	\
	hpux.emf	htmlcore.emf	htmltool.emf	htmlutil.emf	\
	insdate.emf	irix.emf	itemlist.emf	\
	javatags.emf	jst2html.emf	jst2ltx.emf	jst2rtf.emf	\
	jst2text.emf	jstags.emf	jstutl.emf	\
	keyboard.emf	killlist.emf	\
	language.emf	langutl.emf	linux.emf	list.emf	\
	mahjongg.emf	mail.emf	mailmerg.emf	matchit.emf	\
	me.emf		melogo.emf	meme3_8.emf	menedit.emf	\
	meserver.emf	metris.emf	misc.emf	mouse.emf	\
	mouseosd.emf	msshift.emf	\
	narrow.emf	newuser.emf	ntags.emf	\
	openbsd.emf	openstep.emf	organiza.emf	organizd.emf	\
	organize.emf	organizi.emf	organizl.emf	organizm.emf	\
	organizt.emf	organizw.emf	osd.emf		osdhelp.emf	\
	osdnedit.emf	osf1.emf	\
	pagefile.emf	patience.emf	perltags.emf	print.emf	\
	printall.emf	printd.emf	printepc.emf	printers.emf	\
	printf.emf	printstp.emf	\
	rect.emf	\
	scheme.emf	schemeb.emf	schemebc.emf	schemebd.emf	\
	schemebh.emf	schemed.emf	schemej.emf	schemel.emf	\
	schemelj.emf	schememd.emf	schemepd.emf	schemepl.emf	\
	schemes.emf	schemesf.emf	schemetb.emf	schemevi.emf	\
	schemosd.emf	search.emf	spell.emf	spellaut.emf	\
	spellutl.emf	ssaver.emf	sunos.emf	\
	tcltags.emf	textags.emf	toolbar.emf	tooldef.emf	\
	toollist.emf	tools.emf	toolstd.emf	toolstp.emf	\
	triangle.emf	\
	unix2dos.emf	unix2win.emf	unixterm.emf	userstp.emf	\
	utils.emf	\
	vbtags.emf	vm.emf		\
	watch.emf	win32.emf	word.emf	\
	zaurus.emf	zfile.emf
# ETFS:sh =ls *.etf
ETFS=	asmx86.etf	asn1.etf	\
	bibtex.etf	\
	c.etf		cobol.etf	cpp.etf		\
	doc.etf		\
	emf.etf		euphor.etf	\
	f.etf		f90.etf		\
	h.etf		hpp.etf		html.etf	\
	idl.etf		imake.etf	\
	java.etf	js.etf		jst.etf		\
	latex.etf	\
	makefile.etf	mhg.etf		\
	newcomp.etf	newuser.etf	nroff.etf	\
	pascal.etf	perl.etf	pls.etf		psf.etf		\
	rul.etf		\
	spec.etf	\
	tcl.etf		\
	vhdl.etf	\
	xml.etf
#EAFS:sh=ls *.eaf
EAFS=	ada.eaf		asp.eaf		\
	c.eaf		cpp.eaf		\
	emf.eaf		euphor.eaf	\
	hkasp.eaf	hkc.eaf		hkemf.eaf	hkhtml.eaf	\
	hkjsp.eaf	hkjst.eaf	hknroff.eaf	hkphp.eaf	\
	html.eaf	\
	java.eaf	js.eaf		jsp.eaf		jst.eaf		\
	latex.eaf	\
	pascal.eaf	perl.eaf	php.eaf		pls.eaf		\
	sgml.eaf	\
	tcl.eaf		txt.eaf		\
	vhdl.eaf
# ERFS:sh=ls *.erf
ERFS=	jstprint.erf	\
	print.erf	printeps.erf	printhpl.erf	printwdw.erf	\
	printd.erf	printhpd.erf	printhtm.erf	\
	newuser.erf
# ERFS:sh=ls *.edf
EDFS=	matchit.edf
OTHS=	null
#
# Destination directory
#
RELEMFS	= $(EMFS:%=$(JASMACDIR)/%)
RELETFS	= $(ETFS:%=$(JASMACDIR)/%)
RELEAFS	= $(EAFS:%=$(JASMACDIR)/%)
RELERFS	= $(ERFS:%=$(JASMACDIR)/%)
RELEDFS	= $(EDFS:%=$(JASMACDIR)/%)
RELOTHS = $(OTHS:%=$(JASMACDIR)/%)
#
# Build everything
#
all:
	echo "Nothing to do for $@"
#
# Build Release
#
release::	$(JASMACDIR) $(RELEMFS) $(RELETFS) $(RELEAFS) \
		$(RELERFS) $(RELEDFS) $(RELOTHS)
# Release target files
$(RELEMFS): $(EMFS)
	$(INSTALL_FIL) $(@F) $(@D)
$(RELETFS): $(ETFS)
	$(INSTALL_FIL) $(@F) $(@D)
$(RELEAFS): $(EAFS)
	$(INSTALL_FIL) $(@F) $(@D)
$(RELERFS): $(ERFS)
	$(INSTALL_FIL) $(@F) $(@D)
$(RELEDFS): $(EDFS)
	$(INSTALL_FIL) $(@F) $(@D)
$(RELOTHS): $(OTHERS)
	$(INSTALL_FIL) $(@F) $(@D)
# Release target directory.
$(JASMACDIR):
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
