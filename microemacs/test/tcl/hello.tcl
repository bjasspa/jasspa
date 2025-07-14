#!/usr/bin/env tclsh
##############################################################################
#
#  Date          : $Date$
#  Author        : $Author$
#  Created By    : MicroEmacs User
#  Created       : 2025-03-25 07:26:26
#  Last Modified : <250714.1315>
#
#  Description	 :
#
#  Notes         :
#
#  History       :
#
##############################################################################
#
#  Copyright (c) 2025 MicroEmacs User.
#
#  All Rights Reserved.
#
#  This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
#  reproduced,  translated,  or  reduced to any  electronic  medium or machine
#  readable form without prior written consent from MicroEmacs User.
#
##############################################################################

puts Hi
set x 2
if {$x < 3} {puts "x is small"}

proc test {x} {
    pts " $x"
}

test [lindex $argv 0]
