#[
 #
 #  Copyright (c) 2025, University of Potsdam, Germany
 #  License       :                     
 #  Date          : $Date$              
 #  Author        : $Author$            
 #  Created By    : Detlef Groth         
 #  Created       : Thu Oct 9 22:31:28 2025        
 #  Last Modified : <251009.2306>       
 #                                      
 #  Description   :                     
 #                                      
]#

when isMainModule:
  echo "testing"
var file = open()
defer: f.close()
var line
while f.readLine(line):
  test
    
let f  = open(,fmWrite)
defer: f.close()
f.writeLine("Hello")
    

