##############################################################################
#
#  			Copyright 2004 <unknown>.
#			      All Rights Reserved
#
#
#  System        : 
#  Module        : 
#  Object Name   : $RCSfile: Makefile,v $
#  Revision      : $Revision: 1.2 $
#  Date          : $Date: 2004-02-07 17:38:37 $
#  Author        : $Author: jon $
#  Created By    : <unknown>
#  Created       : Sat Feb 7 15:01:38 2004
#  Last Modified : <040207.1701>
#
#  Description	
#
#  Notes
#
#  History
#
##############################################################################
#
#  Copyright (c) 2004 <unknown>.
# 
#  All Rights Reserved.
# 
#  This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
#  reproduced,  translated,  or  reduced to any  electronic  medium or machine
#  readable form without prior written consent from <unknown>.
#
##############################################################################

TOPDIR	= .
include $(TOPDIR)/etc/makeinc

install all spotless clean::
	cd utils;    $(MAKE) $@
	cd nroff;    $(MAKE) $@
	cd idc;      $(MAKE) $@
	cd nroffdoc; $(MAKE) $@

clean spotless::
	cd etc; $(MAKE) $@

spotless::
	$(RMDIR) $(BINDIR)
	$(RMDIR) $(LIBDIR)
	$(RMDIR) $(INSTINCDIR)
	$(RM) *~
	$(RM) tags

