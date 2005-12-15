-- bitwise.e 
-- created september 14th, 1996
-- by Jacques Deschenes, Baie-Comeau, PQ, Canada, e-mail: desja@quebectel.com
-- implement bitwise logicals functions: NOT, OR, XOR and AND

include machine.e

sequence BitOrCode,BitAndCode, BitXorCode, BitNotCode -- contain HEX code for..

atom BitOr,BitAnd,BitXor, BitNot -- pointer to assembler routines

BitOrCode = {
		#50,                -- PUSH EAX
		#52,                -- PUSH EDX
		#BA,0,0,0,0,        -- MOV  EDX, Storage pointer
		#B8,0,0,0,0,        -- MOV EAX,  A
		#0D,0,0,0,0,        -- OR  EAX,  B
		#89,#02,            -- MOV [EDX], EAX
		#5A,                -- POP EDX
		#58,                -- POP EAX
		#C3,                -- RET
		0,0,0,0             -- result storage
	    }
	    
BitAndCode = {
		#50,                -- PUSH EAX
		#52,                -- PUSH EDX
		#BA,0,0,0,0,        -- MOV  EDX, Storage pointer
		#B8,0,0,0,0,        -- MOV EAX,  A
		#25,0,0,0,0,        -- AND  EAX,  B
		#89,#02,            -- MOV [EDX], EAX
		#5A,                -- POP EDX
		#58,                -- POP EAX
		#C3,                -- RET
		0,0,0,0             -- result storage
	    }
	    
BitXorCode = {
		#50,                -- PUSH EAX
		#52,                -- PUSH EDX
		#BA,0,0,0,0,        -- MOV  EDX, Storage pointer
		#B8,0,0,0,0,        -- MOV EAX,  A
		#35,0,0,0,0,        -- XOR  EAX,  B
		#89,#02,            -- MOV [EDX], EAX
		#5A,                -- POP EDX
		#58,                -- POP EAX
		#C3,                    -- RET
		0,0,0,0             -- result storage
	    }

BitNotCode = {
		#52,                -- PUSH EDX
		#BA,0,0,0,0,        -- MOV  EDX, Storage pointer
		#F7,#12,            -- NOT  [EDX] 
		#5A,                -- POP EDX
		#C3,                -- RET
		0,0,0,0             -- operand & result  storage
	    }

global procedure InitCode()
-- Code initialisation          
-- abort if failed
--  could be a function returning a SUCCESS/FAIL code

    BitOr = allocate(length(BitOrCode))
    if not BitOr then
	abort(1)
    end if
    poke(BitOr,BitOrCode)
    poke(BitOr+3,int_to_bytes(BitOr+length(BitOrCode)-4))

    BitAnd = allocate(length(BitAndCode))
    if not BitAnd then
	abort(1)
    end if
    poke(BitAnd,BitAndCode)
    poke(BitAnd+3,int_to_bytes(BitAnd+length(BitOrCode)-4))

    BitXor = allocate(length(BitXorCode))
    if not BitXor then
	abort(1)
    end if
    poke(BitXor,BitXorCode)
    poke(BitXor+3,int_to_bytes(BitXor+length(BitXorCode)-4))

    BitNot = allocate(length(BitNotCode))
    if not BitNot then
	abort(1)
    end if
    poke(BitNot,BitNotCode)
    poke(BitNot+2,int_to_bytes(BitNot+length(BitNotCode)-4))
end procedure -- InitCode()

global function AND(integer a, integer b)
   poke(BitAnd+8,int_to_bytes(a))
   poke(BitAnd+13,int_to_bytes(b))
   call(BitAnd)
   return bytes_to_int(peek({BitAnd+length(BitAndCode)-4,4}))
end function -- AND()

global function OR(integer a, integer b)
   poke(BitOr+8,int_to_bytes(a))
   poke(BitOr+13,int_to_bytes(b))
   call(BitOr)
   return bytes_to_int(peek({BitOr+length(BitOrCode)-4,4}))
end function -- OR()

global function XOR(integer a, integer b)
   poke(BitXor+8,int_to_bytes(a))
   poke(BitXor+13,int_to_bytes(b))
   call(BitXor)
   return bytes_to_int(peek({BitXor+length(BitXorCode)-4,4}))
end function -- XOR()

global function NOT(integer a)
   poke(BitNot+length(BitNotCode)-4,int_to_bytes(a))
   call(BitNot)
   return bytes_to_int(peek({BitNot+length(BitNotCode)-4,4}))
end function -- NOT()

global procedure FreeCodeSpace()
    free(BitAnd)
    free(BitOr)
    free(BitXor)
    free(BitNot)
end procedure

InitCode()

