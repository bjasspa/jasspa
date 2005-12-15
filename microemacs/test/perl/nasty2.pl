# -*-perl -*- ################################################################
#
#  			Copyright 2001 JASSPA.
#			      All Rights Reserved
#
#
#  System        : 
#  Module        : 
#  Object Name   : $RCSfile: nasty2.pl,v $
#  Revision      : $Revision: 1.1 $
#  Date          : $Date: 2005-12-15 22:36:42 $
#  Author        : $Author: bill $
#  Created By    : Steven Phillips
#  Created       : Wed Mar 14 15:45:43 2001
#  Last Modified : <010314.1545>
#
#  Description	
#
#  Notes
#
#  History
#	
#  $Log: not supported by cvs2svn $
#  Revision 1.1  2000/10/09 19:35:50  jon
#  Updated perl with standard tools
#
#
##############################################################################
#
#  Copyright (c) 2001 JASSPA.
# 
#  All Rights Reserved.
# 
#  This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
#  reproduced,  translated,  or  reduced to any  electronic  medium or machine
#  readable form without prior written consent from JASSPA.
#
##############################################################################


{
    @rows  = split(/\n/, $data);
    @yPsnr = map{(split(' ', $_))[0] . " ". (split(' ', $_))[3] . "\n"}
@rows;

    @yVals = map {(split(' ', $_))[3] . ","} @rows;
}

