		----------------------------------------
		-- Machine Level Programming for 386+ --
		----------------------------------------

-- Warning: Some of these routines require a knowledge of 
-- machine-level programming. You could crash your system!

-- These routines, along with peek(), poke() and call(), let you access all 
-- of the features of your computer.  You can read and write to any memory 
-- location, and you can create and execute machine code subroutines.

-- Writing characters to screen memory with poke() is much faster than  
-- using puts().
-- address of start of text screen memory 
--       mono: #B0000
--      color: #B8000

-- see demo\callmach.ex for an example of calling a machine language routine

constant M_ALLOC = 16,
	 M_FREE = 17,
	 M_ALLOC_LOW = 32,
	 M_FREE_LOW = 33,
	 M_INTERRUPT = 34,
	 M_SET_RAND = 35,
	 M_USE_VESA = 36,
	 M_CRASH_MESSAGE = 37,
	 M_TICK_RATE = 38

-- biggest address on a 32-bit machine
constant MAX_ADDR = power(2, 32)-1

-- biggest address accessible to 16-bit real mode
constant LOW_ADDR = power(2, 20)-1

type machine_addr(atom a)
-- a legal machine address 
    return a > 0 and a <= MAX_ADDR and floor(a) = a
end type

type low_machine_addr(atom a)
-- a legal low machine address 
    return a > 0 and a <= LOW_ADDR and floor(a) = a
end type

global constant REG_LIST_SIZE = 10
global constant REG_DI = 1,      
		REG_SI = 2,
		REG_BP = 3,
		REG_BX = 4,
		REG_DX = 5,
		REG_CX = 6,
		REG_AX = 7,
		REG_FLAGS = 8, -- on input: ignored 
			       -- on output: low bit has carry flag for 
			       -- success/fail
		REG_ES = 9,
		REG_DS = 10

type register_list(sequence r)
-- a list of register values
    return length(r) = REG_LIST_SIZE
end type

global function allocate(integer n)
-- Allocate n bytes of memory and return the address.
-- Free the memory using free() below.
    return machine_func(M_ALLOC, n)
end function

global procedure free(machine_addr a)
-- free the memory at address a
    machine_proc(M_FREE, a)
end procedure

global function allocate_low(integer n)
-- Allocate n bytes of low memory (address less than 1Mb) 
-- and return the address. Free this memory using free_low() below.
-- Addresses in this range can be passed to DOS during software interrupts.
    return machine_func(M_ALLOC_LOW, n)
end function

global procedure free_low(low_machine_addr a)
-- free the low memory at address a
    machine_proc(M_FREE_LOW, a)
end procedure

global function dos_interrupt(integer int_no, register_list input_regs)
-- call the DOS operating system via software interrupt int_no, using the
-- register values in input_regs. A similar register_list is returned.
-- It contains the register values after the interrupt.
    return machine_func(M_INTERRUPT, {int_no, input_regs})
end function

global function int_to_bytes(atom x)
-- returns value of x as a sequence of 4 bytes 
-- that you can poke into memory 
--      {bits 0-7,  (least significant)
--       bits 8-15,
--       bits 16-23,
--       bits 24-31} (most significant)
-- This is the order of bytes in memory on 386+ machines.
    integer a,b,c,d
    
    a = remainder(x, #100)
    x = floor(x / #100)
    b = remainder(x, #100)
    x = floor(x / #100)
    c = remainder(x, #100)
    x = floor(x / #100)
    d = remainder(x, #100)
    return {a,b,c,d}
end function

global function bytes_to_int(sequence s)
-- converts 4-byte peek() sequence into an integer value
    return s[1] + 
	   s[2] * #100 + 
	   s[3] * #10000 + 
	   s[4] * #1000000
end function

global function int_to_bits(atom x, integer nbits)
-- Returns the low-order nbits bits of x as a sequence of 1's and 0's. 
-- Note that the least significant bits come first. You can use Euphoria's
-- and/or/not operators on sequences of bits. You can also subscript, 
-- slice, concatenate etc. to manipulate bits.
    sequence bits
    
    if x < 0 then
	x = x + power(2, nbits) -- provide 2's complement bit pattern
    end if
    bits = repeat(0, nbits)
    for i = 1 to nbits do
	bits[i] = remainder(x, 2) 
	x = floor(x / 2)
    end for
    return bits
end function

global function bits_to_int(sequence bits)
-- get the (positive) value of a sequence of "bits"
    atom value, p
    
    value = 0
    p = 1
    for i = 1 to length(bits) do
	if bits[i] then
	    value = value + p
	end if
	p = p + p
    end for
    return value
end function

global procedure set_rand(integer seed)
-- reset the random number generator 
-- A given value of seed will cause the same series of
-- random numbers to be generated from the rand() function
    machine_proc(M_SET_RAND, seed)
end procedure

global procedure use_vesa(integer code)
-- If code is 1 then force Euphoria to use the VESA graphics standard.
-- This may let Euphoria work better in SVGA modes with certain graphics cards.
-- If code is 0 then Euphoria's normal use of the graphics card is restored.
-- Values of code other than 0 or 1 should not be used.
    machine_proc(M_USE_VESA, code)
end procedure

global procedure crash_message(sequence msg)
-- Specify a final message to display for your user, in the event 
-- that Euphoria has to shut down your program due to an error.
    machine_proc(M_CRASH_MESSAGE, msg)
end procedure

global procedure tick_rate(atom rate)
-- Specify the number of clock-tick interrupts per second.
-- This determines the precision of the time() library routine, 
-- and also the sampling rate for time profiling.
    machine_proc(M_TICK_RATE, rate)
end procedure

