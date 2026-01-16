primitive mux_21_udp(out, sel, i0, i1);
output out; 
input sel, i0, i1;
table
   // sel i0 i1   out
        0  0  ?  : 0 ; // 1
        0  1  ?  : 1 ; // 2
        1  ?  0  : 0 ; // 3
        1  ?  1  : 1 ; // 4
        ?  0  0  : 0 ; // 5
        ?  1  1  : 1 ; // 6
endtable
endprimitive
