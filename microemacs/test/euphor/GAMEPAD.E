-- NAME: GAMEPAD.E
-- OBJECT: use keyboard special keys as a game pad.
-- CREATION DATE: Dec. 8th, 1996
-- BY: Jacques Deschenes, Baie-Comeau, Canada,  e-mail:Desja@quebectel.com
-- RATIONALE: the problem with using the keyboard as a game pad is that one
--   can't press 2 keys at the same time, because the keyboard service routine
--   only track one key. If the player hold down a key and then push a second
--   key the Interrupt service routine doesn't see the first key any more. The
--  player have to release it and push it again for the ISR to detect it.
--  Possible solutions:
--   1) rewrite keyboard ISR. Best solution if you are system programmer.
--   2) on the keyboard there is special keys that the service routine process
--      differently.  Those keys are flagged has long as they are holded down.
--      The player can hold down 2 or 3 of those keys without loosing tracking.
--      The 9 flagged keys are:
--      RIGHT SHIFT
--      LEFT SHIFT
--      RIGHT CONTROL
--      LEFT CONTROL
--      RIGHT ALT
--      LEFT ALT
--      CAPS LOCK
--      NUM LOCK
--      SCROLL LOCK
-- For their position on the keyboard the first 6 are the best to use for game.
--
-- This include file implement solution 2.
-- Another problem arise from that solution: The key code returned by normal
-- key is modified by the control and alt keys.  So if the game use normal key
-- like spacebar the get_key() will return different value depending if the
-- player is holding down control, alt or control-alt at the same time he press
-- spacebar.
-- A partial solution to that problem is to read the scan code instead of the 
-- key code as scan codes of alpha keys are not modified by the specials keys.
-- (but arrows and functions keys scan codes are modified)
--
-- This include file export only 3 functions:
-- GetKeyPad()  which return a sequence containing the state of all specials
--              keys.
-- KeyPressed() return 1 if a key is pressed.
-- GetScanCode() which return the scan code or normal key if one is pressed.

include machine.e

--  index constants to access keypad sequence returned by GetKeyPad()
global constant siR_SHIFT = 1,
		siL_SHIFT = 2,
		siL_CTRL  = 3,
		siL_ALT   = 4,
		siR_CTRL  = 5,
		siR_ALT   = 6,
		siSCR_LOCK  = 7,
		siNUM_LOCK = 8,
		siCAP_LOCK = 9
	 
global function GetKeyPad()
-- return keyboard flags
-- output register AH containt AT flags and AL containt XT flags
sequence r, at, xt
  r = repeat(0,10)
  r[REG_AX] = #1200
  r = dos_interrupt(#16,r)
  at = int_to_bits(floor(r[REG_AX]/256),8)
  xt = int_to_bits(remainder(r[REG_AX],256),8)
  return xt[1..2]&at[1..7]
end function -- GetKeyPad()

global function KeyPressed()
-- check if a key is present in keyboard buffer
sequence r
  r = repeat(0,10)
  r[REG_AX] = #1100
  r = dos_interrupt(#16,r)
  return remainder(r[REG_FLAGS],8) < 4 -- key present if zero flag = 0
end function --KeyPressed()


--global function GetScanCode()
--sequence r
    --if KeyPressed() then
	--r = repeat(0,10)
	--r[REG_AX] = #1000
	--r = dos_interrupt(#16,r)
	--return floor(r[REG_AX]/256)
    --end if
    --return 0
--end function -- GetScanCode()

