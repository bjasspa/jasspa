##############################################################################
#
#			Copyright 2004 Jon Green.
#			      All Rights Reserved
#
#
#  System        :
#  Module        :
#  Object Name   : $RCSfile: Makefile,v $
#  Revision      : $Revision: 1.1 $
#  Date          : $Date: 2004-02-13 15:23:17 $
#  Author        : $Author: jon $
#  Created By    : Jon Green
#  Created       : Fri Feb 13 15:12:17 2004
#  Last Modified : <040213.1523>
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

TOPDIR=.
include etc/makeinc

all install clean:
	cd dos2unix; $(MAKE) $@
	cd mac2unix; $(MAKE) $@
	cd scrubit; $(MAKE) $@

spotless:
	cd dos2unix; $(MAKE) $@
	cd mac2unix; $(MAKE) $@
	cd scrubit; $(MAKE) $@
	$(RMDIR) ./bin
	$(RMDIR) ./lib
	$(RMDIR) ./include
	$(RM) *~
	$(RM) tags
