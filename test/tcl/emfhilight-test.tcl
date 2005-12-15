#!/bin/sh
# -*- tcl -*- \
exec wish -file $0 ${1+"$@"}
##############################################################################
#
#  			Copyright 2005 $COMPANY$.
#			      All Rights Reserved
#
#
#  System        : EMFHILIGHT-TEST_TCL
#  Module        : 
#  Object Name   : $RCSfile: emfhilight-test.tcl,v $
#  Revision      : $Revision: 1.1 $
#  Date          : $Date: 2005-12-15 22:36:58 $
#  Author        : $Author: bill $
#  Created By    : $AUTHOR$
#  Created       : $DATE$
#  Last Modified : <050223.1936>
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
#  Copyright (c) 2005 $COMPANY$
# 
#  All Rights Reserved.
# 
#  This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
#  reproduced,  translated,  or  reduced to any  electronic  medium or machine
#  readable form without prior written consent from MPIMG Berlin, Germany.
#
##############################################################################
package provide app-emfhilight-test 1.0
lappend auto_path [file join "[file rootname [info script]].vfs" lib]
lappend auto_path d:/tcl-lib
package require snit 0.93
snit::type emfhilight-test {
    # options
    option -op1 ""
    # variables
    variable v1
    # typevariables
    typevariable tv1
    
    constructor {args} {
         $self configurelist $args
     }   
     destructor {
         
     }
    typeconstructor {

    }
    # public methods (are lowercase)
    method m1 {args} {

    }
    # private methods (are uppercase)
    method M1 {args} {

    }
}

# test
proc ::test {} {
  emfhilight-test test
  test m1
}
namespace eval sample {
    variable var
}
proc sample::sample {args} {
  puts "inside sample"
}
# prns mulitliner oo
prns::obj o1 -color red       ;# create new object/class o1 "namespace ::o1 , object proc ::o1::o1, interp alias ::o1"
o1 instproc foo {} {    ;# create method of object "o1" named "foo"
    instvar color
    puts $color
}
o1 foo              ;# invoke method foo

o1 instproc init args {      ;# constructor proc
    puts "init [my_ set color]"
}

# thingy an onliner oo
proc thingy name {
    proc $name args "namespace eval $name \$args"
}

thingy foo
foo set bar 1                ;#== set foo::bar 1
foo proc grill x {
    subst $x!
}
foo proc test {arg} {
    puts test
}
# xotcl
Class BaseClass
BaseClass instproc speak {} {
    puts "Hello from BC."
}

Class DerivedClass -superclass BaseClass
DerivedClass instproc speak {} {
    next
    puts "Hello from DC."
}
BaseClass instproc unknown {m args} {
    puts "What? $m? I don't understand."
}
o1 sing

o1 proc sing {} {
    puts "Ok, here it goes: Lala Lala!"
}
o1 sing
Class M
M instproc sing {} {
  puts -nonewline "[self] sings: "
  next
}
M instproc unknown args {
  puts -nonewline "[self] is confused: "
  next
}
rename string tcl::string
Object string

# Now we can define the new procs as object procs like in the following. These object procs can be used as subcommands.

string proc charsort { string }  {
    return [join [lsort [split $string {}] ] {} ]
}

string proc insert {string pos char} {
    set original [string index $string $pos]
    string replace $string $pos $pos $char$original
}

string proc letterspace s {
    join [split $s ""] " "
}

string proc linbreak {s {width 80}} {
    set res {}
    while {[string length $s]>$width} {
        set     pos [string wordstart $s $width]
        lappend res [string range     $s 0 [expr {$pos-1}]]
        set     s   [string range     $s $pos end]
    }
    lappend res $s
}
# incr Tcl

itcl::class C1 {

}

itcl::class C2 {
    variable m1
    variable m2
    constructor {} {
        set m1 [C1 #auto]
        set m2 [C1 #auto]
    }
    destructor {
        itcl::delete object $m1 $m2
    }
}
class File {
    private variable fid ""
    constructor {name access} {
        set fid [open $name $access]
    }
    destructor {
        close $fid
    }
    
    method get {}
    method put {line}
    method eof {}
}

body File::get {} {
    return [gets $fid]
}
body File::put {line} {
    puts $fid $line
}
body File::eof {} {
    return [::eof $fid]
}

#
# See the File class in action:
#
File x /etc/passwd "r"
while {![x eof]} {
    puts "=> [x get]"
}
delete object x
# stooop

class C2 {
    proc C2 {this} {
        set ($this,o1) [new C1]
        set ($this,o2) [new C1]
    }
    proc ~C2 {this} {
        delete $($this,o1) $($this,o2)
    }
    class C1 {
        proc C1 {this} {}
        proc ~C1 {this} {}
    }
}
delete [new C2]

# Note: in the example above, C1 is a class embedded in C2, 
# but might as well be defined outside of C2, as in:
class C1 {
    proc C1 {this} {}
    proc ~C1 {this} {}
}
class C2 {
    proc C2 {this} {
        set ($this,o1) [new C1]
        set ($this,o2) [new C1]
    }
    proc ~C2 {this} {
        delete $($this,o1) $($this,o2)
    }
}
