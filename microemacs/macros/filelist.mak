##-*- makefile -*-############################################################
#
# Copyright (C) 2005-2025 JASSPA (www.jasspa.com).
#
# This is part of JASSPA's MicroEmacs, see the LICENSE file for licensing and
# copying information.
#
# Synopsis:    Installation Makefile
# Authors:     Jon Green & Steven Phillips
#
##############################################################################
#
# Basic macros to release
#
# EMFS:sh=ls *.emf
EMFS=	2dos.emf	2mac.emf	2unix.emf	2win.emf	\
	abbrev.emf	abbrlist.emf	aix.emf		\
	bintools.emf	bookmark.emf	brag.emf	\
	buffinit.emf	bytecomp.emf	\
	calc.emf	cfind.emf	charset.emf	charsutl.emf	\
	clearcs.emf	cmacros.emf	collapse.emf	comment.emf	\
	crosswd.emf	crtappln.emf	ctags.emf	cvs.emf		\
	cygwin.emf	\
	darwin.emf	diffline.emf    dmf.emf		docmacro.emf	\
	docutl.emf	dos.emf		draw.emf	\
	ehftools.emf	emftags.emf	etfinsrt.emf	fahtzee.emf	\
	fattrib.emf	favorite.emf	fileopen.emf	filetool.emf	\
	filetype.emf	find.emf	fold.emf	\
	format.emf	freebsd.emf	ftp.emf		\
	games.emf	gdiff.emf	gentagsc.emf	gentags.emf	\
	git.emf	\
	hkada.emf	hkadoc.emf	hkapache.emf	hkapt.emf	\
	hkasmx86.emf	\
	hkasn1.emf	hkasp.emf	hkau3.emf	hkawk.emf	\
	hkbibtex.emf	hkbinary.emf	hkblist.emf	hkbnf.emf	\
	hkc.emf		hkcfm.emf	hkcfms.emf	hkcobol.emf	\
	hkcpp.emf	hkcs.emf	hkcss.emf	hkcygwin.emf	\
	hkc3.emf	hkdart.emf	\
	hkdiff.emf	hkdirlst.emf	hkdirtre.emf	hkdman.emf	\
	hkdoc.emf	hkdos.emf	hkdot.emf	hkdtools.emf	\
	hkehf.emf	hkemf.emf	hkerf.emf	hkeuphor.emf	\
	hkf90.emf	hkfvwm.emf	hkgo.emf	hkhask.emf	\
	hkhtml.emf	\
	hkidl.emf	hkimake.emf	hkinfo.emf	hkini.emf	\
	hkipipe.emf	hkiss.emf	\
	hkjava.emf	hkjs.emf	hkjson.emf	hkjsp.emf	\
	hkjst.emf	hkjulia.emf	hkkotlin.emf	\
	hklatex.emf	hklhtml.emf	hklisp.emf	hklists.emf	\
	hklua.emf	\
	hkm4.emf	hkmake.emf	hkman.emf	hkmd.emf	\
	hkmeta.emf	hkmhg.emf	hkmod2.emf	hknim.emf	\
	hknroff.emf	hkoctave.emf	\
	hkpascal.emf	hkperl.emf	hkphp.emf	hkphps.emf	\
	hkpls.emf	hkps1.emf	hkpseudo.emf	hkpsf.emf	\
	hkpython.emf	\
	hkr.emf		hkrbin.emf	hkrd.emf	hkreg.emf	\
	hkruby.emf	hkrul.emf	hkrust.emf	\
	hksamba.emf	hkscheme.emf	hksgml.emf	hkshell.emf	\
	hkslang.emf	hkspec.emf	hksql.emf	hkswift.emf	\
	hktcl.emf	hktexi.emf	hktlsosd.emf	hktools.emf	\
	hktxt.emf	hktypst.emf	hkvala.emf	\
	hkv.emf		hkvb.emf	hkverilg.emf	hkvhdl.emf	\
	hkvrml.emf	hkwiki.emf	hkyaml.emf	\
	hkxml.emf	xmlutil.emf	\
	hkyacc.emf	hkzig.emf	\
	hpux.emf	htmlcore.emf	htmltool.emf	htmlutil.emf	\
	insdate.emf	irix.emf	itemlist.emf	\
	javatags.emf	jst2html.emf	jst2ltx.emf	jst2rtf.emf	\
	jst2text.emf	jstags.emf	jstutl.emf	\
	keyboard.emf	killlist.emf	\
	language.emf	langutl.emf	linux.emf	list.emf	\
	macos.emf	mahjongg.emf	mail.emf	mailmerg.emf	\
	majormod.emf	matchit.emf	matrix.emf	mdtools.emf	\
	me.emf		mecua.emf	meemacs.emf	melogo.emf	\
	meme3_8.emf	menedit.emf	mepinst.emf	meserver.emf	\
	meth.emf	metris.emf	misc.emf	mouse.emf	\
	mouseosd.emf	mouserec.emf	msshift.emf	\
	narrow.emf	newuser.emf	notes.emf	notesutl.emf	\
	ntags.emf	\
	openbsd.emf	openstep.emf	organiza.emf	organizd.emf	\
	organize.emf	organizi.emf	organizl.emf	organizm.emf	\
	organizt.emf	organizw.emf	osd.emf		osdcua.emf	\
	osdhelp.emf	osdmisc.emf	osdnedit.emf	osf1.emf	\
	pagefile.emf	password.emf	patience.emf	perltags.emf	\
	phptags.emf	print.emf	printall.emf	printd.emf	\
	printepc.emf	printers.emf	printf.emf	printstp.emf	\
	rect.emf	\
	scheme.emf	schemeaw.emf	schemeal.emf	schemead.emf	\
	schemeay.emf	schemeb.emf	schemebc.emf	schemech.emf	\
	schemecf.emf	schemecl.emf	schemecm.emf	schemecn.emf	\
	schemebh.emf	schemed.emf	\
	schemedp.emf	schemedr.emf	schemegb.emf	schemege.emf	\
	schemegg.emf	schemego.emf	schemej.emf	schemelm.emf	\
	schemel.emf	schemel2.emf	schemelj.emf	schememd.emf	\
	schememw.emf	schememx.emf	schemepd.emf	schemepl.emf	\
	schemes.emf	schemesf.emf	schemetb.emf	schemetw.emf	\
	schemesd.emf	schemesl.emf	schemetn.emf	\
	schemevi.emf	schemosd.emf	\
	search.emf	session.emf	sessnstp.emf	spell.emf	\
	spellaut.emf	spellutl.emf	ssaver.emf	sunos.emf	\
	svn.emf		tcltools.emf	\
	tcltags.emf	textags.emf	toolbar.emf	tooldef.emf	\
	toollist.emf	tools.emf	toolstd.emf	toolstp.emf	\
	triangle.emf	typing.emf	\
	unixterm.emf	userstp.emf	utils.emf	\
	vbtags.emf	vm.emf		\
	watch.emf	windows.emf	word.emf	\
	xfind.emf	xrdb.emf	\
	zaurus.emf	zfile.emf
# ETFS:sh =ls *.etf
ETFS=	ada.etf		adoc.etf	asmx86.etf	asn1.etf	\
	au3.etf		awk.etf		bibtex.etf	\
	c.etf		cfm.etf		cobol.etf	cpp.etf		\
	cs.etf		c3.etf\
	dart.etf	doc.etf		dot.etf		dtools.etf	\
	emf.etf		euphor.etf	\
	f.etf		f90.etf		hask.etf	\
	h.etf		hpp.etf		html.etf	go.etf	\
	idl.etf		imake.etf	\
	java.etf	js.etf		jst.etf		julia.etf	\
	kotlin.eaf	latex.etf	lua.etf		\
	makefile.etf	md.etf		mhg.etf		mod2.etf	\
	newcomp.etf	newuser.etf	nim.etf		\
	nroff.etf	octave.etf	\
	pascal.etf	perl.etf	php.etf		pls.etf		\
	ps1.etf		psf.etf		python.etf	qd.etf		\
	rd.etf		rmd.etf		ruby.etf	rul.etf		\
	rust.etf	shell.etf	spec.etf	sql.etf		\
	tcl.etf		typst.etf	\
	vala.etf	v.etf		vhdl.etf	\
	xml.etf		\
	yacc.etf	zig.etf
#EAFS:sh=ls *.eaf
EAFS=	ada.eaf		adoc.eaf	asp.eaf		au3.eaf		\
	awk.eaf		\
	c.eaf		c3.eaf		cfm.eaf		cpp.eaf		\
	cs.eaf		\
	dart.eaf	dot.eaf		dtools.eaf	emf.eaf		\
	euphor.eaf	f90.eaf		euphor.eaf	go.eaf		\
	hask.eaf	\
	hkasp.eaf	hkc.eaf		hkcpp.eaf	hkemf.eaf	\
	hkhtml.eaf	hkjsp.eaf	hkjst.eaf	hknroff.eaf	\
	hkphp.eaf	\
	html.eaf	\
	java.eaf	js.eaf		jsp.eaf		jst.eaf		\
	julia.eaf	kotlin.eaf	latex.eaf	lua.eaf		\
	md.eaf		nim.eaf		octave.eaf	\
	pascal.eaf	perl.eaf	php.eaf		pls.eaf		\
	ps1.eaf		python.eaf	\
	r.eaf		rd.eaf		rust.eaf	\
	shell.eaf	sgml.eaf	\
	tcl.eaf		txt.eaf		typst.eaf	\
	vala.eaf	v.eaf		vhdl.eaf	zig.eaf
# ERFS:sh=ls *.erf
ERFS=	jstprint.erf	newuser.erf	\
	print.erf	printeps.erf	printhpl.erf	printwdw.erf	\
	printd.erf	printhpd.erf	printhtm.erf
# ERFS:sh=ls *.edf
EDFS=	matchit.edf	password.lst
