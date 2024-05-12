#############################################################################
#
# Utilities Makefile
#
#############################################################################
#
#  System        : Nroff
#  Module        : All
#  Object Name   : $RCSfile: win32v6.mak,v $
#  Revision      : $Revision: 1.2 $
#  Date	         : $Date: 2002-03-10 18:36:43 $
#  Author        : $Author: jon $
#  Last Modified : <240509.0024>
#
#  Description
#
#  Notes
#
#  History
#
#
##############################################################################

ROOTDIR	= .
include etc\win32v16.mak

install all spotless clean::
	cd utils
	$(MAKE) -f win32v16.mak $@
	cd ..
	cd nroff
	$(MAKE) -f win32v16.mak $@
	cd ..
	cd idc
	$(MAKE) -f win32v16.mak $@
	cd ..

	

	
