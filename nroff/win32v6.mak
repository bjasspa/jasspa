##############################################################################
#
#  			Copyright 2000 Samsung Electronics (UK) Ltd.
#			      All Rights Reserved
#
#
#  System        : 
#  Module        : 
#  Object Name   : $RCSfile: win32v6.mak,v $
#  Revision      : $Revision: 1.1 $
#  Date          : $Date: 2000-10-21 14:31:20 $
#  Author        : $Author: jon $
#  Created By    : Jon Green
#  Created       : Sat Oct 21 13:43:24 2000
#  Last Modified : <001021.1403>
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
include etc\win32v6.mak

install all spotless clean::
	cd utils
	$(MAKE) -f win32v6.mak $@
	cd ..
	cd nroff
	$(MAKE) -f win32v6.mak $@
	cd ..
	cd idc
	$(MAKE) -f win32v6.mak $@
	cd ..

	

	
