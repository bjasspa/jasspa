-- include file to program DMA controllers.
-- by Jacques Deschenes, Baie-Comeau, P.Q., Canada, e-mail:desja@quebectel.com
-- Creation date: October 6th, 1996
-- Adapted from Peter Sprenger, DMALIB.C from his soundx library
-- ***** revision history ****
-- revision date: Oct 31th, 1996
-- Added to globals functions
--   ReadCurrentCount()   to read the current word count of a given dma channel
--   ReadCurrentAddress() to read current address of a given dma channel.
--   ResetDma()  -- to reset end of process flag of a channel.
--   modified SetDma() to reset status register on each call.
--
--  jan. 5th, 1997,  modification to ReadStatus()   
--  Jan. 20th, 1997, modification to ResetDma() and SetDma()
 
include ports.e
include bitwise.e
include machine.e
 
-- DMA i/o ports constants
constant DMA8_STATUS_REG  = #8   -- read status register
constant DMA8_REQUEST_REG = #9   -- write request bit
constant DMA8_MASK_REG    = #A   -- mask i/o port
constant DMA8_MODE_REG    = #B   -- mode i/o port
constant DMA8_FF_REG  =    #C    -- reset flipflop i/o port
 
constant DMA16_STATUS_REG  = #D0  -- read status
constant DMA16_REQUEST_REG = #D2  -- write request bit
constant DMA16_MASK_REG  = #D4    -- write mask bit
constant DMA16_MODE_REG  = #D6    -- write mode
constant DMA16_FF_REG    = #D8    -- reset flipflop
 
-- DMA mode constant
global constant 
    DMA_IN = 4,
    DMA_OUT = 8,
    DMA_SINGLE = #40,
    DMA_AUTO = #10,
    DMA_BLOCK = #80,
    DMA_DEMAND = 0,
    DMA_DECREMENT =#20
 
constant DISABLE = 4 -- to disable dma channel
 
constant ADDR_REG = 1, COUNT_REG= 2, PAGE_REG = 3 
 
constant DMA_CHANNELS = { -- I/O port for each DMA channel
    {0,1,#87},  -- channel 0  {adr,count,page}
    {2,3,#83},  -- channel 1
    {4,5,#81},  -- channel 2 not used by sound card.
    {6,7,#82},-- channel 3 
    {#C0,#C1,#8F},  -- channel 4 not used by sound card.
    {#C2,#C3,#8B}, -- channel 5
    {#C4,#C5,#89},  -- channel 6
    {#C6,#C7,#8A}   -- channel 7
    } -- DMA_CHANNELS
 
-------------------------------------------------------------------------------
 
global procedure SetDma(atom buffer, integer size,
                        integer channel, integer mode)
-- Set dma controller for transfert
-- parameters:
--      buffer -> pointer to dma buffer (low memory linear address)
--      size   -> number of bytes to transfert
--      channel -> which dma channel to use (0-7)
--      mode -> mode of transfert:  [DMA_SINGLE | DMA_BLOCK | DMA_DEMAND]+
--                                  [DAM_IN|DMA_OUT]+
--                                  [DMA_AUTO]+
--                                  [DMA_DECREMENT]
 
integer page, adr, Mask, Cmd, Flip, adr_r, cnt_r, page_r, junk, Request, Status
 
  page = floor(buffer/#10000)
  adr = remainder(buffer,#10000)
 
  adr_r = DMA_CHANNELS[channel+1][ADDR_REG]
  cnt_r = DMA_CHANNELS[channel+1][COUNT_REG]
  page_r = DMA_CHANNELS[channel+1][PAGE_REG]
 
  if channel>3 then  --  channel > 3 are 16 bits 
    Mask = DMA16_MASK_REG
    Cmd = DMA16_MODE_REG
    Request = DMA16_REQUEST_REG
    Flip = DMA16_FF_REG 
    Status = DMA16_STATUS_REG
    channel = channel - 4   
    size = floor(size/2)  -- number of words to transfert
    if not size then size = size + 1 end if
   else
    Mask = DMA8_MASK_REG
    Cmd = DMA8_MODE_REG
    Request = DMA8_REQUEST_REG
    Flip = DMA8_FF_REG
    Status = DMA8_STATUS_REG
  end if
  Output(channel+DISABLE,Mask)  -- set mask bit
  Output(channel,Request) --clear request bit
  junk = Input(Status) -- reset status bits
  Output(channel+mode,Cmd)  -- set mode
  Output(0,Flip)  -- reset flipflop
  OutputW(size-1,cnt_r)
  Output(0,Flip)  -- reset flipflop
  OutputW(adr,adr_r)
  Output(page,page_r)
  Output(channel,Mask) -- clear mask bit to enable dma
end procedure -- SetDma()
 
-------------------------------------------------------------------------------
 
global procedure ResetDma(integer Channel)
-- reset EOP bit in status register of dma controller
-- set mask bit
-- clear request bit
 
integer Junk, Status, Request, Mask
 if Channel < 4 then
    Status = DMA8_STATUS_REG
    Request = DMA8_REQUEST_REG
    Mask = DMA8_MASK_REG
 else
    Status = DMA16_STATUS_REG
    Request = DMA16_REQUEST_REG
    Mask = DMA16_MASK_REG
    Channel = Channel - 4
 end if
 Junk = Input(Status)  -- disable EOP bit in status register
 Output(Channel+DISABLE,Mask) -- set mask bit
 Output(Channel,Request) -- clear request bit
end procedure -- ResetDma()
 
-------------------------------------------------------------------------------
 
global function ReadCurrentCount(integer channel)
-- read current count register of given dma channel
integer flip, cnt_r
   cnt_r = DMA_CHANNELS[channel+1][COUNT_REG]
   if channel > 3 then
      flip = DMA16_FF_REG
   else
      flip = DMA8_FF_REG
   end if
   Output(0,flip)
   return InputW(cnt_r)
end function -- ReadCurrentCount()
--------------------------------------------------------------------------------
 
global function ReadCurrentAddress(integer channel)
-- read current address of given dma channel
integer flip, adr_r   
    adr_r = DMA_CHANNELS[channel+1][ADDR_REG]
    if channel > 3 then
        flip = DMA16_FF_REG
    else
        flip = DMA8_FF_REG
    end if
    Output(0,flip)
    return InputW(adr_r)
end function -- ReadCurrentAddress()
 
-------------------------------------------------------------------------------
 
global function ReadStatus(integer channel)
-- read status of dma chip to know if terminal count as been reached.
-- if terminal count bit (transfert is done)
integer Status
sequence StatusBits
  
  if channel < 4 then
    Status = DMA8_STATUS_REG
  else
    Status = DMA16_STATUS_REG
    channel = channel - 4
  end if
  StatusBits = int_to_bits(Input(Status),8)
  return StatusBits[channel+1]
end function -- ReadStatus()
 
-------------------------------------------------------------------------------
 
global atom WinVer  -- windows version
constant MULTIPLEX = #2F
 
function WindowIsRunning()
-- check if euphoria program is running under window
-- this information is needed by AllocateDmaBuffer()
sequence r
integer r_AL
  r = repeat(0,10)
  r[REG_AX] = #1600
  r = dos_interrupt(MULTIPLEX,r)
  r_AL = remainder(r[REG_AX],256)
  if r_AL = 1 or r_AL = 255 then
    WinVer = 2.00
    return 1
  elsif  r_AL = 0 or r_AL = #80 then
    r[REG_AX] = #4680
    r = dos_interrupt(MULTIPLEX,r)
    r_AL = remainder(r[REG_AX],256)
    if r_AL = #80 then
        WinVer = 0
        return 0 -- windows not running
    else
        WinVer = 3.00
        return 1
    end if
  else --windows is running in extended mode, version in ax, al=major,ah=minor  
    WinVer = r_AL + floor(r[REG_AX]/256)/100
    return 1
  end if    
end function  -- WindowIsRunning()
 
-------------------------------------------------------------------------------
 
function BufferGood(atom buffer, integer size)
-- check if buffer is all on same page. return 1 if good else 0
-- DMA chip work with 64KB page
atom page
    page = floor(buffer/#10000)
    if buffer+size <= #10000*(page+1) then
        return 1
    end if
    return 0
end function -- BufferGood()
 
-------------------------------------------------------------------------------
 
--        *******    AllocateDMABuffer()   **********************
-- DMA chip need a buffer that reside entirely on de same 64KB page.
-- if not possible to allocate such a buffer, return 0 else return buffer 
-- address
-- about memory allocation:
-- The algorythm used here try to force allocation to take place at beginning
-- of a page by allocating temporary blocks of a size that would just fill the
-- memory up to next page.
-- for example if we need a 30000 bytes buffer and the first buffer allocated
-- cross the page boundary by say 10000 bytes then we release that bad buffer
-- and allocate a 20000 - 64 (size of each buffer header) bytes temporay 
-- buffer to fill memory up to next page.  Then we reallocate a buffer of
-- 30000 bytes.  If the gap fill buffer was allocated at the same address as
-- the first buffer then we are certain that the second buffer will begin at
-- at the next page boundary and consequently it will fit entirely inside the
-- page.  But the problem is that it's not certain that the gap fill block will
-- be allocate at the first buffer address because the euphoria memory manager
-- allocate the first that fit memory block.  As gap fill block is smaller than
-- the needed buffer (10000 bytes instead of 30000) it can be allocate at a
-- lower memory address.  But by repeating the process the algorythm will
-- eventualy find a good buffer if one exist.  
-- When a good buffer is found the temporary gap fill buffers are released
-- and then good buffer is returned to the caller.
 
 
global function AllocateDMABuffer(integer size)
sequence TmpBufs -- to keep "page filler" memory blocks
atom buffer,temp, page, TmpSize
integer gap
    if WindowIsRunning() then
        gap = 48 -- don't ask me why. I just found it by experimenting.
    else         -- I only knows that without that gap it doesn't work.
        gap = 64
    end if  
   TmpBufs = {}
   buffer = allocate_low(size)
   while buffer > 0 and not BufferGood(buffer,size) do
     page = floor(buffer/#10000)
     free_low(buffer)
     TmpSize = #10000*(page+1)-buffer - gap -- try to fill this page to end.
     if TmpSize < 0 then
        TmpSize = 0
     end if
     temp = allocate_low(TmpSize)
     if temp = 0 then
       exit
     end if
     TmpBufs = TmpBufs & temp
     buffer = allocate_low(size)   -- allocate a new buffer hoping it will be at
                                   -- page boundary 
   end while
   for i = 1 to length(TmpBufs) do
      free_low(TmpBufs[i])
   end for
   return buffer
end function -- AllocateDMABuffer()
 
 
