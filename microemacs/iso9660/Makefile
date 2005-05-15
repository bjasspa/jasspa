##############################################################################
#
#			Copyright 2003-2004 Jon Green.
#			      All Rights Reserved
#
#
#  System        : MicroEmacs
#  Module        : CD-ROM image
#  Object Name   : $RCSfile: Makefile,v $
#  Revision      : $Revision: 1.10 $
#  Date          : $Date: 2005-05-15 00:20:26 $
#  Author        : $Author: jon $
#  Created By    : Jon Green
#  Created       : Sun Aug 10 11:57:07 2003
#  Last Modified : <050515.0102>
#
#  Description   : Construct the ISO CD-ROM images of MicroEmacs
#
#  Notes         : Runs under sunOS only.
#                  All images are taken from the www.jasspa.com site
#                  following a release.
#                  Uses "unix2dos" and "unix2mac" these are private
#	           tools.
#
#  History
#
##############################################################################
#
#  Copyright (c) 2003-2004 Jon Green.
#
#  All Rights Reserved.
#
#  This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
#  reproduced,  translated,  or  reduced to any  electronic  medium or machine
#  readable form without prior written consent from Jon Green.
#
##############################################################################

TOPDIR=..
include $(TOPDIR)/etc/makeinc
#
# Define the image source locations
#
BASEDIR=$(TOPDIR)/../jasspacom
CDVERSION=14
SPLDIR=$(BASEDIR)/spelling
RELDIR=$(BASEDIR)/release_$(MEVERS)
MISDIR=$(BASEDIR)/misc
SUBDIR=$(BASEDIR)/subs

FLABEL="jasspa$(MEYY)$(MEMM)f$(CDVERSION)"
ELABEL="jasspa$(MEYY)$(MEMM)e$(CDVERSION)"
LLABEL="jasspa$(MEYY)$(MEMM)x$(CDVERSION)"
FDISK=jasspa-mecd-full-$(MEVERS).iso
EDISK=jasspa-mecd-exec-$(MEVERS).iso
LDISK=jasspa-mecd-lexe-$(MEVERS).iso
FZIP =jasspa-mecd-full-$(MEVERS).zip
EZIP =jasspa-mecd-exec-$(MEVERS).zip
LZIP =jasspa-mecd-lexe-$(MEVERS).zip
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
RM	= 	rm -f
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
	$(RMDIR) ./$(FDIR)
	$(RMDIR) ./$(EDIR)
	rm -f *~
	rm -f make.err

spotless: clean
	rm -f $(FZIP) $(EZIP) $(LZIP)
	rm -f *.iso
#
# Root directory readme's
#
$(FDIR) $(EDIR):
	$(MKDIR) $@
	$(CP) $(RELDIR)/doc/faq.txt $@/faq.unix
	$(UNIX2DOS) $(RELDIR)/doc/faq.txt $@/faq.joliet
	$(UNIX2MAC) $(RELDIR)/doc/faq.txt $@/faq.hfs
	$(CP) $(RELDIR)/doc/license.txt $@/license.unix
	$(UNIX2DOS) $(RELDIR)/doc/license.txt $@/license.joliet
	$(UNIX2MAC) $(RELDIR)/doc/license.txt $@/license.hfs
	$(CP) $(RELDIR)/doc/COPYING $@/copying.unix
	$(UNIX2DOS) $(RELDIR)/doc/COPYING $@/copying.joliet
	$(UNIX2MAC) $(RELDIR)/doc/COPYING $@/copying.hfs
	$(CP) $(RELDIR)/doc/build.txt $@/build.unix
	$(UNIX2DOS) $(RELDIR)/doc/build.txt $@/build.joliet
	$(UNIX2MAC) $(RELDIR)/doc/build.txt $@/build.hfs
	$(CP) $(RELDIR)/doc/readme.txt $@/readme.unix
	$(UNIX2DOS) $(RELDIR)/doc/readme.txt $@/readme.joliet
	$(UNIX2MAC) $(RELDIR)/doc/readme.txt $@/readme.hfs
	$(CP) cdrom.txt $@/cdrom.unix
	$(UNIX2DOS) cdrom.txt $@/cdrom.joliet
	$(UNIX2MAC) cdrom.txt $@/cdrom.hfs
#
# Nano Emacs excecutable directory.
#
$(FDIR)/ne $(EDIR)/ne:	ne.txt
	$(MKDIR) $@
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-linux-i386-2.4-20050505.gz > $@/nelinux24
	chmod a+rx $@/nelinux24
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-linux-i386-2.6-20050505.gz > $@/nelinux26
	chmod a+rx $@/nelinux26
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-sun-i386-58-20050505.gz > $@/nesun86_58
	chmod a+rx $@/nesun86_58
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-sun-sparc-56-20050505.gz> $@/nesunos_56
	chmod a+rx $@/nesunos_56
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-sun-sparc-58-20050505.gz> $@/nesunos_58
	chmod a+rx $@/nesunos_58
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-sun-sparc-59-20050505.gz> $@/nesunos_59
	chmod a+rx $@/nesunos_59
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-sgi-irix-6-20050505.gz > $@/neirix6
	chmod a+rx $@/neirix6
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-hpux-pa-10.20-20050505.gz > $@/nehpux10
	chmod a+rx $@/nehpux10
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-hpux-pa-11.00-20050505.gz > $@/nehpux11
	chmod a+rx $@/nehpux11
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-ibm-aix-4-20050505.gz > $@/neaix4
	chmod a+rx $@/neaix4
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-ibm-aix-5-20050505.gz > $@/neaix5
	chmod a+rx $@/neaix5
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-cygwin-i386-20050505.gz > $@/necygwin.exe
	chmod a+rx $@/necygwin.exe
	$(UNZIP) $(RELDIR)/jasspa-ne-ms-win32-20050505.zip -d $@
	$(MV) $@/ne32.exe $@/nec32.exe
	$(UNZIP) $(RELDIR)/jasspa-ne-msdos-djgpp1-20050505.zip -d $@
	$(MV) $@/ne.exe $@/nedos.exe
	$(UNZIP) $(RELDIR)/jasspa-ne-msdos-djgpp2-20050505.zip -d $@
	$(MV) $@/ne.exe $@/nedosdj2.exe
	$(UNZIP) $(RELDIR)/jasspa-new-ms-win32-20050505.zip  -d $@
	$(CP) $(RELDIR)/ne.emf $@
	$(CP) ne.txt $@/readme.unix
	$(UNIX2DOS) ne.txt $@/readme.joliet
	$(UNIX2MAC) ne.txt $@/readme.hfs
#	$(MV) $@/new32.exe $@/new32.exe
#	$(GUNZIP) -c $(RELDIR)/jasspa-ne-openbsd-i386-3.4-20050505.gz > $@/neopenbsd34
#	chmod a+rx $@/neopenbsd34
#	$(GUNZIP) -c $(RELDIR)/jasspa-ne-apple-darwin-6.6-20050505.gz > $@/nedarwin
#	chmod a+rx $@/nedarwin
#	$(GUNZIP) -c $(RELDIR)/jasspa-ne-sun-i386-59-20050505.gz > $@/nesun86_59
#	chmod a+rx $@/nesun86_59

#
# Spelling dictionaries.
#
$(FDIR)/archive/spelling: spell.txt
	$(MKDIR) $@
	$(CP) $(SPLDIR)/ls_dede.tar.gz $(SPLDIR)/ls_dede.zip $@
	$(CP) $(SPLDIR)/ls_engb.tar.gz $(SPLDIR)/ls_engb.zip $@
	$(CP) $(SPLDIR)/ls_enus.tar.gz $(SPLDIR)/ls_enus.zip $@
	$(CP) $(SPLDIR)/ls_eses.tar.gz $(SPLDIR)/ls_eses.zip $@
	$(CP) $(SPLDIR)/ls_fifi.tar.gz $(SPLDIR)/ls_fifi.zip $@
	$(CP) $(SPLDIR)/ls_frfr.tar.gz $(SPLDIR)/ls_frfr.zip $@
	$(CP) $(SPLDIR)/ls_itit.tar.gz $(SPLDIR)/ls_itit.zip $@
	$(CP) $(SPLDIR)/ls_plpl.tar.gz $(SPLDIR)/ls_plpl.zip $@
	$(CP) $(SPLDIR)/ls_ptpt.tar.gz $(SPLDIR)/ls_ptpt.zip $@
	$(CP) $(SPLDIR)/ls_ruye.tar.gz $(SPLDIR)/ls_ruye.zip $@
	$(CP) $(SPLDIR)/ls_ruyo.tar.gz $(SPLDIR)/ls_ruyo.zip $@
	$(CP) spell.txt $@/readme.unix
	$(UNIX2DOS) spell.txt $@/readme.joliet
	$(UNIX2MAC) spell.txt $@/readme.hfs
#
# Documentation directories.
#
$(FDIR)/doc: doc.txt
	$(MKDIR) $@
	$(UNZIP) $(RELDIR)/jasspa-mehtm-20050505.zip -d $@
	$(UNZIP) $(RELDIR)/jasspa-mewinhlp-ms-win32-20050505.zip -d $@
	$(CP) $(MISDIR)/docmode.txt $@
	$(CP) doc.txt $@/readme.unix
	$(UNIX2DOS) doc.txt $@/readme.joliet
	$(UNIX2MAC) doc.txt $@/readme.hfs
#
# Packages.
#
$(FDIR)/packages:
	$(MKDIR) $@/windows
	$(UNZIP) $(RELDIR)/jasspa-mepkg-ms-win32-20050505.zip -d $@/windows
	$(MKDIR) $@/sunos
	$(UNZIP) $(RELDIR)/jasspa-mepkg-sun-sparc-59-20050505.zip -d $@/sunos
	$(MV) $@/sunos/jasspa-me $@/sunos/jasspa-me-sun-sparc-59-20050505.pkg
	$(UNZIP) $(RELDIR)/jasspa-mepkg-sun-sparc-58-20050505.zip -d $@/sunos
	$(MV) $@/sunos/jasspa-me $@/sunos/jasspa-me-sun-sparc-58-20050505.pkg
	$(UNZIP) $(RELDIR)/jasspa-mepkg-sun-i386-58-20050505.zip -d $@/sunos
	$(MV) $@/sunos/jasspa-me $@/sunos/jasspa-me-sun-i386-58-20050505.pkg
	$(MKDIR) $@/hpux
	$(GUNZIP) -c $(RELDIR)/jasspa-mepkg-hpux-pa-10.20-20050505.depot.gz > $@/hpux/jasspa-mepkg-hpux-pa-10.20-20050505.depot
	$(GUNZIP) -c $(RELDIR)/jasspa-mepkg-hpux-pa-11.00-20050505.depot.gz > $@/hpux/jasspa-mepkg-hpux-pa-11.00-20050505.depot
	$(MKDIR) $@/linux24
	$(MKDIR) $@/linux26
	$(CP) $(RELDIR)/jasspa-me-20050505-1.src.rpm $@/linux24
	$(CP) $(RELDIR)/jasspa-me-20050505-1.i386.rpm $@/linux24
	$(CP) $(RELDIR)/fedora/jasspa-me-20050505-1.src.rpm $@/linux26
	$(CP) $(RELDIR)/fedora/jasspa-me-20050505-1.i386.rpm $@/linux26
	$(MKDIR) $@/cygwin
	$(CP) $(RELDIR)/jasspa-20050505-src.tar.bz2 $@/cygwin
	$(CP) $(RELDIR)/jasspa-20050505.tar.bz2 $@/cygwin
	$(CP) $(RELDIR)/doc/cygwin.txt $@/cygwin
	$(CP) package.txt $@/readme.unix
	$(UNIX2DOS) package.txt $@/readme.joliet
	$(UNIX2MAC) package.txt $@/readme.hfs
# 	$(MKDIR) $@/sunos/i386_59
#	$(UNZIP) $(RELDIR)/jasspa-mepkg-sun-i386-59-20050505.zip -d $@/sunos/i386_59
#	$(MKDIR) $@/sunos/sparc6
#	$(UNZIP) $(RELDIR)/jasspa-mepkg-sun-sparc-56-20050505.zip -d $@/sunos/sparc6

#
# Source directory
#
$(FDIR)/src:
	$(MKDIR) $@
	$(GUNZIP) -c $(RELDIR)/jasspa-mesrc-20050505.tar.gz | ($(CD) $@; tar xf -)
#
# Executable direcotory.
#
$(FDIR)/me $(EDIR)/me: me.txt
	- $(RMDIR) $@
	$(MKDIR) $@
	$(GUNZIP) -c $(RELDIR)/jasspa-metree-20050505.tar.gz | ($(CD) $@; tar xf -)
	$(MV) $@/jasspa/* $@
	$(RMDIR) $@/jasspa
	$(GUNZIP) -c $(RELDIR)/jasspa-me-linux-i386-2.4-20050505.gz > $@/melinux24
	chmod a+rx $@/melinux24
	$(GUNZIP) -c $(RELDIR)/jasspa-me-linux-i386-2.6-20050505.gz > $@/melinux26
	chmod a+rx $@/melinux26
	$(GUNZIP) -c $(RELDIR)/jasspa-me-sun-sparc-56-20050505.gz > $@/mesunos56
	chmod a+rx $@/mesunos56
	$(GUNZIP) -c $(RELDIR)/jasspa-me-sun-sparc-58-20050505.gz > $@/mesunos58
	chmod a+rx $@/mesunos58
	$(GUNZIP) -c $(RELDIR)/jasspa-me-sun-sparc-59-20050505.gz > $@/mesunos59
	chmod a+rx $@/mesunos59
	$(GUNZIP) -c $(RELDIR)/jasspa-me-sun-i386-58-20050505.gz > $@/mesun86_58
	chmod a+rx $@/mesun86_58
	$(GUNZIP) -c $(RELDIR)/jasspa-me-sgi-irix-6-20050505.gz > $@/meirix6
	chmod a+rx $@/meirix6
	$(GUNZIP) -c $(RELDIR)/jasspa-me-hpux-pa-10.20-20050505.gz > $@/mehpux10
	chmod a+rx $@/mehpux10
	$(GUNZIP) -c $(RELDIR)/jasspa-me-hpux-pa-11.00-20050505.gz > $@/mehpux11
	chmod a+rx $@/mehpux11
	$(GUNZIP) -c $(RELDIR)/jasspa-me-ibm-aix-4-20050505.gz > $@/meaix4
	chmod a+rx $@/meaix4
	$(GUNZIP) -c $(RELDIR)/jasspa-me-ibm-aix-5-20050505.gz > $@/meaix5
	chmod a+rx $@/meaix5
	$(UNZIP) $(RELDIR)/jasspa-me-ms-win32-20050505.zip -d $@
	$(UNZIP) $(RELDIR)/jasspa-me-ms-win32s-20050505.zip -d $@
	$(UNZIP) $(RELDIR)/jasspa-mecw-ms-win32-20050505.zip -d $@
	$(CP) $(RELDIR)/meicons.exe $@
	$(UNZIP) $(RELDIR)/jasspa-me-msdos-djgpp1-20050505.zip -d $@
	$(MV) $@/me.exe $@/medos.exe
	$(UNZIP) $(RELDIR)/jasspa-me-msdos-djgpp2-20050505.zip -d $@
	$(MV) $@/me.exe $@/medosdj2.exe
	$(GUNZIP) -c $(RELDIR)/jasspa-me-cygwin-i386-20050505.gz > $@/mecygwin.exe
	chmod a+rx $@/mecygwin.exe
	$(GUNZIP) -c $(RELDIR)/jasspa-mex-cygwin-i386-20050505.gz > $@/mexcygwin.exe
	chmod a+rx $@/mexcygwin.exe
	$(CP) me.txt $@/readme.unix
	$(UNIX2DOS) me.txt $@/readme.joliet
	$(UNIX2MAC) me.txt $@/readme.hfs
#	$(GUNZIP) -c $(RELDIR)/jasspa-me-sun-i386-59-20050505.gz > $@/mesun86_59
#	chmod a+rx $@/mesun86_59
#	$(GUNZIP) -c $(RELDIR)/jasspa-me-openbsd-i386-3.4-20050505.gz > $@/meopenbsd34
#	chmod a+rx $@/meopenbsd34
#	$(GUNZIP) -c $(RELDIR)/jasspa-mec-apple-darwin-6.6-20050505.gz > $@/mecdarwin
#	chmod a+rx $@/mecdarwin
	
#
# Archive directories.
#
$(FDIR)/archive:
	$(MKDIR) $@
	$(CP) $(RELDIR)/jasspa-mesrc-20050505.tar.gz $@
	$(CP) $(RELDIR)/jasspa-mesrc-20050505.zip $@
	$(MKDIR) $@/bin
	$(CP) $(RELDIR)/jasspa-me-hpux-pa-10.20-20050505.gz $@/bin
	$(CP) $(RELDIR)/jasspa-me-hpux-pa-11.00-20050505.gz $@/bin
	$(CP) $(RELDIR)/jasspa-me-ibm-aix-4-20050505.gz $@/bin
	$(CP) $(RELDIR)/jasspa-me-ibm-aix-5-20050505.gz $@/bin
	$(CP) $(RELDIR)/jasspa-me-linux-i386-2.4-20050505.gz $@/bin
	$(CP) $(RELDIR)/jasspa-me-linux-i386-2.6-20050505.gz $@/bin
	$(CP) $(RELDIR)/jasspa-me-ms-win32-20050505.zip $@/bin
	$(CP) $(RELDIR)/jasspa-me-ms-win32s-20050505.zip $@/bin
	$(CP) $(RELDIR)/jasspa-me-msdos-djgpp1-20050505.zip $@/bin
	$(CP) $(RELDIR)/jasspa-me-msdos-djgpp2-20050505.zip $@/bin
	$(CP) $(RELDIR)/jasspa-me-sgi-irix-6-20050505.gz $@/bin
	$(CP) $(RELDIR)/jasspa-me-sun-i386-58-20050505.gz $@/bin
	$(CP) $(RELDIR)/jasspa-me-sun-sparc-56-20050505.gz $@/bin
	$(CP) $(RELDIR)/jasspa-me-sun-sparc-58-20050505.gz $@/bin
	$(CP) $(RELDIR)/jasspa-me-sun-sparc-59-20050505.gz $@/bin
	$(CP) $(RELDIR)/jasspa-mecw-ms-win32-20050505.zip $@/bin
	$(CP) $(RELDIR)/jasspa-ne-hpux-pa-10.20-20050505.gz $@/bin
	$(CP) $(RELDIR)/jasspa-ne-hpux-pa-11.00-20050505.gz $@/bin
	$(CP) $(RELDIR)/jasspa-ne-ibm-aix-4-20050505.gz $@/bin
	$(CP) $(RELDIR)/jasspa-ne-ibm-aix-5-20050505.gz $@/bin
	$(CP) $(RELDIR)/jasspa-ne-linux-i386-2.4-20050505.gz $@/bin
	$(CP) $(RELDIR)/jasspa-ne-linux-i386-2.6-20050505.gz $@/bin
	$(CP) $(RELDIR)/jasspa-ne-msdos-djgpp1-20050505.zip $@/bin
	$(CP) $(RELDIR)/jasspa-ne-msdos-djgpp2-20050505.zip $@/bin
	$(CP) $(RELDIR)/jasspa-ne-sgi-irix-6-20050505.gz $@/bin
	$(CP) $(RELDIR)/jasspa-ne-sun-i386-58-20050505.gz $@/bin
	$(CP) $(RELDIR)/jasspa-ne-sun-sparc-56-20050505.gz $@/bin
	$(CP) $(RELDIR)/jasspa-ne-sun-sparc-58-20050505.gz $@/bin
	$(CP) $(RELDIR)/jasspa-ne-sun-sparc-59-20050505.gz $@/bin
	$(CP) $(RELDIR)/jasspa-new-ms-win32-20050505.zip $@/bin
	$(CP) $(RELDIR)/jasspa-ne-ms-win32-20050505.zip $@/bin
	$(CP) $(RELDIR)/jasspa-me-cygwin-i386-20050505.gz $@/bin
	$(CP) $(RELDIR)/jasspa-mex-cygwin-i386-20050505.gz $@/bin
	$(CP) $(RELDIR)/jasspa-ne-cygwin-i386-20050505.gz $@/bin
	$(MKDIR) $@/macros
	$(CP) $(RELDIR)/jasspa-memacros-20050505.tar.gz $@/macros
	$(CP) $(RELDIR)/jasspa-memacros-20050505.zip $@/macros
	$(CP) $(RELDIR)/jasspa-metree-20050505.tar.gz $@
	$(CP) $(RELDIR)/jasspa-metree-20050505.zip $@
	$(MKDIR) $@/doc
	$(CP) $(RELDIR)/jasspa-mehtml-20050505.tar.gz $@/doc
	$(CP) $(RELDIR)/jasspa-mehtm-20050505.zip $@/doc
	$(CP) $(RELDIR)/jasspa-mewinhlp-ms-win32-20050505.zip $@/doc
	$(MKDIR) $@/icons
	$(CP) $(RELDIR)/meicons.tar.gz $@/icons
	$(CP) $(RELDIR)/meicons.exe $@/icons
	$(CP) $(RELDIR)/meicons.zip $@/icons
	$(CP) $(RELDIR)/readme.jasspa_gnome $@/icons
	$(CP) $(RELDIR)/jasspa_gnome.tar.zip $@/icons
	$(CP) $(RELDIR)/meicons-extra.tar.gz $@/icons
	$(CP) $(RELDIR)/meicons-extra.tar.zip $@/icons
	$(CP) $(RELDIR)/meicons_pm.tar.gz $@/icons
#	$(CP) $(RELDIR)/jasspa-ne-apple-darwin-6.6-20050505.gz $@/bin
#	$(CP) $(RELDIR)/jasspa-me-openbsd-i386-3.4-20050505.gz $@/bin
#	$(CP) $(RELDIR)/jasspa-me-sun-i386-59-20050505.gz $@/bin
#	$(CP) $(RELDIR)/jasspa-mec-apple-darwin-6.6-20050505.gz $@/bin
#	$(CP) $(RELDIR)/jasspa-ne-sun-i386-59-20050505.gz $@/bin
#	$(CP) $(RELDIR)/jasspa-ne-openbsd-i386-3.4-20050505.gz $@/bin

#
# Limited archive.
#
$(EDIR)/archive:
	$(MKDIR) $@
	$(CP) $(RELDIR)/jasspa-mesrc-20050505.tar.gz $@
	$(CP) $(RELDIR)/jasspa-metree-20050505.tar.gz $@
#
# User submissions.
#
$(FDIR)/subs $(EDIR)/subs:
	$(MKDIR) $@
	$(CP) $(SUBDIR)/*.txt $@
#
# CD Labels.
#
$(FDIR)/cdlabels $(EDIR)/cdlabels:
	$(MKDIR) $@
	$(CP) a4_neto.pdf $@
	$(CP) a4_pressit.pdf $@
	$(CP) letter_neto.pdf $@
	$(CP) letter_pressit.pdf $@
	$(CP) cdback.png $@
	$(CP) cdfull.png $@
	$(CP) cdexec.png $@
	$(CP) cdlabel.std $@
#
# Icons.
#
$(FDIR)/icons:
	$(MKDIR) $@/windows
	$(UNZIP) $(RELDIR)/meicons.zip  -d $@/windows
	$(RMDIR) $@/png
	$(GUNZIP) -c $(RELDIR)/meicons-extra.tar.gz | ($(CD) $@; tar xf -)
	$(MV) $@/icons $@/png
	$(GUNZIP) -c $(RELDIR)/meicons.tar.gz | ($(CD) $@; tar xf -)
	$(MKDIR) $@/pm
	$(GUNZIP) -c $(RELDIR)/meicons_pm.tar.gz | ($(CD) $@/pm; tar xf -)
	$(CP) $(RELDIR)/../images/nlogo.png $@
	$(CP) $(RELDIR)/../images/logo.bmp $@
	$(CP) $(RELDIR)/../images/logo.png $@

jasspa-mecd-full-20050505.iso: $(FDIRS)
	$(CD) $(FDIR); $(MKISOFS) -o ../$@ -V $(FLABEL) $(FULLMKISOFSFLAGS) ./
jasspa-mecd-exec-20050505.iso: $(EDIRS)
	$(CD) $(EDIR); $(MKISOFS) -o ../$@ -V $(ELABEL) $(EXECMKISOFSFLAGS) ./
jasspa-mecd-lexe-20050505.iso: $(EDIRS)
	$(CD) $(EDIR); $(MKISOFS) $(ISOEXTRA) -o ../$@ -V $(LLABEL) $(EXECMKISOFSFLAGS) ./

jasspa-mecd-full-20050505.zip:	jasspa-mecd-full-20050505.iso
	$(ZIP) $@ jasspa-mecd-full-20050505.iso
jasspa-mecd-exec-20050505.zip: jasspa-mecd-exec-20050505.iso
	$(ZIP) $@ jasspa-mecd-exec-20050505.iso
jasspa-mecd-lexe-20050505.zip: jasspa-mecd-lexe-20050505.iso
	$(ZIP) $@ jasspa-mecd-lexe-20050505.iso
