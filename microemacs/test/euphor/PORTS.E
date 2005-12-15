--  ports.e 
--  by Jacques Deschenes, Baie-Comeau, PQ, Canada, e-mail: desja@quebectel.com
--  creation date: september 7th, 1996
--
--  the functions defined in this file are for reading and writing to I/O ports.
--  *****     Exported functions ***
--      Input  to read a byte from an I/O port
--      Output to write a byte to an I/O port
--      InputW to read a word from an I/O port
--      OutputW to write a word to an I/O port

include machine.e

global type byte(integer b)
   return b >= 0 and b < 256
end type --byte()

global type word(integer w)
   return w >= 0 and w <= #FFFF
end type -- word()

sequence OutputCode, InputCode, OutputWCode, InputWCode 

OutputCode = {
		#50,        -- PUSH EAX
		#52,        -- PUSH EDX
		#BA,0,0,0,0,-- MOV EDX, PORT  (port, to be poked in) (3) 
		#B0,#00,    -- MOV AL, byte  (byte, to be poked in) (8) 
		#EE,        -- OUT DX, AL           
		#5A,        -- POP EDX
		#58,        -- POP EAX
		#C3         -- RET
	    }
	    
InputCode = {
		#50,        -- PUSH EAX
		#52,        -- PUSH EDX
		#BA,0,0,0,0,-- MOV EDX, PORT  (port to be poked in) (3)
		#EC,        -- IN AL, DX            
		#BA,0,0,0,0,--  MOV EDX, storage address (9)
		#88,#02,    -- MOV [EDX],AL
		#5A,        -- POP EDX
		#58,        -- POP EAX
		#C3,        -- RET
		#0          -- DB ? result holder
	    }
	    
OutputWCode = {
		#50,        -- PUSH EAX
		#52,        -- PUSH EDX
		#BA,0,0,0,0,-- MOV EDX, PORT  (port, to be poked in) (3) 
		#B8,0,0,0,0,-- MOV EAX, word  ;word to be poked in) (8) 
		#EE,        -- OUT DX, AL   ;output low byte        
		#86,#C4,    -- XCHG AH,AL    ; send high byte in al
		#EE,        -- OUT DX, AL    ;output high byte        
		#5A,        -- POP EDX
		#58,        -- POP EAX
		#C3         -- RET
		}
		
InputWCode = {
		#50,        -- PUSH EAX
		#53,        -- PUSH EBX
		#52,        -- PUSH EDX
		#BA,0,0,0,0,-- MOV EDX, PORT  (port to be poked in) (4)
		#BB,0,0,0,0,-- MOV EBX, storage address (9)
		#EC,        -- IN AL, EDX            
		#88,#03,    -- MOV [EBX],AL
		#43,        -- INC EBX
		#EC,        -- IN AL, EDX
		#88,#03,    -- MOV [EBX],AL
		#5A,        -- POP EDX
		#5B,        -- POP EBX
		#58,        -- POP EAX
		#C3,        -- RET
		#0,#0       -- DW ? result holder
		}
		
		
atom InputAsm, OutputAsm, OutputWAsm, InputWAsm

integer InputCodeLength, OutputCodeLength, InputWCodeLength, OuputWCodeLength

constant InitPortsError = "Not enough memory to initialise ports.e\n"

integer AlreadyInit  -- set to 1 by InitPorts
AlreadyInit = 0

global procedure InitPorts()
-- init code space for assembler routine
    if AlreadyInit then
	return
    end if
    InputCodeLength = length(InputCode)
    InputAsm = allocate(InputCodeLength)
    if not InputAsm then
	puts(1,InitPortsError)
	abort(1)
    end if
    poke(InputAsm,InputCode)
    poke(InputAsm+9,int_to_bytes(InputAsm+InputCodeLength-1)) -- storage pointer
    
    OutputCodeLength = length(OutputCode)
    OutputAsm = allocate(OutputCodeLength)
    if not InputAsm then
	puts(1,InitPortsError)
	abort(1)
    end if
    poke(OutputAsm,OutputCode)
    
    OuputWCodeLength = length(OutputWCode)
    OutputWAsm = allocate(OuputWCodeLength)
    if not OutputWAsm then
	puts(1,InitPortsError)
	abort(1)
    end if
    poke(OutputWAsm,OutputWCode)
    
    InputWCodeLength = length(InputWCode)
    InputWAsm = allocate(InputWCodeLength)
    if not InputWAsm then
	puts(1,InitPortsError)
	abort(1)
    end if
    poke(InputWAsm,InputWCode)
    poke(InputWAsm+9,int_to_bytes(InputWAsm+InputWCodeLength-2)) -- storage pointer
    AlreadyInit = 1
end procedure -- InitPorts()

global procedure FreePorts()
-- free memory occupied by assembler routines
  if not AlreadyInit then
     return
  end if
  free(InputAsm)
  InputAsm = 0
  free(OutputAsm)
  OutputAsm = 0
  free(OutputWAsm)
  OutputWAsm = 0
  free(InputWAsm)
  InputWAsm = 0
  AlreadyInit = 0 
end procedure -- FreePorts()

global function Input(word port)
 byte b
    if not InputAsm then
	return -1
    end if
    poke(InputAsm+3,int_to_bytes(port))
    call(InputAsm)
    b = peek(InputAsm+InputCodeLength-1)
    return b
end function -- Input()

global  procedure Output(byte b, word port)
    if not OutputAsm then
	return 
    end if
    poke(OutputAsm+3,int_to_bytes(port))
    poke(OutputAsm+8,b)
    call(OutputAsm)
end procedure -- Output()

global function InputW(word port)
-- read a word form port.  Low byte is read then High byte
-- return HighByte*256+LowByte
    if not OutputWAsm then
	return -1
    end if
    poke(InputWAsm+4,int_to_bytes(port))
    call(InputWAsm)
    return peek(InputWAsm+InputWCodeLength-1)*256 + 
	   peek(InputWAsm+InputWCodeLength-2)
end function -- InputW()

global procedure OutputW(word w, word port)
    if not OutputWAsm then
	return
    end if
    poke(OutputWAsm+3,int_to_bytes(port))
    poke(OutputWAsm+8,int_to_bytes(w))
    call(OutputWAsm)
end procedure -- OutputW()

InitPorts()

