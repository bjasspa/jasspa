##############################################################################
#
#  			Copyright 2004 Jon Green.
#			      All Rights Reserved
#
#
#  System        : 
#  Module        : 
#  Object Name   : $RCSfile: sunos.mak,v $
#  Revision      : $Revision: 1.1 $
#  Date          : $Date: 2004-01-06 00:51:59 $
#  Author        : $Author: jon $
#  Created By    : Jon Green
#  Created       : Sat Jan 3 19:38:25 2004
#  Last Modified : <040103.1938>
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

ROOTDIR	= .
include etc/sunos.mak

install all spotless clean::
	cd utils; $(MAKE) -f sunos.mak $@
	cd nroff; $(MAKE) -f sunos.mak $@
	cd idc; $(MAKE)   -f sunos.mak $@
	cd nroffdoc; $(MAKE) -f sunos.mak $@

clean spotless::
	cd etc; $(MAKE) -f sunos.mak $@

spotless::
	$(RM) -r $(INSTBINDIR)/*
	$(RM) -r $(INSTLIBDIR)/*
	$(RM) -r $(INSTINCDIR)/*
	- $(RMDIR) $(INSTBINDIR)
	- $(RMDIR) $(INSTLIBDIR)
	- $(RMDIR) $(INSTINCDIR)
	- $(RMDIR) ./lib
