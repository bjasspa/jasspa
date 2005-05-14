##############################################################################
# This is part of the JASSPA MicroEmacs.
# Copyright (C) 2004 JASSPA (www.jasspa.com)
# See the file COPYING for copying and conditions.
#
# Created:       Thu Feb 5 21:36:24 2004
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
	$(RM) cygwin/*.1
	$(RM) cygwin/*.gz
	$(RM) cygwin/*.bz2
	$(RM) -r solaris/jasspa
	$(RM) -r hpux/jasspa
	$(RM) -r cygwin/usr
#
# Remove all working files.
#
spotless: clean
	$(RM) solaris/*~
	$(RM) redhat/*~
	$(RM) hpux/*~
	$(RM) cygwin/*~
	$(RM) *~
