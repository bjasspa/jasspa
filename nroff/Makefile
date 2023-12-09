##############################################################################
#
#                         Copyright 2004 Jon Green.
#                             All Rights Reserved
#
#
#  System        :
#  Module        :
#  Object Name   : $RCSfile: Makefile,v $
#  Revision      : $Revision: 1.3 $
#  Date          : $Date: 2004-02-07 19:29:48 $
#  Author        : $Author: jon $
#  Created By    : Jon Green
#  Created       : Sat Feb 7 15:01:38 2004
#  Last Modified : <231205.1140>
#
#  Description
#
#  Notes
#
#  History
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

TOPDIR  = .
include $(TOPDIR)/etc/makeinc

install all spotless clean::
	cd utils$(sc)    $(MAKE) $@
	cd nroff$(sc)    $(MAKE) $@
	cd idc$(sc)      $(MAKE) $@
	cd nroffdoc$(sc) $(MAKE) $@

clean spotless::
	cd etc$(sc) $(MAKE) $@

spotless::
	$(RMDIR) $(BINDIR)
	$(RMDIR) $(LIBDIR)
	$(RMDIR) $(INSTINCDIR)
	$(RM) *~
	$(RM) tags
