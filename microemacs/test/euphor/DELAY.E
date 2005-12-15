-- delay.e
-- Creation date: September 20th, 1996
-- By: Jacques Deschenes, Baie-Comeau, PQ, Canada, e-mail desja@quebectel.com
--
-- PC's timer chip (Intel 8253) contain 3 16 bits counters. Used by the system
-- for intervals timing.
-- timer 0 of this chip is running at a frequency of 1.193180MHz
-- As timer 0 is a 16 bits counter it takes about .054 seconds to cycle the
-- counter. (an interrupt is generate at this interval)
-- delay read the counter of timer 0 and base it's interval on the count.
-- This timer chip uses 4 I/O ports:
--      control register at #43
--      timer 0 count read/write at #40
--      timer 1 count read/write at #41
--      timer 2 count read/write at #42

include ports.e

constant  TMR_CTRL = #43,  TMR0_COUNT = #40

global procedure delay(atom sec)
 -- parameter:  sec is delay in seconds
 -- The timer 0 runs in mode 3, in which it counts down by twos,
 -- so delay(1193180) will only delay half a second, therefore one should
 -- double that number:
 -- clocks = sec * 2386360
 -- theorical resolution is .41 microsecond
 -- practical resolution is reduce due to overhead and depend on computer speed
 
   atom clocks,elapsed, last, next, ncopy
   clocks = sec*2386360
   -- Read the counter value.  
   Output(0,TMR_CTRL)                              -- latch timer 0 counter 
   last=InputW(TMR0_COUNT)                            -- read count
   elapsed = 0
   while elapsed < clocks do
      -- Read the counter value.  
      Output(0,TMR_CTRL)                             
      next=InputW(TMR0_COUNT)                          
      ncopy = next  
      next = last - next
      if next < 0 then
	next = next + 65536
      end if
      elapsed = elapsed + next  -- add to total elapsed clock pulses  
      last = ncopy 
   end while -- elapsed<clocks
end procedure --delay()

