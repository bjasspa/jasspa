##############################################################################
#
#			Copyright (C) 2003-2006 Jon Green.
#			      All Rights Reserved
#
#
#  System        : MicroEmacs
#  Module        : CD-ROM image
#  Object Name   : $RCSfile: Makefile,v $
#  Revision      : $Revision: 1.15 $
#  Date          : $Date: 2006-10-14 17:52:42 $
#  Author        : $Author: jon $
#  Created By    : Jon Green
#  Created       : Sun Aug 10 11:57:07 2003
#  Last Modified : <061014.1738>
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
#  Copyright (C) 2003-2005 Jon Green.
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
# Override
MEYY=06
MEMM=09
MEDD=09
MEVERS=20$(MEYY)$(MEMM)$(MEDD)
# Patched versions
PMEYY=06
PMEMM=10
PMEDD=13
PMEVERS=20$(PMEYY)$(PMEMM)$(PMEDD)
# 
ZCAT = zcat
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
	$(FDIR)/icons \
	$(FDIR)/man/cat1

EDIRS=	$(EDIR) \
	$(EDIR)/ne \
	$(EDIR)/me \
	$(EDIR)/subs \
	$(EDIR)/cdlabels \
	$(EDIR)/archive \
	$(EDIR)/man/cat1
#
# Basic tools
#
RM	=	rm -f
UNIX2DOS=	/opt/jon/bin/unix2dos
UNIX2MAC=	/opt/jon/bin/unix2mac

#ISOEXTRA=	-allow-multidot -allow-lowercase -hide-joliet-trans-tbl -l
ISOEXTRA=	-U
MKISOFS=	mkisofs -r -J -hfs
FULLMKISOFSFLAGS=-graft-points						\
		-hide readme.hfs -hide readme.joliet			\
		-hide-joliet readme.hfs -hide-joliet readme.unix	\
		-hide-hfs readme.joliet -hide-hfs readme.unix		\
		-hide faq.hfs -hide faq.joliet				\
		-hide-joliet faq.hfs -hide-joliet faq.unix		\
		-hide-hfs faq.joliet -hide-hfs faq.unix			\
		-hide build.hfs -hide build.joliet			\
		-hide-joliet build.hfs -hide-joliet build.unix		\
		-hide-hfs build.joliet -hide-hfs build.unix		\
		-hide copying.hfs -hide copying.joliet			\
		-hide-joliet copying.hfs -hide-joliet copying.unix	\
		-hide-hfs copying.joliet -hide-hfs copying.unix		\
		-hide cdrom.hfs -hide cdrom.joliet			\
		-hide-joliet cdrom.hfs -hide-joliet cdrom.unix		\
		-hide-hfs cdrom.joliet -hide-hfs cdrom.unix		\
		-hide license.hfs -hide license.joliet			\
		-hide-joliet license.hfs -hide-joliet license.unix 	\
		-hide-hfs license.joliet -hide-hfs license.unix		\
		-hide doc/readme.hfs -hide doc/readme.joliet		\
		-hide-joliet doc/readme.hfs -hide-joliet doc/readme.unix \
		-hide-hfs doc/readme.joliet -hide-hfs doc/readme.unix	\
		-hide ne/readme.hfs -hide ne/readme.joliet		\
		-hide-joliet ne/readme.hfs -hide-joliet ne/readme.unix	\
		-hide-hfs ne/readme.joliet -hide-hfs ne/readme.unix	\
		-hide me/readme.hfs -hide me/readme.joliet		\
		-hide-joliet me/readme.hfs -hide-joliet me/readme.unix	\
		-hide-hfs me/readme.joliet -hide-hfs me/readme.unix	\
		-hide archive/spelling/readme.hfs -hide archive/spelling/readme.joliet		\
		-hide-joliet archive/spelling/readme.hfs -hide-joliet archive/spelling/readme.unix \
		-hide-hfs archive/spelling/readme.joliet -hide-hfs archive/spelling/readme.unix	\
		-hide packages/readme.hfs -hide packages/readme.joliet \
		-hide-joliet packages/readme.hfs -hide-joliet packages/readme.unix \
		-hide-hfs packages/readme.joliet -hide-hfs packages/readme.unix	\
		-hide change.hfs -hide change.joliet			\
		-hide-joliet change.hfs -hide-joliet change.unix	\
		-hide-hfs change.joliet -hide-hfs change.unix		\
	        readme.txt=readme.hfs					\
	        readme.txt=readme.joliet				\
	        readme.txt=readme.unix					\
	        faq.txt=faq.hfs						\
	        faq.txt=faq.joliet					\
	        faq.txt=faq.unix					\
	        change.log=change.hfs					\
	        change.log=change.joliet				\
	        change.log=change.unix					\
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
	        packages/readme.txt=packages/readme.hfs 		\
	        packages/readme.txt=packages/readme.joliet 		\
	        packages/readme.txt=packages/readme.unix 		\

EXECMKISOFSFLAGS=-graft-points						\
		-hide readme.hfs -hide readme.joliet			\
		-hide-joliet readme.hfs -hide-joliet readme.unix	\
		-hide-hfs readme.joliet -hide-hfs readme.unix		\
		-hide faq.hfs -hide faq.joliet				\
		-hide-joliet faq.hfs -hide-joliet faq.unix		\
		-hide-hfs faq.joliet -hide-hfs faq.unix			\
		-hide build.hfs -hide build.joliet			\
		-hide-joliet build.hfs -hide-joliet build.unix		\
		-hide-hfs build.joliet -hide-hfs build.unix		\
		-hide copying.hfs -hide copying.joliet			\
		-hide-joliet copying.hfs -hide-joliet copying.unix	\
		-hide-hfs copying.joliet -hide-hfs copying.unix		\
		-hide cdrom.hfs -hide cdrom.joliet			\
		-hide-joliet cdrom.hfs -hide-joliet cdrom.unix		\
		-hide-hfs cdrom.joliet -hide-hfs cdrom.unix		\
		-hide license.hfs -hide license.joliet			\
		-hide-joliet license.hfs -hide-joliet license.unix	\
		-hide-hfs license.joliet -hide-hfs license.unix		\
		-hide ne/readme.hfs -hide ne/readme.joliet		\
		-hide-joliet ne/readme.hfs -hide-joliet ne/readme.unix	\
		-hide-hfs ne/readme.joliet -hide-hfs ne/readme.unix	\
		-hide change.hfs -hide change.joliet			\
		-hide-joliet change.hfs -hide-joliet change.unix	\
		-hide-hfs change.joliet -hide-hfs change.unix		\
	        readme.txt=readme.hfs					\
	        readme.txt=readme.joliet				\
	        readme.txt=readme.unix					\
	        faq.txt=faq.hfs						\
	        faq.txt=faq.joliet					\
	        faq.txt=faq.unix					\
	        change.log=change.hfs					\
	        change.log=change.joliet				\
	        change.log=change.unix					\
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
	        ne/readme.txt=ne/readme.joliet				\
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
	$(CP) $(RELDIR)/doc/jasspame.pdf $@
	$(CP) cdrom.txt $@/cdrom.unix
	$(UNIX2DOS) cdrom.txt $@/cdrom.joliet
	$(UNIX2MAC) cdrom.txt $@/cdrom.hfs
	$(CP) $(RELDIR)/doc/change.log $@/change.unix
	$(UNIX2DOS) $(RELDIR)/doc/change.log $@/change.joliet
	$(UNIX2MAC) $(RELDIR)/doc/change.log $@/change.hfs
#
# Nano Emacs excecutable directory.
#
$(FDIR)/ne $(EDIR)/ne:	ne.txt
	$(MKDIR) $@
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-linux-i386-2.2-$(MEVERS).gz > $@/nelinux22
	chmod a+rx $@/nelinux22
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-linux-i386-2.4-$(MEVERS).gz > $@/nelinux24
	chmod a+rx $@/nelinux24
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-linux-curses-i386-2.4-$(MEVERS).gz > $@/nelinux24c
	chmod a+rx $@/nelinux24c
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-linux-i386-2.6-$(MEVERS).gz > $@/nelinux26
	chmod a+rx $@/nelinux26
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-sun-i386-58-$(MEVERS).gz > $@/nesun86_58
	chmod a+rx $@/nesun86_58
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-sun-i386-510-$(MEVERS).gz > $@/nesun86_510
	chmod a+rx $@/nesun86_510
	$(ZCAT) $(RELDIR)/jasspa-ne-sun-sparc-56-$(MEVERS).Z > $@/nesunos56
	chmod a+rx $@/nesunos56
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-sun-sparc-58-$(MEVERS).gz> $@/nesunos58
	chmod a+rx $@/nesunos58
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-sun-sparc-510-$(MEVERS).gz> $@/nesunos510
	chmod a+rx $@/nesunos510
	$(ZCAT) $(RELDIR)/jasspa-ne-sgi-irix-6-$(MEVERS).Z > $@/neirix6
	chmod a+rx $@/neirix6
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-hpux-pa-10.20-$(MEVERS).gz > $@/nehpux10
	chmod a+rx $@/nehpux10
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-hpux-pa-11.00-$(MEVERS).gz > $@/nehpux11
	chmod a+rx $@/nehpux11
	$(ZCAT) $(RELDIR)/jasspa-ne-ibm-aix-4-$(MEVERS).Z > $@/neaix4
	chmod a+rx $@/neaix4
	$(ZCAT) $(RELDIR)/jasspa-ne-ibm-aix-5-$(MEVERS).Z > $@/neaix5
	chmod a+rx $@/neaix5
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-apple-ppc-10.3-$(MEVERS).gz > $@/nemac10_3
	chmod a+rx $@/nemac10_3
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-openbsd-i386-3.4-$(MEVERS).gz > $@/neopenbsd34
	chmod a+rx $@/neopenbsd34
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-freebsd-i386-5.2-$(MEVERS).gz > $@/nefreebsd52
	chmod a+rx $@/nefreebsd52
	$(GUNZIP) -c $(RELDIR)/jasspa-ne-cygwin-i386-$(PMEVERS).gz > $@/necygwin.exe
	chmod a+rx $@/necygwin.exe
	$(UNZIP) $(RELDIR)/jasspa-ne-ms-win32-$(MEVERS).zip -d $@
	$(UNZIP) $(RELDIR)/jasspa-nec-ms-win32s-$(MEVERS).zip -d $@
	$(MV) $@/nec32s.exe $@/ne32s.exe
	$(UNZIP) $(RELDIR)/jasspa-ne-ms-dos-$(MEVERS).zip -d $@
	$(MV) $@/ne.exe $@/nedos.exe
	$(UNZIP) $(RELDIR)/jasspa-ne-ms-win32-$(MEVERS).zip -d $@
	$(UNZIP) $(RELDIR)/jasspa-new-ms-win32-$(MEVERS).zip -d $@
	$(UNZIP) $(RELDIR)/jasspa-nec-ms-win32s-$(MEVERS).zip -d $@
	$(MV) $@/nec32s.exe $@/ne32s.exe
	$(UNZIP) $(RELDIR)/jasspa-new-ms-win32s-$(MEVERS).zip -d $@
	$(CP) $(RELDIR)/ne.emf $@
	$(CP) ne.txt $@/readme.unix
	$(UNIX2DOS) ne.txt $@/readme.joliet
	$(UNIX2MAC) ne.txt $@/readme.hfs
#	$(UNZIP) $(RELDIR)/jasspa-ne-msdos-djgpp2-$(MEVERS).zip -d $@
#	$(MV) $@/ne.exe $@/nedosdj2.exe

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
	$(UNZIP) $(RELDIR)/jasspa-mehtm-$(MEVERS).zip -d $@
	$(UNZIP) $(RELDIR)/jasspa-mewinhlp-ms-win32-$(MEVERS).zip -d $@
	$(CP) $(MISDIR)/docmode.txt $@
	$(CP) $(RELDIR)/doc/jasspame.pdf $@
	$(CP) doc.txt $@/readme.unix
	$(UNIX2DOS) doc.txt $@/readme.joliet
	$(UNIX2MAC) doc.txt $@/readme.hfs
#
# Packages.
#
$(FDIR)/packages:
	$(MKDIR) $@/windows
	$(UNZIP) $(RELDIR)/jasspa-mepkg-ms-win32-$(MEVERS).zip -d $@/windows
	$(MKDIR) $@/sunos
	$(GUNZIP) -c $(RELDIR)/jasspa-mepkg-sun-sparc-5.8-$(MEVERS).pkg.gz > $@/sunos/jasspa-mepkg-sun-sparc-5.8-$(MEVERS).pkg
	$(GUNZIP) -c $(RELDIR)/jasspa-mepkg-sun-sparc-5.10-$(MEVERS).pkg.gz > $@/sunos/jasspa-mepkg-sun-sparc-5.10-$(MEVERS).pkg
	$(GUNZIP) -c $(RELDIR)/jasspa-mepkg-sun-i386-5.8-$(MEVERS).pkg.gz > $@/sunos/jasspa-mepkg-sun-i386-5.8-$(MEVERS).pkg
	$(GUNZIP) -c $(RELDIR)/jasspa-mepkg-sun-i386-5.10-$(MEVERS).pkg.gz > $@/sunos/jasspa-mepkg-sun-i386-5.10-$(MEVERS).pkg
	$(MKDIR) $@/hpux
	$(GUNZIP) -c $(RELDIR)/jasspa-mepkg-hpux-pa-10.20-$(MEVERS).depot.gz > $@/hpux/jasspa-mepkg-hpux-pa-10.20-$(MEVERS).depot
	$(GUNZIP) -c $(RELDIR)/jasspa-mepkg-hpux-pa-11.00-$(MEVERS).depot.gz > $@/hpux/jasspa-mepkg-hpux-pa-11.00-$(MEVERS).depot
	$(MKDIR) $@/linux
	$(CP) $(RELDIR)/jasspa-me-$(MEVERS)-1.src.rpm $@/linux/jasspa-me-redhat9-$(MEVERS)-1.src.rpm
	$(CP) $(RELDIR)/jasspa-me-$(MEVERS)-1.i386.rpm $@/linux/jasspa-me-redhat9-$(MEVERS)-1.i386.rpm
	$(CP) $(RELDIR)/jasspa-me_$(MEVERS)_i386.deb $@/linux
	$(CP) $(RELDIR)/centos44/jasspa-me-$(MEVERS)-1.src.rpm $@/linux/jasspa-me-centos44-$(MEVERS)-1.src.rpm
	$(CP) $(RELDIR)/centos44/jasspa-me-$(MEVERS)-1.i386.rpm $@/linux/jasspa-me-centos44-$(MEVERS)-1.i386.rpm
	$(CP) package.txt $@/readme.unix
	$(UNIX2DOS) package.txt $@/readme.joliet
	$(UNIX2MAC) package.txt $@/readme.hfs
	$(MKDIR) $@/cygwin
	$(CP) $(RELDIR)/jasspa-$(PMEVERS)-src.tar.bz2 $@/cygwin
	$(CP) $(RELDIR)/jasspa-$(PMEVERS).tar.bz2 $@/cygwin
	$(CP) $(RELDIR)/doc/cygwin.txt $@/cygwin
#	$(MKDIR) $@/sunos/i386_59
#	$(UNZIP) $(RELDIR)/jasspa-mepkg-sun-i386-59-$(MEVERS).zip -d $@/sunos/i386_59
#	$(MKDIR) $@/sunos/sparc6
#	$(UNZIP) $(RELDIR)/jasspa-mepkg-sun-sparc-56-$(MEVERS).zip -d $@/sunos/sparc6

#
# Source directory
#
$(FDIR)/src:
	$(MKDIR) $@
	$(GUNZIP) -c $(RELDIR)/jasspa-mesrc-$(PMEVERS).tar.gz | ($(CD) $@; tar xf -)
#
# Executable direcotory.
#
$(FDIR)/me $(EDIR)/me: me.txt
	- $(RMDIR) $@
	$(MKDIR) $@
	$(GUNZIP) -c $(RELDIR)/jasspa-metree-$(PMEVERS).tar.gz | ($(CD) $@; tar xf -)
	$(MV) $@/jasspa/* $@
	$(RMDIR) $@/jasspa
	$(GUNZIP) -c $(RELDIR)/jasspa-me-linux-i386-2.2-$(MEVERS).gz > $@/melinux22
	chmod a+rx $@/melinux22
	$(GUNZIP) -c $(RELDIR)/jasspa-me-linux-i386-2.4-$(MEVERS).gz > $@/melinux24
	chmod a+rx $@/melinux24
	$(GUNZIP) -c $(RELDIR)/jasspa-me-linux-curses-i386-2.4-$(MEVERS).gz > $@/melinux24c
	chmod a+rx $@/melinux24c
	$(GUNZIP) -c $(RELDIR)/jasspa-me-linux-i386-2.6-$(MEVERS).gz > $@/melinux26
	chmod a+rx $@/melinux26
	$(ZCAT) $(RELDIR)/jasspa-me-sun-sparc-56-$(MEVERS).Z > $@/mesunos56
	chmod a+rx $@/mesunos56
	$(GUNZIP) -c $(RELDIR)/jasspa-me-sun-sparc-58-$(MEVERS).gz > $@/mesunos58
	chmod a+rx $@/mesunos58
	$(GUNZIP) -c $(RELDIR)/jasspa-me-sun-i386-58-$(MEVERS).gz > $@/mesun86_58
	chmod a+rx $@/mesun86_58
	$(GUNZIP) -c $(RELDIR)/jasspa-me-sun-sparc-510-$(MEVERS).gz > $@/mesunos510
	chmod a+rx $@/mesunos510
	$(GUNZIP) -c $(RELDIR)/jasspa-me-sun-i386-510-$(MEVERS).gz > $@/mesun86_510
	chmod a+rx $@/mesun86_58
	$(ZCAT) $(RELDIR)/jasspa-me-sgi-irix-6-$(MEVERS).Z > $@/meirix6
	chmod a+rx $@/meirix6
	$(GUNZIP) -c $(RELDIR)/jasspa-me-hpux-pa-10.20-$(MEVERS).gz > $@/mehpux10
	chmod a+rx $@/mehpux10
	$(GUNZIP) -c $(RELDIR)/jasspa-me-hpux-pa-11.00-$(MEVERS).gz > $@/mehpux11
	chmod a+rx $@/mehpux11
	$(ZCAT) $(RELDIR)/jasspa-me-ibm-aix-4-$(MEVERS).Z > $@/meaix4
	chmod a+rx $@/meaix4
	$(ZCAT) $(RELDIR)/jasspa-me-ibm-aix-5-$(MEVERS).Z > $@/meaix5
	chmod a+rx $@/meaix5
	$(GUNZIP) -c $(RELDIR)/jasspa-mec-apple-ppc-10.3-$(MEVERS).gz > $@/memacppc10_3
	chmod a+rx $@/memacppc10_3
	$(GUNZIP) -c $(RELDIR)/jasspa-me-apple-ppc-10.3-$(MEVERS).gz > $@/memacppc10_3_x11
	chmod a+rx $@/memacppc10_3_x11
	$(GUNZIP) -c $(RELDIR)/jasspa-me-openbsd-i386-3.4-$(MEVERS).gz > $@/meopenbsd34
	chmod a+rx $@/meopenbsd34
	$(GUNZIP) -c $(RELDIR)/jasspa-me-freebsd-i386-5.2-$(MEVERS).gz > $@/mefreebsd52
	chmod a+rx $@/mefreebsd52
	$(GUNZIP) -c $(RELDIR)/jasspa-me-cygwin-i386-$(PMEVERS).gz > $@/mecygwin.exe
	chmod a+rx $@/mecygwin.exe
	$(GUNZIP) -c $(RELDIR)/jasspa-mex-cygwin-i386-$(PMEVERS).gz > $@/mecygwin_x11.exe
	chmod a+rx $@/mecygwin_x11.exe
	$(UNZIP) $(RELDIR)/jasspa-me-ms-win32-$(MEVERS).zip -d $@
	$(UNZIP) $(RELDIR)/jasspa-mec-ms-win32-$(MEVERS).zip -d $@
	$(UNZIP) $(RELDIR)/jasspa-me-ms-win32s-$(MEVERS).zip -d $@
	$(UNZIP) $(RELDIR)/jasspa-mecw-ms-win32-$(MEVERS).zip -d $@
	$(CP) $(RELDIR)/meicons.exe $@
	$(UNZIP) $(RELDIR)/jasspa-me-ms-dos-$(MEVERS).zip -d $@
	$(MV) $@/me.exe $@/medos.exe
	$(CP) me.txt $@/readme.unix
	$(UNIX2DOS) me.txt $@/readme.joliet
	$(UNIX2MAC) me.txt $@/readme.hfs
#	$(GUNZIP) -c $(RELDIR)/jasspa-me-sun-i386-59-$(MEVERS).gz > $@/mesun86_59
#	chmod a+rx $@/mesun86_59
#	$(GUNZIP) -c $(RELDIR)/jasspa-me-sun-sparc-59-$(MEVERS).gz > $@/mesunos59
#	chmod a+rx $@/mesunos59
#	$(UNZIP) $(RELDIR)/jasspa-me-msdos-djgpp2-$(MEVERS).zip -d $@
#	$(MV) $@/me.exe $@/medosdj2.exe

#
# Archive directories.
#
$(FDIR)/archive:
	$(MKDIR) $@
	$(CP) $(RELDIR)/jasspa-mesrc-$(MEVERS).tar.gz $@
	$(CP) $(RELDIR)/jasspa-mesrc-$(MEVERS).zip $@
	$(CP) $(RELDIR)/jasspa-mesrc-$(PMEVERS).tar.gz $@
	$(CP) $(RELDIR)/jasspa-mesrc-$(PMEVERS).zip $@
	$(MKDIR) $@/bin
	$(CP) $(RELDIR)/jasspa-me-freebsd-i386-5.2-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-me-hpux-pa-10.20-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-me-hpux-pa-11.00-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-me-ibm-aix-4-$(MEVERS).Z $@/bin
	$(CP) $(RELDIR)/jasspa-me-ibm-aix-5-$(MEVERS).Z $@/bin
	$(CP) $(RELDIR)/jasspa-me-linux-i386-2.2-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-me-linux-i386-2.4-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-me-linux-curses-i386-2.4-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-me-linux-i386-2.6-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-me-ms-win32-$(MEVERS).zip $@/bin
	$(CP) $(RELDIR)/jasspa-me-ms-win32s-$(MEVERS).zip $@/bin
	$(CP) $(RELDIR)/jasspa-me-ms-dos-$(MEVERS).zip $@/bin
	$(CP) $(RELDIR)/jasspa-me-openbsd-i386-3.4-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-me-sgi-irix-6-$(MEVERS).Z $@/bin
	$(CP) $(RELDIR)/jasspa-me-sun-i386-58-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-me-sun-i386-510-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-me-sun-sparc-56-$(MEVERS).Z $@/bin
	$(CP) $(RELDIR)/jasspa-me-sun-sparc-58-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-me-sun-sparc-510-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-mec-ms-win32-$(MEVERS).zip $@/bin
	$(CP) $(RELDIR)/jasspa-mecw-ms-win32-$(MEVERS).zip $@/bin
	$(CP) $(RELDIR)/jasspa-me-apple-ppc-10.3-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-mec-apple-ppc-10.3-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-me-cygwin-i386-$(PMEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-mex-cygwin-i386-$(PMEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-ne-freebsd-i386-5.2-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-ne-hpux-pa-10.20-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-ne-hpux-pa-11.00-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-ne-ibm-aix-4-$(MEVERS).Z $@/bin
	$(CP) $(RELDIR)/jasspa-ne-ibm-aix-5-$(MEVERS).Z $@/bin
	$(CP) $(RELDIR)/jasspa-ne-linux-i386-2.2-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-ne-linux-i386-2.4-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-ne-linux-curses-i386-2.4-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-ne-linux-i386-2.6-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-ne-ms-dos-$(MEVERS).zip $@/bin
	$(CP) $(RELDIR)/jasspa-ne-openbsd-i386-3.4-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-ne-sgi-irix-6-$(MEVERS).Z $@/bin
	$(CP) $(RELDIR)/jasspa-ne-sun-i386-58-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-ne-sun-sparc-56-$(MEVERS).Z $@/bin
	$(CP) $(RELDIR)/jasspa-ne-sun-sparc-58-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-ne-sun-sparc-510-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-new-ms-win32-$(MEVERS).zip $@/bin
	$(CP) $(RELDIR)/jasspa-ne-ms-win32-$(MEVERS).zip $@/bin
	$(CP) $(RELDIR)/jasspa-new-ms-win32s-$(MEVERS).zip $@/bin
	$(CP) $(RELDIR)/jasspa-nec-ms-win32s-$(MEVERS).zip $@/bin
	$(CP) $(RELDIR)/jasspa-ne-apple-ppc-10.3-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-ne-linux-curses-i386-2.4-$(MEVERS).gz $@/bin
	$(CP) $(RELDIR)/jasspa-ne-cygwin-i386-$(PMEVERS).gz $@/bin
	$(CP) $(RELDIR)/meicons.exe $@/bin
	$(MKDIR) $@/macros
	$(CP) $(RELDIR)/jasspa-memacros-$(MEVERS).tar.gz $@/macros
	$(CP) $(RELDIR)/jasspa-memacros-$(MEVERS).zip $@/macros
	$(CP) $(RELDIR)/jasspa-metree-$(MEVERS).tar.gz $@
	$(CP) $(RELDIR)/jasspa-metree-$(MEVERS).zip $@
	$(CP) $(RELDIR)/jasspa-memacros-$(PMEVERS).tar.gz $@/macros
	$(CP) $(RELDIR)/jasspa-memacros-$(PMEVERS).zip $@/macros
	$(CP) $(RELDIR)/jasspa-metree-$(PMEVERS).tar.gz $@
	$(CP) $(RELDIR)/jasspa-metree-$(PMEVERS).zip $@
	$(MKDIR) $@/doc
	$(CP) $(RELDIR)/jasspa-mehtml-$(MEVERS).tar.gz $@/doc
	$(CP) $(RELDIR)/jasspa-mehtm-$(MEVERS).zip $@/doc
	$(CP) $(RELDIR)/jasspa-mewinhlp-ms-win32-$(MEVERS).zip $@/doc
	$(MKDIR) $@/icons
	$(CP) $(RELDIR)/meicons.tar.gz $@/icons
	$(CP) $(RELDIR)/meicons.exe $@/icons
	$(CP) $(RELDIR)/meicons.zip $@/icons
	$(CP) $(RELDIR)/readme.jasspa_gnome $@/icons
	$(CP) $(RELDIR)/jasspa_gnome.tar.zip $@/icons
	$(CP) $(RELDIR)/meicons-extra.tar.gz $@/icons
	$(CP) $(RELDIR)/meicons-extra.tar.zip $@/icons
	$(CP) $(RELDIR)/meicons_pm.tar.gz $@/icons
#
# Limited archive.
#
$(EDIR)/archive:
	$(MKDIR) $@
	$(CP) $(RELDIR)/jasspa-mesrc-$(PMEVERS).tar.gz $@
	$(CP) $(RELDIR)/jasspa-metree-$(PMEVERS).tar.gz $@
#
# User submissions.
#
$(FDIR)/subs $(EDIR)/subs:
	$(MKDIR) $@
	$(CP) $(SUBDIR)/*.txt $@
#
# Manual pages
#
$(FDIR)/man/cat1 $(EDIR)/man/cat1:
	$(MKDIR) $@
	$(GUNZIP) -c $(RELDIR)/me.1.gz > $@/me.1
	$(GUNZIP) -c $(RELDIR)/ne.1.gz > $@/ne.1

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

jasspa-mecd-full-$(MEVERS).iso: $(FDIRS)
	$(CD) $(FDIR); $(MKISOFS) -o ../$@ -V $(FLABEL) $(FULLMKISOFSFLAGS) ./
jasspa-mecd-exec-$(MEVERS).iso: $(EDIRS)
	$(CD) $(EDIR); $(MKISOFS) -o ../$@ -V $(ELABEL) $(EXECMKISOFSFLAGS) ./
jasspa-mecd-lexe-$(MEVERS).iso: $(EDIRS)
	$(CD) $(EDIR); $(MKISOFS) $(ISOEXTRA) -o ../$@ -V $(LLABEL) $(EXECMKISOFSFLAGS) ./

jasspa-mecd-full-$(MEVERS).zip:	jasspa-mecd-full-$(MEVERS).iso
	$(ZIP) $@ jasspa-mecd-full-$(MEVERS).iso
jasspa-mecd-exec-$(MEVERS).zip: jasspa-mecd-exec-$(MEVERS).iso
	$(ZIP) $@ jasspa-mecd-exec-$(MEVERS).iso
jasspa-mecd-lexe-$(MEVERS).zip: jasspa-mecd-lexe-$(MEVERS).iso
	$(ZIP) $@ jasspa-mecd-lexe-$(MEVERS).iso
