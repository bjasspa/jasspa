##-*- makefile -*-############################################################
#
# Copyright (C) 2004-2025 JASSPA (www.jasspa.com).
#
# This is part of JASSPA's MicroEmacs, see the LICENSE file for licensing and
# copying information.
#
# Synopsis:      Installation Makefile
# Authors:       Jon Green & Steven Phillips
#
##############################################################################

TOPDIR=..
include $(TOPDIR)/etc/makeinc

release all:
	echo "Nothing to do for $@"
#
# Clean area.
#
clean:
	$(RM) solaris/*.1
	$(RM) solaris/*.gz
	$(RM) solaris/*.zip
	$(RM) redhat/*.1
	$(RM) redhat/*.gz
	$(RM) redhat/*.rpm
	$(RM) hpux/*.gz
	$(RM) hpux/*.1
	$(RM) debian/*.1
	$(RM) debian/*.deb
	$(RM) debian/*.gz
	$(RM) debian/*.pdf
	$(RM) debian/*.txt
	$(RM) debian/*.log
	$(RM) debian/COPYING
	$(RM) cygwin/*.1
	$(RM) cygwin/*.gz
	$(RM) cygwin/*.bz2
	$(RM) -r solaris/jasspa
	$(RM) -r hpux/jasspa
	$(RM) -r debian/package
	$(RM) -r cygwin/usr
#
# Remove all working files.
#
spotless: clean
	$(RM) solaris/*~
	$(RM) redhat/*~
	$(RM) hpux/*~
	$(RM) cygwin/*~
	$(RM) debian/*~
	$(RM) *~
