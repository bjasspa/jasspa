##############################################################################
# This is part of the JASSPA MicroEmacs.
# Copyright (C) 2005-2009 JASSPA (www.jasspa.com)
# See the file COPYING for copying and conditions.
#
# Created:       Thu Feb 5 21:36:24 2004
# Synopsis:      Installation Makefile
# Authors:       Jon Green & Steven Phillips
#
##############################################################################

TOPDIR=..
include $(TOPDIR)/etc/makeinc
#
# Basic macros to release
#
# EMFS:sh=ls *.emf
EMFS=	2dos.emf	2mac.emf	2unix.emf	2win.emf	\
	abbrev.emf	abbrlist.emf	aix.emf		\
	benchmrk.emf	bookmark.emf	brag.emf	buffinit.emf	\
	calc.emf	cfind.emf	charset.emf	charsutl.emf	\
	clearcs.emf	cmacros.emf	collapse.emf	comment.emf	\
	crosswd.emf	ctags.emf	cvs.emf		cygwin.emf	\
	darwin.emf	dmf.emf		docmacro.emf	docutl.emf	\
	dos.emf		draw.emf	\
	ehftools.emf	emftags.emf	etfinsrt.emf	\
	fattrib.emf	favorite.emf	fileopen.emf	filetool.emf	\
	filetype.emf	find.emf	fold.emf	format.emf	\
	freebsd.emf	ftp.emf		\
	games.emf	gdiff.emf	gentags.emf	\
	hkada.emf	hkapache.emf	hkasmx86.emf	hkasn1.emf	\
	hkasp.emf	hkau3.emf	hkawk.emf	\
	hkbibtex.emf	hkbinary.emf	hkblist.emf	hkbnf.emf	\
	hkc.emf		hkcfm.emf	hkcfms.emf	hkcobol.emf	\
	hkcpp.emf	hkcss.emf	hkcygwin.emf	\
	hkdiff.emf	hkdirlst.emf	hkdirtre.emf	hkdoc.emf	\
	hkdos.emf	hkdot.emf	\
	hkehf.emf	hkemf.emf	hkerf.emf	hkeuphor.emf	\
	hkf90.emf	hkfvwm.emf	\
	hkhtml.emf	\
	hkidl.emf	hkimake.emf	hkinfo.emf	hkini.emf	\
	hkiss.emf	\
	hkipipe.emf	\
	hkjava.emf	hkjs.emf	hkjsp.emf	hkjst.emf	\
	hklatex.emf	hklhtml.emf	hklisp.emf	hklists.emf	\
	hklua.emf	\
	hkm4.emf	hkmake.emf	hkman.emf	hkmeta.emf	\
	hkmhg.emf	hkmod2.emf	\
	hknroff.emf	\
	hkpascal.emf	hkperl.emf	hkphp.emf	hkphps.emf	\
	hkpls.emf	hkpseudo.emf	hkpsf.emf	hkpython.emf	\
	hkr.emf		hkreg.emf	hkruby.emf	hkrul.emf	\
	hksamba.emf	hkscheme.emf	hksgml.emf	hkshell.emf	\
	hkslang.emf	hkspec.emf	hksql.emf	hkswift.emf	\
	hktcl.emf	hktexi.emf	hktxt.emf	\
	hkvb.emf	hkverilg.emf	hkvhdl.emf	hkvrml.emf	\
	hkwiki.emf	\
	hkxml.emf	xmlutil.emf	\
	hkyacc.emf	\
	hpux.emf	htmlcore.emf	htmltool.emf	htmlutil.emf	\
	insdate.emf	irix.emf	itemlist.emf	\
	javatags.emf	jst2html.emf	jst2ltx.emf	jst2rtf.emf	\
	jst2text.emf	jstags.emf	jstutl.emf	\
	keyboard.emf	killlist.emf	\
	language.emf	langutl.emf	linux.emf	list.emf	\
	macos.emf	mahjongg.emf	mail.emf	mailmerg.emf	\
	majormod.emf	matchit.emf	me.emf		mecua.emf	\
	meemacs.emf	melogo.emf	meme3_8.emf	menedit.emf	\
	meserver.emf	metris.emf	misc.emf	mouse.emf	\
	mouseosd.emf	mouserec.emf	msshift.emf	\
	narrow.emf	newuser.emf	notes.emf	notesutl.emf	\
	ntags.emf	\
	openbsd.emf	openstep.emf	organiza.emf	organizd.emf	\
	organize.emf	organizi.emf	organizl.emf	organizm.emf	\
	organizt.emf	organizw.emf	osd.emf		osdcua.emf	\
	osdhelp.emf	osdmisc.emf	osdnedit.emf	osf1.emf	\
	pagefile.emf	password.emf	patience.emf	perltags.emf	\
	print.emf	printall.emf	printd.emf	printepc.emf	\
	printers.emf	printf.emf	printstp.emf	\
	rect.emf	\
	scheme.emf	schemeb.emf	schemebc.emf	schemech.emf	\
	schemebh.emf	schemed.emf	schemegb.emf	schemege.emf	\
	schemegg.emf	schemego.emf	schemej.emf	schemelm.emf	\
	schemel.emf	schemel2.emf	schemelj.emf	schememd.emf	\
	schememw.emf	schemepd.emf	schemepl.emf	schemes.emf	\
	schemesf.emf	schemetb.emf	schemetw.emf	schemevi.emf	\
	schemosd.emf	\
	search.emf	session.emf	sessnstp.emf	spell.emf	\
	spellaut.emf	spellutl.emf	ssaver.emf	sunos.emf	\
	svn.emf		\
	tcltags.emf	textags.emf	toolbar.emf	tooldef.emf	\
	toollist.emf	tools.emf	toolstd.emf	toolstp.emf	\
	triangle.emf	\
	unixterm.emf	userstp.emf	utils.emf	\
	vbtags.emf	vm.emf		\
	watch.emf	win32.emf	word.emf	\
	xfind.emf	\
	zaurus.emf	zfile.emf
# ETFS:sh =ls *.etf
ETFS=	asmx86.etf	asn1.etf	au3.etf		\
	bibtex.etf	\
	c.etf		cfm.etf		cobol.etf	cpp.etf		\
	doc.etf		\
	emf.etf		euphor.etf	\
	f.etf		f90.etf		\
	h.etf		hpp.etf		html.etf	\
	idl.etf		imake.etf	\
	java.etf	js.etf		jst.etf		\
	latex.etf	\
	makefile.etf	mhg.etf		mod2.etf	\
	newcomp.etf	newuser.etf	nroff.etf	\
	pascal.etf	perl.etf	pls.etf		psf.etf		\
	python.etf	\
	ruby.etf	rul.etf		\
	spec.etf	\
	tcl.etf		\
	vhdl.etf	\
	xml.etf		\
	yacc.etf
#EAFS:sh=ls *.eaf
EAFS=	ada.eaf		asp.eaf		au3.eaf		\
	c.eaf		cfm.eaf		cpp.eaf		\
	emf.eaf		euphor.eaf	\
	hkasp.eaf	hkc.eaf		hkcpp.eaf	hkemf.eaf	\
	hkhtml.eaf	hkjsp.eaf	hkjst.eaf	hknroff.eaf	\
	hkphp.eaf	\
	html.eaf	\
	java.eaf	js.eaf		jsp.eaf		jst.eaf		\
	latex.eaf	\
	pascal.eaf	perl.eaf	php.eaf		pls.eaf		\
	python.eaf	\
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
