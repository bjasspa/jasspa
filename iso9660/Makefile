##############################################################################
#
#			Copyright 2003 Jon Green.
#			      All Rights Reserved
#
#
#  System        :
#  Module        :
#  Object Name   : $RCSfile: Makefile,v $
#  Revision      : $Revision: 1.2 $
#  Date          : $Date: 2003-08-14 17:09:31 $
#  Author        : $Author: jon $
#  Created By    : Jon Green
#  Created       : Sun Aug 10 11:57:07 2003
#  Last Modified : <030814.1653>
#
#  Description
#
#  Notes
#
#  History
#
#  $Log: not supported by cvs2svn $
#  Revision 1.1  2003/08/10 22:49:11  jon
#  CD ISO9660 Image build directory
#
#
##############################################################################
#
#  Copyright (c) 2003 Jon Green.
#
#  All Rights Reserved.
#
#  This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
#  reproduced,  translated,  or  reduced to any  electronic  medium or machine
#  readable form without prior written consent from Jon Green.
#
##############################################################################

#
# Define the image source locations
#
SPLDIR=/home/jon/www.jasspa.com/spelling
RELDIR=/home/jon/www.jasspa.com/release_021205
EXPDIR=/home/jon/www.jasspa.com/experimental
MISDIR=/home/jon/www.jasspa.com/misc
SUBDIR=/home/jon/www.jasspa.com/subs

FLABEL="jasspa0308f11"
ELABEL="jasspa0308e11"
FDISK=jasspafull.iso
EDISK=jasspaexec.iso
FZIP =jasspafull.zip
EZIP =jasspaexec.zip
FDIR = fcd
EDIR = ecd

FDIRS=	$(FDIR) \
	$(FDIR)/ne \
	$(FDIR)/archive \
	$(FDIR)/archive/spelling \
	$(FDIR)/doc \
	$(FDIR)/packages \
	$(FDIR)/src \
	$(FDIR)/macros \
	$(FDIR)/me \
	$(FDIR)/archive \
	$(FDIR)/experimental \
	$(FDIR)/submissions \
	$(FDIR)/cdlabels \
	$(FDIR)/icons

EDIRS=	$(EDIR) \
	$(EDIR)/ne \
	$(EDIR)/me \
	$(EDIR)/cdlabels \
	$(EDIR)/archive	
#
# Basic tools
#
RM	= rm -f

MKISOFS=	mkisofs -r -J -hfs
FULLMKISOFSFLAGS=-graft-points						\
		-hide readme.hfs -hide readme.joliet		\
		-hide-joliet readme.hfs -hide-joliet readme.unix	\
		-hide-hfs readme.joliet -hide-hfs readme.unix		\
		-hide faq.hfs -hide faq.joliet			\
		-hide-joliet faq.hfs -hide-joliet faq.unix		\
		-hide-hfs faq.joliet -hide-hfs faq.unix			\
		-hide build.hfs -hide build.joliet			\
		-hide-joliet build.hfs -hide-joliet build.unix		\
		-hide-hfs build.joliet -hide-hfs build.unix		\
		-hide copying.hfs -hide copying.joliet		\
		-hide-joliet copying.hfs -hide-joliet copying.unix	\
		-hide-hfs copying.joliet -hide-hfs copying.unix		\
		-hide cdrom.hfs -hide cdrom.joliet			\
		-hide-joliet cdrom.hfs -hide-joliet cdrom.unix		\
		-hide-hfs cdrom.joliet -hide-hfs cdrom.unix		\
		-hide license.hfs -hide license.joliet		\
		-hide-joliet license.hfs -hide-joliet license.unix \
		-hide-hfs license.joliet -hide-hfs license.unix		\
		-hide doc/readme.hfs -hide doc/readme.joliet		\
		-hide-joliet doc/readme.hfs -hide-joliet doc/readme.unix \
		-hide-hfs doc/readme.joliet -hide-hfs doc/readme.unix	\
		-hide ne/readme.hfs -hide ne/readme.joliet		\
		-hide-joliet ne/readme.hfs -hide-joliet ne/readme.unix \
		-hide-hfs ne/readme.joliet -hide-hfs ne/readme.unix	\
		-hide archive/spelling/readme.hfs -hide archive/spelling/readme.joliet		\
		-hide-joliet archive/spelling/readme.hfs -hide-joliet archive/spelling/readme.unix \
		-hide-hfs archive/spelling/readme.joliet -hide-hfs archive/spelling/readme.unix	\
		-hide packages/readme.hfs -hide packages/readme.joliet \
		-hide-joliet packages/readme.hfs -hide-joliet packages/readme.unix \
		-hide-hfs packages/readme.joliet -hide-hfs packages/readme.unix	\
	        readme.txt=readme.hfs					\
	        readme.txt=readme.joliet				\
	        readme.txt=readme.unix					\
	        faq.txt=faq.hfs						\
	        faq.txt=faq.joliet					\
	        faq.txt=faq.unix					\
	        build.txt=build.hfs					\
	        build.txt=build.joliet					\
	        build.txt=build.unix					\
	        COPYING.txt=copying.hfs					\
	        COPYING.txt=copying.joliet				\
	        COPYING.txt=copying.unix				\
	        CDROM.txt=cdrom.hfs					\
	        CDROM.txt=cdrom.joliet					\
	        CDROM.txt=cdrom.unix					\
	        license.txt=license.hfs					\
	        license.txt=license.joliet				\
	        license.txt=license.unix				\
	        doc/readme.txt=doc/readme.hfs				\
	        doc/readme.txt=doc/readme.joliet			\
	        doc/readme.txt=doc/readme.unix				\
	        ne/readme.txt=ne/readme.hfs				\
	        ne/readme.txt=ne/readme.joliet			\
	        ne/readme.txt=ne/readme.unix				\
	        archive/spelling/readme.txt=archive/spelling/readme.hfs \
	        archive/spelling/readme.txt=archive/spelling/readme.joliet \
	        archive/spelling/readme.txt=archive/spelling/readme.unix \
	        packages/readme.txt=packages/readme.hfs \
	        packages/readme.txt=packages/readme.joliet \
	        packages/readme.txt=packages/readme.unix \

EXECMKISOFSFLAGS=-graft-points						\
		-hide readme.hfs -hide readme.joliet		\
		-hide-joliet readme.hfs -hide-joliet readme.unix	\
		-hide-hfs readme.joliet -hide-hfs readme.unix		\
		-hide faq.hfs -hide faq.joliet			\
		-hide-joliet faq.hfs -hide-joliet faq.unix		\
		-hide-hfs faq.joliet -hide-hfs faq.unix			\
		-hide build.hfs -hide build.joliet			\
		-hide-joliet build.hfs -hide-joliet build.unix		\
		-hide-hfs build.joliet -hide-hfs build.unix		\
		-hide copying.hfs -hide copying.joliet		\
		-hide-joliet copying.hfs -hide-joliet copying.unix	\
		-hide-hfs copying.joliet -hide-hfs copying.unix		\
		-hide cdrom.hfs -hide cdrom.joliet			\
		-hide-joliet cdrom.hfs -hide-joliet cdrom.unix		\
		-hide-hfs cdrom.joliet -hide-hfs cdrom.unix		\
		-hide license.hfs -hide license.joliet		\
		-hide-joliet license.hfs -hide-joliet license.unix \
		-hide-hfs license.joliet -hide-hfs license.unix		\
		-hide ne/readme.hfs -hide ne/readme.joliet		\
		-hide-joliet ne/readme.hfs -hide-joliet ne/readme.unix \
		-hide-hfs ne/readme.joliet -hide-hfs ne/readme.unix	\
	        readme.txt=readme.hfs					\
	        readme.txt=readme.joliet				\
	        readme.txt=readme.unix					\
	        faq.txt=faq.hfs						\
	        faq.txt=faq.joliet					\
	        faq.txt=faq.unix					\
	        build.txt=build.hfs					\
	        build.txt=build.joliet					\
	        build.txt=build.unix					\
	        COPYING.txt=copying.hfs					\
	        COPYING.txt=copying.joliet				\
	        COPYING.txt=copying.unix				\
	        CDROM.txt=cdrom.hfs					\
	        CDROM.txt=cdrom.joliet					\
	        CDROM.txt=cdrom.unix					\
	        license.txt=license.hfs					\
	        license.txt=license.joliet				\
	        license.txt=license.unix				\
	        ne/readme.txt=ne/readme.hfs				\
	        ne/readme.txt=ne/readme.joliet			\
	        ne/readme.txt=ne/readme.unix				\

all:	$(FZIP) $(EZIP)

clean:
	rm -rf ./$(FDIR)
	rm -rf ./$(EDIR)
	rm -f *~
	rm -f make.err

spotless: clean
	rm -f $(FZIP) $(EZIP)
	rm -f *.iso
#
# Root directory readme's
# 
$(FDIR) $(EDIR):
	mkdir -p $@
	cp $(RELDIR)/faq.txt $@/faq.unix
	unix2dos $(RELDIR)/faq.txt $@/faq.joliet
	unix2mac $(RELDIR)/faq.txt $@/faq.hfs
	cp $(RELDIR)/license.txt $@/license.unix
	unix2dos $(RELDIR)/license.txt $@/license.joliet
	unix2mac $(RELDIR)/license.txt $@/license.hfs
	cp $(RELDIR)/COPYING $@/copying.unix
	unix2dos $(RELDIR)/COPYING $@/copying.joliet
	unix2mac $(RELDIR)/COPYING $@/copying.hfs
	cp $(RELDIR)/build.txt $@/build.unix
	unix2dos $(RELDIR)/build.txt $@/build.joliet
	unix2mac $(RELDIR)/build.txt $@/build.hfs
	cp $(RELDIR)/readme.txt $@/readme.unix
	unix2dos $(RELDIR)/readme.txt $@/readme.joliet
	unix2mac $(RELDIR)/readme.txt $@/readme.hfs
	cp cdrom.txt $@/cdrom.unix
	unix2dos cdrom.txt $@/cdrom.joliet
	unix2mac cdrom.txt $@/cdrom.hfs
#
# Nano Emacs excecutable directory.
# 
$(FDIR)/ne $(EDIR)/ne:	ne.txt
	mkdir -p $@
	gunzip -c $(RELDIR)/nelinux24.gz > $@/nelinux24
	chmod a+rx $@/nelinux24
	gunzip -c $(RELDIR)/nesunos56.gz > $@/nesunos56
	chmod a+rx $@/nesunos56
	unzip -o $(RELDIR)/nesunos58.zip -d $@
	chmod a+rx $@/nesunos58
	unzip -o $(RELDIR)/nesunos59.zip -d $@
	chmod a+rx $@/nesunos59
	unzip -o $(RELDIR)/nesunx86_58.zip -d $@
	chmod a+rx $@/nesunx86_58
	gunzip -c $(RELDIR)/neirix6.gz > $@/neirix6
	chmod a+rx $@/neirix6
	gunzip -c $(RELDIR)/nehpux10.gz > $@/nehpux10
	chmod a+rx $@/nehpux10
	gunzip -c $(RELDIR)/nehpux11.gz > $@/nehpux11
	chmod a+rx $@/nehpux11
	gunzip -c $(RELDIR)/neaix4.gz > $@/neaix4
	chmod a+rx $@/neaix4
	gunzip -c $(RELDIR)/necygwin.exe.gz > $@/necygwin.exe
	chmod a+rx $@/necygwin.exe
	unzip -o $(RELDIR)/newin32.zip -d $@
	unzip -o $(RELDIR)/necwin32.zip -d $@
	cp $(RELDIR)/ne.emf $@
	cp ne.txt $@/readme.unix
	unix2dos ne.txt $@/readme.joliet
	unix2mac ne.txt $@/readme.hfs
#
# Spelling dictionaries.
# 
$(FDIR)/archive/spelling: spell.txt
	mkdir -p $@
	cp $(SPLDIR)/ls_dede.tar.gz $(SPLDIR)/ls_dede.zip $@
	cp $(SPLDIR)/ls_engb.tar.gz $(SPLDIR)/ls_engb.zip $@
	cp $(SPLDIR)/ls_enus.tar.gz $(SPLDIR)/ls_enus.zip $@
	cp $(SPLDIR)/ls_eses.tar.gz $(SPLDIR)/ls_eses.zip $@
	cp $(SPLDIR)/ls_fifi.tar.gz $(SPLDIR)/ls_fifi.zip $@
	cp $(SPLDIR)/ls_frfr.tar.gz $(SPLDIR)/ls_frfr.zip $@
	cp $(SPLDIR)/ls_itit.tar.gz $(SPLDIR)/ls_itit.zip $@
	cp $(SPLDIR)/ls_plpl.tar.gz $(SPLDIR)/ls_plpl.zip $@
	cp $(SPLDIR)/ls_ptpt.tar.gz $(SPLDIR)/ls_ptpt.zip $@
	cp spell.txt $@/readme.unix
	unix2dos spell.txt $@/readme.joliet
	unix2mac spell.txt $@/readme.hfs
#
# Documentation directories.
#
$(FDIR)/doc: doc.txt
	mkdir -p $@
	(cd $@; gunzip -c $(RELDIR)/mehtml.tar.gz | tar xvf -)
	unzip -o $(RELDIR)/mewinhlp.zip -d $@
	cp me2002.chm $@
	cp me2002.pdf $@
	cp $(MISDIR)/docmode.txt $@
	cp doc.txt $@/readme.unix
	unix2dos doc.txt $@/readme.joliet
	unix2mac doc.txt $@/readme.hfs
#
# Packages.
# 
$(FDIR)/packages:
	mkdir -p $@/cygwin
	cp $(RELDIR)/jasspa-20021216-src.tar.bz2 $@/cygwin
	unix2dos $(RELDIR)/jasspa-20021216.README $@/cygwin/readme.txt
	cp $(RELDIR)/jasspa-20021216.tar.bz2 $@/cygwin
	mkdir -p $@/windows
	cp $(RELDIR)/jasspame.exe $@/windows
	mkdir -p $@/sun
	unzip -o $(RELDIR)/sun-sparc-9-studio7-jasspame-20021216.zip -d  $@/sun
	mv $@/sun/jasspame-20021216 $@/sun/jasspa-sparc-sol9.pkg
	unzip -o $(RELDIR)/sun-sparc-8-jasspame-20021216.zip -d  $@/sun
	mv $@/sun/jasspame-20021216 $@/sun/jasspa-sparc-sol8.pkg
	unzip -o $(RELDIR)/sun-i386-8-jasspame-20021216.zip -d  $@/sun
	mv $@/sun/jasspame-20021216 $@/sun/jasspa-i386-sol8.pkg
	mkdir -p $@/linux
	cp microemacs-jasspa-2002_12-1.i386.rpm $@/linux
	cp package.txt $@/readme.unix
	unix2dos package.txt $@/readme.joliet
	unix2mac package.txt $@/readme.hfs
#
# Source directory
#
$(FDIR)/src:
	mkdir -p $@
	(cd $@; gunzip -c $(RELDIR)/mesrc.tar.gz | tar xvf -)
#
# Macros directory.
#
$(FDIR)/macros:
	mkdir -p $@
	(cd $@; gunzip -c $(RELDIR)/memacros.tar.gz | tar xvf -)
#
# Executable direcotory.
# 
$(FDIR)/me $(EDIR)/me:
	mkdir -p $@
	(cd $@; gunzip -c $(RELDIR)/memacros.tar.gz | tar xvf -)
	gunzip -c $(RELDIR)/me.ehf.gz > $@/me.ehf
	gunzip -c $(RELDIR)/melinux24.gz > $@/melinux24
	chmod a+rx $@/melinux24
	gunzip -c $(RELDIR)/mesunos56.gz > $@/mesunos56
	chmod a+rx $@/mesunos56
	unzip -o $(RELDIR)/mesunos59.zip -d $@
	chmod a+rx $@/mesunos59
	unzip -o $(RELDIR)/mesunos58.zip -d $@
	chmod a+rx $@/mesunos58
	unzip -o $(RELDIR)/mesunx86_58.zip -d $@
	chmod a+rx $@/mesunx86_58
	gunzip -c $(RELDIR)/meirix6.gz > $@/meirix6
	chmod a+rx $@/meirix6
	gunzip -c $(RELDIR)/mehpux10.gz > $@/mehpux10
	chmod a+rx $@/mehpux10
	gunzip -c $(RELDIR)/mehpux11.gz > $@/mehpux11
	chmod a+rx $@/mehpux11
	gunzip -c $(RELDIR)/meaix4.gz > $@/meaix4
	chmod a+rx $@/meaix4
	unzip -o $(RELDIR)/mewin32.zip -d $@
	unzip -o $(RELDIR)/mecwin32.zip -d $@
#
# Executable directory.
# 
$(FDIR)/bin:
	gunzip -c $(RELDIR)/melinux24.gz > $@/melinux24
	chmod a+rx $@/melinux24
	gunzip -c $(RELDIR)/mesunos56.gz > $@/mesunos56
	chmod a+rx $@/mesunos56
	unzip -o $(RELDIR)/mesunos58.zip -d $@
	chmod a+rx $@/mesunos58
	unzip -o $(RELDIR)/mesunx86_58.zip -d $@
	chmod a+rx $@/mesunx86_58
	unzip -o $(RELDIR)/mesunos59.zip -d $@
	chmod a+rx $@/mesunos59
	gunzip -c $(RELDIR)/meirix6.gz > $@/meirix6
	chmod a+rx $@/meirix6
	gunzip -c $(RELDIR)/mehpux10.gz > $@/mehpux10
	chmod a+rx $@/mehpux10
	gunzip -c $(RELDIR)/mehpux11.gz > $@/mehpux11
	chmod a+rx $@/mehpux11
	gunzip -c $(RELDIR)/meaix4.gz > $@/meaix4
	chmod a+rx $@/meaix4
	unzip -o $(RELDIR)/mewin32.zip -d $@
	unzip -o $(RELDIR)/mecwin32.zip -d $@
#
# Archive directories.
#
$(FDIR)/archive:
	mkdir -p $@
	mkdir -p $@/src
	cp $(RELDIR)/mesrc.tar.gz $@/src
	cp $(RELDIR)/mesrc.zip $@/src
	mkdir -p $@/bin
	cp $(RELDIR)/mesunos56.gz $@/bin
	cp $(RELDIR)/mesunos58.zip $@/bin
	cp $(RELDIR)/mesunx86_58.zip $@/bin
	cp $(RELDIR)/mesunos59.zip $@/bin
	cp $(RELDIR)/meuwin32.zip $@/bin
	cp $(RELDIR)/mewin32.zip $@/bin
	cp $(RELDIR)/neaix4.gz $@/bin
	cp $(RELDIR)/necwin32.zip $@/bin
	cp $(RELDIR)/necygwin.exe.gz $@/bin
	cp $(RELDIR)/meaix4.gz $@/bin
	cp $(RELDIR)/nehpux10.gz $@/bin
	cp $(RELDIR)/mecwin32.zip $@/bin
	cp $(RELDIR)/nehpux11.gz $@/bin
	cp $(RELDIR)/neirix6.gz $@/bin
	cp $(RELDIR)/mehpux10.gz $@/bin
	cp $(RELDIR)/nelinux24.gz $@/bin
	cp $(RELDIR)/mehpux11.gz $@/bin
	cp $(RELDIR)/nesunos56.gz $@/bin
	cp $(RELDIR)/nesunos58.zip $@/bin
	cp $(RELDIR)/nesunx86_58.zip $@/bin
	cp $(RELDIR)/nesunos59.zip $@/bin
	cp $(RELDIR)/newin32.zip $@/bin
	cp $(RELDIR)/meirix6.gz $@/bin
	cp $(RELDIR)/melinux24.gz $@/bin
	mkdir -p $@/macros
	cp $(RELDIR)/me.ehf.gz $@/macros
	cp $(RELDIR)/meehf.zip $@/macros
	cp $(RELDIR)/memacros.tar.gz $@/macros
	cp $(RELDIR)/memacros.zip $@/macros
	mkdir -p $@/doc
	cp $(RELDIR)/mehtm.zip $@/doc
	cp $(RELDIR)/mehtml.tar.gz $@/doc
	cp $(RELDIR)/mewinhlp.zip $@/doc
	mkdir -p $@/icons
	cp $(RELDIR)/meicons.tar.gz $@/icons
	cp $(RELDIR)/../release_020105/meicons.zip $@/icons
	cp $(RELDIR)/readme.jasspa_gnome $@/icons
	cp $(RELDIR)/jasspa_gnome.tar.zip $@/icons
	cp $(RELDIR)/meicons-extra.tar.gz $@/icons
	cp $(RELDIR)/meicons-extra.tar.zip $@/icons
	cp $(RELDIR)/meicons_pm.tar.gz $@/icons
#
# Limited archive.
# 
$(EDIR)/archive:
	mkdir -p $@
	cp $(RELDIR)/mesrc.tar.gz $@
	cp $(RELDIR)/memacros.tar.gz $@
	cp $(RELDIR)/me.ehf.gz $@
	cp $(RELDIR)/meicons.tar.gz $@
#
# Any experimental.
# 
$(FDIR)/experimental:
	mkdir -p $@/bin
	cp $(EXPDIR)/medevbin_sparc_sol9.zip $@/bin
	cp $(EXPDIR)/medevbin_linux_24.tgz   $@/bin
	cp $(EXPDIR)/medevbin_win32.zip      $@/bin
	mkdir -p $@/src
	cp $(EXPDIR)/medevsrc.tar.gz         $@/src
	mkdir -p $@/macros
	cp $(EXPDIR)/medevmac.tar.gz         $@/macros
#
# User submissions.
#
$(FDIR)/submissions:
	mkdir -p $@
	cp $(SUBDIR)/*.txt $@
#
# CD Labels.
#
$(FDIR)/cdlabels $(EDIR)/cdlabels:
	mkdir -p $@
	cp a4_neto_0308.pdf $@
	cp a4_pressit_0308.pdf $@
	cp letter_neto_0308.pdf $@
	cp letter_pressit_0308.pdf $@
	cp cdback.png $@
	cp cdfull.png $@
	cp cdexec.png $@
	cp label0308.std $@
#
# Icons.
#
$(FDIR)/icons:
	mkdir -p $@/windows
	unzip -o $(RELDIR)/../release_020105/meicons.zip  -d $@/windows
	rm -rf $@/png
	(cd $@; gunzip -c $(RELDIR)/meicons-extra.tar.gz | tar xvf -)
	mv $@/icons $@/png
	(cd $@; gunzip -c $(RELDIR)/meicons.tar.gz | tar xvf -)
	mkdir -p $@/pm
	(cd $@/pm; gunzip -c $(RELDIR)/meicons_pm.tar.gz | tar xvf -)
	cp $(RELDIR)/../images/nlogo.png $@
	cp $(RELDIR)/../images/logo.bmp $@
	cp $(RELDIR)/../images/logo.png $@

jasspafull.iso: $(FDIRS)
	cd $(FDIR); $(MKISOFS) -o ../$@ -V $(FLABEL) $(FULLMKISOFSFLAGS) ./
jasspaexec.iso: $(EDIRS)
	cd $(EDIR); $(MKISOFS) -o ../$@ -V $(ELABEL) $(EXECMKISOFSFLAGS) ./

jasspafull.zip:	jasspafull.iso
	zip -9 $@ jasspafull.iso
jasspaexec.zip: jasspaexec.iso
	zip -9 $@ jasspaexec.iso
