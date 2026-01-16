//-----------------------------------------------------
// Design Name : jkff_udp
// File Name   : jkff_udp.v
// Function    : JK Flip Flop Using UDP
// Coder       : Deepak Kumar Tala
//-----------------------------------------------------
primitive jkff_udp (q,clk,j,k);
	input clk,j,k;
	output q;
	reg q;
	table
		// clk j k : q : q+
		    r  0 0 : ? : - ;
		    r  0 1 : ? : 0 ;
		    r  1 0 : ? : 1 ;
		    r  1 1 : 0 : 1 ;
		    r  1 1 : 1 : 0 ;
		    f  ? ? : ? : - ;
		    ?  * ? : ? : - ;
		    ?  ? * : ? : - ;
	endtable
endprimitive
