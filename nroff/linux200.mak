##############################################################################
#
#  			Copyright 2000 Samsung Electronics (UK) Ltd.
#			      All Rights Reserved
#
#
#  System        : 
#  Module        : 
#  Object Name   : $RCSfile: linux200.mak,v $
#  Revision      : $Revision: 1.1 $
#  Date          : $Date: 2000-10-21 15:01:44 $
#  Author        : $Author: jon $
#  Created By    : Jon Green
#  Created       : Sat Oct 21 13:43:24 2000
#  Last Modified : <001021.1456>
#
#  Description	
#
#  Notes
#
#  History
#	
#  $Log: not supported by cvs2svn $
#  Revision 1.1  2000/10/21 14:31:20  jon
#  Import
#
#
##############################################################################
#
#  Copyright (c) 2000 Samsung Electronics (UK) Ltd.
# 
#  All Rights Reserved.
# 
#  This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
#  reproduced,  translated,  or  reduced to any  electronic  medium or machine
#  readable form without prior written consent from Samsung Electronics (UK) Ltd.
#
##############################################################################

ROOTDIR	= .
include etc/linux200.mak

install all spotless clean::
	cd utils; $(MAKE) -f linux200.mak $@
	cd nroff; $(MAKE) -f linux200.mak $@
	cd idc; $(MAKE)   -f linux200.mak $@
	cd nroffdoc; $(MAKE) -f linux200.mak $@

clean spotless::
	cd etc; $(MAKE) -f linux200.mak $@

spotless::
	$(RM) -r $(INSTBINDIR)/*
	$(RM) -r $(INSTLIBDIR)/*
	$(RM) -r $(INSTINCDIR)/*
	- $(RMDIR) $(INSTBINDIR)
	- $(RMDIR) $(INSTLIBDIR)
	- $(RMDIR) $(INSTINCDIR)
	- $(RMDIR) ./lib
	

	
