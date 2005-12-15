-- Dos function call #21 wrap up.
-- by Jacques Deschenes, Baie-Comeau, PQ, Canada, e-mail: Desja@quebectel.com
-- Creation date: September 28th, 1996
-- Why to bother about using dos for files I/O when euphoria can do that?
-- Because with Euphoria you have to read file to sequence and then to poke
-- the sequence in a buffer.
-- with Dos file I/O one can read and write direct to buffer wich is faster.

include machine.e
include bitwise.e
include ports.e

global constant READ = 0, WRITE = 1, READ_WRITE=2

global function DosCreate(sequence Name, integer attributes)
-- Create a new file.
-- parameters: Name is name of the new file.
--             attributes for the new file.
-- if succes return File handle else return -1
sequence NameBuffer,regs
regs = repeat(0,10)
NameBuffer = allocate_low(length(Name)+1)
if not NameBuffer then
    return -1
end if
poke(NameBuffer,Name & 0)
regs[REG_AX] = #3C00
regs[REG_CX] = attributes
regs[REG_DS] = floor(NameBuffer/16)
regs[REG_DX] = remainder(NameBuffer,16)
regs = dos_interrupt(#21,regs)
if AND(regs[REG_FLAGS],1) then
    return -1 -- fail to open
end if
return regs[REG_AX]  -- return file handle
end function -- DosCreate()

global function DosOpen(sequence FileName, integer Mode)
-- use DOS function #3D to open a file
-- return file handle or -1 if error
-- mode is 0 = read only
--         1 = write only
--         2 = read/write 

atom NameBuffer
sequence regs
regs = repeat(0,10)
NameBuffer = allocate_low(length(FileName)+1)
if not NameBuffer then
    return -1
end if
poke(NameBuffer,FileName & 0)
regs[REG_AX] = #3D00+Mode
regs[REG_DS] = floor(NameBuffer/16)
regs[REG_DX] = remainder(NameBuffer,16)
regs = dos_interrupt(#21,regs)
if AND(regs[REG_FLAGS],1) then
    return -1 -- fail to open
end if
return regs[REG_AX]  -- return file handle
end function -- DosOpen()

global function DosClose(integer Handle)
-- close a file open with DosOpen()
-- return 1 if success else return 0
sequence regs
    regs = repeat(0,10)
    regs[REG_AX] = #3E00
    regs[REG_BX] = Handle
    regs = dos_interrupt(#21,regs)
    if AND(regs[REG_FLAGS],1) then
	return 0
    end if
    return 1
end function -- DosClose()

global function BlockRead(integer Handle, atom buffer, atom NbBytes)
-- Read a given number of bytes from a file open with DosOpen() to a buffer 
-- if success return Number of bytes actually read else return -1
-- parameters:
--    Handle -> the file Handle returned by DosOpen()
--    buffer -> is a pointer returned byt allocate_low() where the data will be
--              read.
--    NbBytes -> is the number of bytes to read.
 
sequence regs
regs = repeat(0,10)
regs[REG_AX] = #3F00
regs[REG_BX] = Handle
regs[REG_CX] = NbBytes
regs[REG_DS] = floor(buffer/16)
regs[REG_DX] = remainder(buffer,16)
regs = dos_interrupt(#21,regs)
if AND(regs[REG_FLAGS],1) then
    return -1 -- failed to read
end if
return regs[REG_AX]
end function --BlockRead()

global function BlockWrite(integer Handle, atom buffer, atom NbBytes)
-- Write to a file open with DosOpen() from a buffer.
-- if succes return number of bytes actually written else return 0
-- parameters: Handle -> handle returned by DosOpen()
--             buffer -> buffer pointer returned by allocate_low() where data
--                       to write is stored.
--             NbBytes -> Number of bytes to write

sequence regs
regs = repeat(0,10)
regs[REG_AX] = #4000
regs[REG_BX] = Handle
regs[REG_CX] = NbBytes
regs[REG_DS] = floor(buffer/16)
regs[REG_DX] = remainder(buffer,16)
regs = dos_interrupt(#21,regs)
if AND(regs[REG_FLAGS],1) then
    return -1 -- failed to read
end if
return regs[REG_AX]
end function -- BlockWrite()

global function DosSeek(integer Handle, integer Distance, integer Origin)
-- Move the file read/write pointer to specified position
-- return new pointer position or -1 if error.
-- parameters: Handle is DosOpen() handle
--             Distance is distance to move to
--             Origin from where Distance is added
--              Origin = 0 beginning of file + Distance
--              Origin = 1 current location + Distance
--              Origin = 2 end of file + Distance
sequence regs
regs = repeat(0,10)
regs[REG_AX] = #4200+Origin
regs[REG_BX] = Handle
regs[REG_CX] = floor(Distance/256)
regs[REG_DX] = remainder(Distance,256)
regs = dos_interrupt(#21,regs)
if AND(regs[REG_FLAGS],1) then
    return -1 -- failed
end if
return regs[REG_AX]
end function --DosSeek()

global function GetVector(byte VecNb)
-- get the pointer of a given interrupt vector VecNb
-- return sequence {segment,offset}
sequence r
    r = repeat(0,10)
    r[REG_AX] = #3500 + VecNb
    r = dos_interrupt(#21,r)
    return {r[REG_ES],r[REG_BX]}
end function -- GetVector()

global procedure SetVector(byte VecNb, integer pointer)
-- set a new interrupt vector.
-- pointer is the new ISR address and VecNb is then interrupt vector number.
-- from a linear address one get a normalised vector by: 
-- segment = floor(linear_address_vector/16) i.e. integer division
-- offset = remainder(linear_address_vector,16) i.e. modulo 16

sequence r
    r = repeat(0,10)
    r[REG_AX] = #2500 + VecNb
    r[REG_DS] = floor(pointer/16)
    r[REG_DX] = remainder(pointer,16)
    r = dos_interrupt(#21,r)
end procedure -- SetVector()



