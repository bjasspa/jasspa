##############################################################################
#
#  			Copyright 2004 Jon Green.
#			      All Rights Reserved
#
#
#  System        : 
#  Module        : 
#  Object Name   : $RCSfile: Makefile,v $
#  Revision      : $Revision: 1.1 $
#  Date          : $Date: 2004-02-06 00:27:31 $
#  Author        : $Author: jon $
#  Created By    : Jon Green
#  Created       : Wed Feb 4 23:33:14 2004
#  Last Modified : <040205.2357>
#
#  Description	
#
#  Notes
#
#  History
#	
#  $Log: not supported by cvs2svn $
#
##############################################################################
#
#  Copyright (c) 2004 Jon Green.
# 
#  All Rights Reserved.
# 
#  This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
#  reproduced,  translated,  or  reduced to any  electronic  medium or machine
#  readable form without prior written consent from Jon Green.
#
##############################################################################

SRCROOT=.
include $(SRCROOT)/etc/makeinc

all:
	cd macros; 	$(MAKE) $@
	cd spelling; 	$(MAKE) $@
	cd company; 	$(MAKE) $@
	cd src; 	$(MAKE) $@

install:
	cd macros; 	$(MAKE) $@
	cd spelling; 	$(MAKE) $@
	cd company; 	$(MAKE) $@
	cd src; 	$(MAKE) $@

release:
	cd macros; 	$(MAKE) $@
	cd spelling; 	$(MAKE) $@
	cd company; 	$(MAKE) $@
	cd src; 	$(MAKE) $@

clean:
	cd macros; 	$(MAKE) $@
	cd spelling; 	$(MAKE) $@
	cd company; 	$(MAKE) $@
	cd src; 	$(MAKE) $@
	$(RMDIR) $(RELDIR)

spotless: clean
	cd macros; 	$(MAKE) $@
	cd spelling; 	$(MAKE) $@
	cd company; 	$(MAKE) $@
	cd src; 	$(MAKE) $@
	$(RM) *~

