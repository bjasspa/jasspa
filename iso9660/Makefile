##############################################################################
#
#			Copyright 2003 Jon Green.
#			      All Rights Reserved
#
#
#  System        :
#  Module        :
#  Object Name   : $RCSfile: Makefile,v $
#  Revision      : $Revision: 1.6 $
#  Date          : $Date: 2004-03-27 21:20:47 $
#  Author        : $Author: jon $
#  Created By    : Jon Green
#  Created       : Sun Aug 10 11:57:07 2003
#  Last Modified : <040327.2119>
#
#  Description
#
#  Notes
#
#  History
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
BASEDIR=/home/jon/jasspacom
SPLDIR=$(BASEDIR)/spelling
RELDIR=$(BASEDIR)/release_040301
MISDIR=$(BASEDIR)/misc
SUBDIR=$(BASEDIR)/subs

FLABEL="jasspa0403f13"
ELABEL="jasspa0403e13"
LLABEL="jasspa0403x13"
FDISK=jasspa-mecd-full-20040301.iso
EDISK=jasspa-mecd-exec-20040301.iso
LDISK=jasspa-mecd-lexe-20040301.iso
FZIP =jasspa-mecd-full-20040301.zip
EZIP =jasspa-mecd-exec-20040301.zip
LZIP =jasspa-mecd-lexe-20040301.zip
FDIR = fcd
EDIR = ecd

FDIRS=	$(FDIR) \
	$(FDIR)/ne \
	$(FDIR)/archive \
	$(FDIR)/archive/spelling \
	$(FDIR)/doc \
	$(FDIR)/packages \
	$(FDIR)/src \
	$(FDIR)/me \
	$(FDIR)/archive \
	$(FDIR)/subs \
	$(FDIR)/cdlabels \
	$(FDIR)/icons

EDIRS=	$(EDIR) \
	$(EDIR)/ne \
	$(EDIR)/me \
	$(EDIR)/subs \
	$(EDIR)/cdlabels \
	$(EDIR)/archive	
#
# Basic tools
#
RM	= rm -f
UNIX2DOS=	/opt/jon/bin/unix2dos
UNIX2MAC=	/opt/jon/bin/unix2mac

#ISOEXTRA=	-allow-multidot -allow-lowercase -hide-joliet-trans-tbl -l
ISOEXTRA=	-U
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
		-hide me/readme.hfs -hide me/readme.joliet		\
		-hide-joliet me/readme.hfs -hide-joliet me/readme.unix \
		-hide-hfs me/readme.joliet -hide-hfs me/readme.unix	\
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
	        COPYING.TXT=copying.hfs					\
	        COPYING.TXT=copying.joliet				\
	        COPYING.TXT=copying.unix				\
	        CDROM.TXT=cdrom.hfs					\
	        CDROM.TXT=cdrom.joliet					\
	        CDROM.TXT=cdrom.unix					\
	        license.txt=license.hfs					\
	        license.txt=license.joliet				\
	        license.txt=license.unix				\
	        doc/readme.txt=doc/readme.hfs				\
	        doc/readme.txt=doc/readme.joliet			\
	        doc/readme.txt=doc/readme.unix				\
	        ne/readme.txt=ne/readme.hfs				\
	        ne/readme.txt=ne/readme.joliet				\
	        ne/readme.txt=ne/readme.unix				\
	        me/readme.txt=me/readme.hfs				\
	        me/readme.txt=me/readme.joliet				\
	        me/readme.txt=me/readme.unix				\
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
	        COPYING.TXT=copying.hfs					\
	        COPYING.TXT=copying.joliet				\
	        COPYING.TXT=copying.unix				\
	        CDROM.TXT=cdrom.hfs					\
	        CDROM.TXT=cdrom.joliet					\
	        CDROM.TXT=cdrom.unix					\
	        license.txt=license.hfs					\
	        license.txt=license.joliet				\
	        license.txt=license.unix				\
	        ne/readme.txt=ne/readme.hfs				\
	        ne/readme.txt=ne/readme.joliet			\
	        ne/readme.txt=ne/readme.unix				\

all:	$(FZIP) $(EZIP) $(LZIP)

clean:
	rm -rf ./$(FDIR)
	rm -rf ./$(EDIR)
	rm -f *~
	rm -f make.err

spotless: clean
	rm -f $(FZIP) $(EZIP) $(LZIP)
	rm -f *.iso
#
# Root directory readme's
# 
$(FDIR) $(EDIR):
	mkdir -p $@
	cp $(RELDIR)/doc/faq.txt $@/faq.unix
	$(UNIX2DOS) $(RELDIR)/doc/faq.txt $@/faq.joliet
	$(UNIX2MAC) $(RELDIR)/doc/faq.txt $@/faq.hfs
	cp $(RELDIR)/doc/license.txt $@/license.unix
	$(UNIX2DOS) $(RELDIR)/doc/license.txt $@/license.joliet
	$(UNIX2MAC) $(RELDIR)/doc/license.txt $@/license.hfs
	cp $(RELDIR)/doc/COPYING $@/copying.unix
	$(UNIX2DOS) $(RELDIR)/doc/COPYING $@/copying.joliet
	$(UNIX2MAC) $(RELDIR)/doc/COPYING $@/copying.hfs
	cp $(RELDIR)/doc/build.txt $@/build.unix
	$(UNIX2DOS) $(RELDIR)/doc/build.txt $@/build.joliet
	$(UNIX2MAC) $(RELDIR)/doc/build.txt $@/build.hfs
	cp $(RELDIR)/doc/readme.txt $@/readme.unix
	$(UNIX2DOS) $(RELDIR)/doc/readme.txt $@/readme.joliet
	$(UNIX2MAC) $(RELDIR)/doc/readme.txt $@/readme.hfs
	cp cdrom.txt $@/cdrom.unix
	$(UNIX2DOS) cdrom.txt $@/cdrom.joliet
	$(UNIX2MAC) cdrom.txt $@/cdrom.hfs
#
# Nano Emacs excecutable directory.
# 
$(FDIR)/ne $(EDIR)/ne:	ne.txt
	mkdir -p $@
	gunzip -c $(RELDIR)/jasspa-ne-linux-i386-2.4-20040301.gz > $@/nelinux24
	chmod a+rx $@/nelinux24
	gunzip -c $(RELDIR)/jasspa-ne-sun-i386-58-20040301.gz > $@/nesun86_58
	chmod a+rx $@/nesun86_58
	gunzip -c $(RELDIR)/jasspa-ne-sun-i386-59-20040301.gz > $@/nesun86_59
	chmod a+rx $@/nesun86_59
	gunzip -c $(RELDIR)/jasspa-ne-sun-sparc-56-20040301.gz> $@/nesunos_56
	chmod a+rx $@/nesunos_56
	gunzip -c $(RELDIR)/jasspa-ne-sun-sparc-58-20040301.gz> $@/nesunos_58
	chmod a+rx $@/nesunos_58
	gunzip -c $(RELDIR)/jasspa-ne-sun-sparc-59-20040301.gz> $@/nesunos_59
	chmod a+rx $@/nesunos_59
	gunzip -c $(RELDIR)/jasspa-ne-sgi-irix-6-20040301.gz > $@/neirix6
	chmod a+rx $@/neirix6
	gunzip -c $(RELDIR)/jasspa-ne-hpux-pa-10.20-20040301.gz > $@/nehpux10
	chmod a+rx $@/nehpux10
	gunzip -c $(RELDIR)/jasspa-ne-hpux-pa-11.00-20040301.gz > $@/nehpux11
	chmod a+rx $@/nehpux11
	gunzip -c $(RELDIR)/jasspa-ne-ibm-aix-4-20040301.gz > $@/neaix4
	chmod a+rx $@/neaix4
	gunzip -c $(RELDIR)/jasspa-ne-openbsd-i386-3.4-20040301.gz > $@/neopenbsd34
	chmod a+rx $@/neopenbsd34
	gunzip -c $(RELDIR)/jasspa-ne-cygwin-20040301.exe.gz > $@/necygwin.exe
	chmod a+rx $@/necygwin.exe
	gunzip -c $(RELDIR)/jasspa-ne-apple-darwin-6.6-20040301.gz > $@/nedarwin
	chmod a+rx $@/nedarwin
	cp $(RELDIR)/jasspa-ne-ms-win32-20040301.exe $@/ne32.exe
	unzip -o $(RELDIR)/jasspa-ne-msdos-djgpp1-20040301.zip -d $@
	mv $@/ne.exe $@/nedos.exe
	cp $(RELDIR)/jasspa-ne-msdos-djgpp2-20040301.exe $@/nedosdj2.exe
	cp $(RELDIR)/jasspa-new-ms-win32-20040301.exe $@/new32.exe				
	cp $(RELDIR)/ne.emf $@
	cp ne.txt $@/readme.unix
	$(UNIX2DOS) ne.txt $@/readme.joliet
	$(UNIX2MAC) ne.txt $@/readme.hfs
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
	$(UNIX2DOS) spell.txt $@/readme.joliet
	$(UNIX2MAC) spell.txt $@/readme.hfs
#
# Documentation directories.
#
$(FDIR)/doc: doc.txt
	mkdir -p $@
	unzip -o $(RELDIR)/jasspa-mehtm-20040301.zip -d $@
	unzip -o $(RELDIR)/jasspa-mewinhlp-20040301.zip -d $@
	cp $(MISDIR)/docmode.txt $@
	cp doc.txt $@/readme.unix
	$(UNIX2DOS) doc.txt $@/readme.joliet
	$(UNIX2MAC) doc.txt $@/readme.hfs
#
# Packages.
# 
$(FDIR)/packages:
	mkdir -p $@/windows
	unzip -o $(RELDIR)/jasspa-mepkg-ms-win32-2004030.zip -d $@/windows
	mkdir -p $@/sunos/sparc9
	unzip -o $(RELDIR)/jasspa-mepkg-sun-sparc-59-20040301.zip -d $@/sunos/sparc9
	mkdir -p $@/sunos/sparc8
	unzip -o $(RELDIR)/jasspa-mepkg-sun-sparc-58-20040301.zip -d $@/sunos/sparc8
	mkdir -p $@/sunos/sparc6
	unzip -o $(RELDIR)/jasspa-mepkg-sun-sparc-56-20040301.zip -d $@/sunos/sparc6
	mkdir -p $@/sunos/intel8
	unzip -o $(RELDIR)/jasspa-mepkg-sun-i386-58-20040301.zip -d $@/sunos/intel8
	mkdir -p $@/sunos/intel9
	unzip -o $(RELDIR)/jasspa-mepkg-sun-i386-59-20040301.zip -d $@/sunos/intel9
	mkdir -p $@/hpux
	gunzip -c $(RELDIR)/jasspa-mepkg-hpux-pa-10.20-20040301.depot.gz > $@/hpux/jasspa-mepkg-hpux-pa-10.20-20040301.depot
	gunzip -c $(RELDIR)/jasspa-mepkg-hpux-pa-11.00-20040301.depot.gz > $@/hpux/jasspa-mepkg-hpux-pa-11.00-20040301.depot
	mkdir -p $@/linux
	cp $(RELDIR)/jasspa-me-20040301-1.src.rpm $@/linux
	cp $(RELDIR)/jasspa-me-20040301-1.i386.rpm $@/linux
	cp package.txt $@/readme.unix
	$(UNIX2DOS) package.txt $@/readme.joliet
	$(UNIX2MAC) package.txt $@/readme.hfs
#
# Source directory
#
$(FDIR)/src:
	mkdir -p $@
	(cd $@; gunzip -c $(RELDIR)/jasspa-mesrc-20040301.tar.gz | tar xvf -)
#
# Executable direcotory.
# 
$(FDIR)/me $(EDIR)/me: me.txt
	- rm -rf $@
	mkdir $@
	(cd $@; gunzip -c $(RELDIR)/jasspa-metree-20040301.tar.gz | tar xvf -)
	mv $@/jasspa/* $@	
	rmdir $@/jasspa
	gunzip -c $(RELDIR)/jasspa-me-linux-i386-2.4-20040301.gz > $@/melinux24
	chmod a+rx $@/melinux24
	gunzip -c $(RELDIR)/jasspa-me-sun-sparc-56-20040301.gz > $@/mesunos56
	chmod a+rx $@/mesunos56
	gunzip -c $(RELDIR)/jasspa-me-sun-sparc-58-20040301.gz > $@/mesunos58
	chmod a+rx $@/mesunos58
	gunzip -c $(RELDIR)/jasspa-me-sun-sparc-59-20040301.gz > $@/mesunos59
	chmod a+rx $@/mesunos59
	gunzip -c $(RELDIR)/jasspa-me-sun-i386-58-20040301.gz > $@/mesun86_58
	chmod a+rx $@/mesun86_58
	gunzip -c $(RELDIR)/jasspa-me-sun-i386-59-20040301.gz > $@/mesun86_59
	chmod a+rx $@/mesun86_59
	gunzip -c $(RELDIR)/jasspa-me-sgi-irix-6-20040301.gz > $@/meirix6
	chmod a+rx $@/meirix6
	gunzip -c $(RELDIR)/jasspa-me-hpux-pa-10.20-20040301.gz > $@/mehpux10
	chmod a+rx $@/mehpux10
	gunzip -c $(RELDIR)/jasspa-me-hpux-pa-11.00-20040301.gz > $@/mehpux11
	chmod a+rx $@/mehpux11
	gunzip -c $(RELDIR)/jasspa-me-ibm-aix-4-20040301.gz > $@/meaix4
	chmod a+rx $@/meaix4
	gunzip -c $(RELDIR)/jasspa-me-openbsd-i386-3.4-20040301.gz > $@/meopenbsd34
	chmod a+rx $@/meopenbsd34
	gunzip -c $(RELDIR)/jasspa-me-cygwin-20040301.exe.gz > $@/mecygwin.exe
	chmod a+rx $@/mecygwin.exe
	gunzip -c $(RELDIR)/jasspa-mec-cygwin-20040301.exe.gz > $@/meccygwin.exe
	chmod a+rx $@/meccygwin.exe
	gunzip -c $(RELDIR)/jasspa-mec-apple-darwin-6.6-20040301.gz > $@/mecdarwin
	chmod a+rx $@/mecdarwin
	unzip -o $(RELDIR)/jasspa-me-ms-win32-20040301.zip -d $@
	unzip -o $(RELDIR)/jasspa-me-ms-win32s-20040301.zip -d $@
	unzip -o $(RELDIR)/jasspa-meu-ms-win32-20040301.zip -d $@
	unzip -o $(RELDIR)/jasspa-meucw-ms-win32-20040301.zip -d $@
	cp $(RELDIR)/meicons.exe $@
	unzip -o $(RELDIR)/jasspa-me-msdos-djgpp1-20040301.zip -d $@
	mv $@/me.exe $@/medos.exe
	unzip -o $(RELDIR)/jasspa-me-msdos-djgpp2-20040301.zip -d $@
	mv $@/me.exe $@/medosdj2.exe
	cp me.txt $@/readme.unix
	$(UNIX2DOS) me.txt $@/readme.joliet
	$(UNIX2MAC) me.txt $@/readme.hfs
#
# Archive directories.
#
$(FDIR)/archive:
	mkdir -p $@
	cp $(RELDIR)/jasspa-mesrc-20040301.tar.gz $@
	cp $(RELDIR)/jasspa-mesrc-20040301.zip $@
	cp $(RELDIR)/jasspa-metree-20040301.tar.gz $@
	cp $(RELDIR)/jasspa-metree-20040301.zip $@
	mkdir -p $@/bin
	cp $(RELDIR)/jasspa-me-cygwin-20040301.exe.gz $@/bin
	cp $(RELDIR)/jasspa-me-hpux-pa-10.20-20040301.gz $@/bin
	cp $(RELDIR)/jasspa-me-hpux-pa-11.00-20040301.gz $@/bin
	cp $(RELDIR)/jasspa-me-ibm-aix-4-20040301.gz $@/bin
	cp $(RELDIR)/jasspa-me-linux-i386-2.4-20040301.gz $@/bin
	cp $(RELDIR)/jasspa-me-ms-win32-20040301.zip $@/bin
	cp $(RELDIR)/jasspa-me-ms-win32s-20040301.zip $@/bin
	cp $(RELDIR)/jasspa-me-msdos-djgpp1-20040301.zip $@/bin
	cp $(RELDIR)/jasspa-me-msdos-djgpp2-20040301.zip $@/bin
	cp $(RELDIR)/jasspa-me-openbsd-i386-3.4-20040301.gz $@/bin
	cp $(RELDIR)/jasspa-me-sgi-irix-6-20040301.gz $@/bin
	cp $(RELDIR)/jasspa-me-sun-i386-58-20040301.gz $@/bin
	cp $(RELDIR)/jasspa-me-sun-i386-59-20040301.gz $@/bin
	cp $(RELDIR)/jasspa-me-sun-sparc-56-20040301.gz $@/bin
	cp $(RELDIR)/jasspa-me-sun-sparc-58-20040301.gz $@/bin
	cp $(RELDIR)/jasspa-me-sun-sparc-59-20040301.gz $@/bin
	cp $(RELDIR)/jasspa-mec-apple-darwin-6.6-20040301.gz $@/bin
	cp $(RELDIR)/jasspa-mec-cygwin-20040301.exe.gz $@/bin
	cp $(RELDIR)/jasspa-meu-ms-win32-20040301.zip $@/bin
	cp $(RELDIR)/jasspa-meucw-ms-win32-20040301.zip $@/bin
	cp $(RELDIR)/jasspa-ne-apple-darwin-6.6-20040301.gz $@/bin
	cp $(RELDIR)/jasspa-ne-cygwin-20040301.exe.gz $@/bin
	cp $(RELDIR)/jasspa-ne-hpux-pa-10.20-20040301.gz $@/bin
	cp $(RELDIR)/jasspa-ne-hpux-pa-11.00-20040301.gz $@/bin
	cp $(RELDIR)/jasspa-ne-ibm-aix-4-20040301.gz $@/bin
	cp $(RELDIR)/jasspa-ne-linux-i386-2.4-20040301.gz $@/bin
	cp $(RELDIR)/jasspa-ne-ms-win32-20040301.exe $@/bin
	cp $(RELDIR)/jasspa-ne-msdos-djgpp1-20040301.zip $@/bin
	cp $(RELDIR)/jasspa-ne-msdos-djgpp2-20040301.exe $@/bin
	cp $(RELDIR)/jasspa-ne-msdos-djgpp2-20040301.zip $@/bin
	cp $(RELDIR)/jasspa-ne-openbsd-i386-3.4-20040301.gz $@/bin
	cp $(RELDIR)/jasspa-ne-sgi-irix-6-20040301.gz $@/bin
	cp $(RELDIR)/jasspa-ne-sun-i386-58-20040301.gz $@/bin
	cp $(RELDIR)/jasspa-ne-sun-i386-59-20040301.gz $@/bin
	cp $(RELDIR)/jasspa-ne-sun-sparc-56-20040301.gz $@/bin
	cp $(RELDIR)/jasspa-ne-sun-sparc-58-20040301.gz $@/bin
	cp $(RELDIR)/jasspa-ne-sun-sparc-59-20040301.gz $@/bin
	cp $(RELDIR)/jasspa-new-ms-win32-20040301.exe $@/bin
	mkdir -p $@/macros
	cp $(RELDIR)/jasspa-memacros-20040301.tar.gz $@/macros
	cp $(RELDIR)/jasspa-memacros-20040301.zip $@/macros
	mkdir -p $@/doc
	cp $(RELDIR)/jasspa-mehtml-20040301.tar.gz $@/doc
	cp $(RELDIR)/jasspa-mehtm-20040301.zip $@/doc
	cp $(RELDIR)/jasspa-mewinhlp-20040301.zip $@/doc
	mkdir -p $@/icons
	cp $(RELDIR)/meicons.tar.gz $@/icons
	cp $(RELDIR)/meicons.exe $@/icons
	cp $(RELDIR)/meicons.zip $@/icons
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
	cp $(RELDIR)/jasspa-mesrc-20040301.tar.gz $@
	cp $(RELDIR)/jasspa-metree-20040301.tar.gz $@
#
# User submissions.
#
$(FDIR)/subs $(EDIR)/subs:
	mkdir -p $@
	cp $(SUBDIR)/*.txt $@
#
# CD Labels.
#
$(FDIR)/cdlabels $(EDIR)/cdlabels:
	mkdir -p $@
	cp a4_neto.pdf $@
	cp a4_pressit.pdf $@
	cp letter_neto.pdf $@
	cp letter_pressit.pdf $@
	cp cdback.png $@
	cp cdfull.png $@
	cp cdexec.png $@
	cp cdlabel.std $@
#
# Icons.
#
$(FDIR)/icons:
	mkdir -p $@/windows
	unzip -o $(RELDIR)/meicons.zip  -d $@/windows
	rm -rf $@/png
	(cd $@; gunzip -c $(RELDIR)/meicons-extra.tar.gz | tar xvf -)
	mv $@/icons $@/png
	(cd $@; gunzip -c $(RELDIR)/meicons.tar.gz | tar xvf -)
	mkdir -p $@/pm
	(cd $@/pm; gunzip -c $(RELDIR)/meicons_pm.tar.gz | tar xvf -)
	cp $(RELDIR)/../images/nlogo.png $@
	cp $(RELDIR)/../images/logo.bmp $@
	cp $(RELDIR)/../images/logo.png $@

jasspa-mecd-full-20040301.iso: $(FDIRS)
	cd $(FDIR); $(MKISOFS) -o ../$@ -V $(FLABEL) $(FULLMKISOFSFLAGS) ./
jasspa-mecd-exec-20040301.iso: $(EDIRS)
	cd $(EDIR); $(MKISOFS) -o ../$@ -V $(ELABEL) $(EXECMKISOFSFLAGS) ./
jasspa-mecd-lexe-20040301.iso: $(EDIRS)
	cd $(EDIR); $(MKISOFS) $(ISOEXTRA) -o ../$@ -V $(LLABEL) $(EXECMKISOFSFLAGS) ./

jasspa-mecd-full-20040301.zip:	jasspa-mecd-full-20040301.iso
	zip -9 $@ jasspa-mecd-full-20040301.iso
jasspa-mecd-exec-20040301.zip: jasspa-mecd-exec-20040301.iso
	zip -9 $@ jasspa-mecd-exec-20040301.iso
jasspa-mecd-lexe-20040301.zip: jasspa-mecd-lexe-20040301.iso
	zip -9 $@ jasspa-mecd-lexe-20040301.iso
