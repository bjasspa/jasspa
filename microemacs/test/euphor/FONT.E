-- font.e
-- Jiri Babor
-- baborj@gns.cri.nz

-- Version 2.00   96-10-01

-- New format:
--    byte 1: ascii code of the first char      (a1)
--    byte 2: ascii code of the last char       (an)
--    byte 3: max width in bytes                (maxw)
--    byte 4: max height in pixels              (maxh)
--    byte 5: base line in pixels from top      (maxh-2)
--    byte 6: reserved                          (0)
--    next (an-a1+1) bytes: actual char width   (w)
--    next (an-a1+1) sets of maxw*maxh bytes:
--       first byte: char actual width in pixels
--       next maxw*maxh bytes: simple char bitmaps: left-to-right,
--                                                  top-to-bottom

include machine.e
include wildcard.e

integer
   cf             -- current font

global integer
   fx,            -- text pointer: left edge char space
   fy,            -- text pointer: top  edge char space
   ftc,           -- text color
   fpc,           -- paper color
   fsc            -- shadow color

global sequence fp   -- font parameters

sequence
   Name,          -- font filenames
   Font,          -- char images
   H,             -- sequence of max font heights
   W,             -- sequence of sequences of char widths
   Fc,            -- sequence of font first chars
   Lc,            -- sequence of font last chars
   Bl             -- sequence of baseline values

type char(integer x)
  return x>=0
end type

global procedure fselect(integer h)
   -- makes font with handle h current
   if h>0 and h<=length(Font) then
      cf = h
   end if
end procedure  -- fselect

function loaded(sequence n)
   integer i,f
   f = 0
   i = 0
   while i<length(Name) do
      i=i+1
      if compare(n,Name[i])=0 then
	 f = i
	 exit
      end if
   end while
   return f
end function   -- loaded

global function fload(sequence file_name)
   -- load font file into memory and make available for selection
   integer n,fn,maxh,maxw,byte,x,y,fc,lc,bl
   sequence bits,font,cs,w
   -- if font is already loaded, do not do it again, just select it
   file_name=lower(file_name)
   n = loaded(file_name)
   if n then
      cf = n
   else
      fn = open(file_name, "rb")
      if fn = -1 then
	puts(1, "Font load error: " & file_name & " not found !\n")
	abort(1)
      end if
      Name = append(Name, file_name)
      fc = getc(fn)
      lc = getc(fn)
      maxw = getc(fn)
      maxh = getc(fn)
      bl = getc(fn)
      byte = getc(fn)
      w = {}
      font = {}
      for i = fc to lc do
	 w = w & getc(fn)
      end for
      for i = fc to lc do
	 cs = {{},{}}
	 for j = 1 to maxw*maxh do
	    byte = getc(fn)
	    if byte>0 then
	       x = 8*remainder(j-1,maxw)
	       y = floor((j-1)/maxw)
	       bits = int_to_bits(byte,8)
	       for k = 1 to 8 do
		  if bits[k] = 1 then
		     cs[1] = append(cs[1], x+8-k)
		     cs[2] = append(cs[2], y)
		  end if
	       end for
	    end if
	 end for
	 font = append(font, cs)
      end for
      close(fn)
      Fc = append(Fc,fc)
      Lc = append(Lc,lc)
      Bl = append(Bl,bl)
      Font = append(Font, font)
      W = append(W, w)
      H = H & maxh
      cf = length(Font)
   end if
   return cf
end function  -- fload

global function flength(sequence s)
   -- calculates the length of string s in pixels
   integer l
   l = 0
   if length(s) > 0 then
      for i = 1 to length(s) do
	 if s[i]>=0 and s[i]<=126 then
	    l = l + W[cf][s[i]-Fc[cf]+1]+fp[5]
	 end if
      end for
   end if
   return l
end function   -- flength

global function fheight()
  -- max font height
  return H[cf]
end function   -- fheight

global procedure fchar(char ch)
-- prints char ch in currently selected font
   integer c,l,co,x,y
   if ch=0 then
      fp[6] = not fp[6]     -- toggle underlining
   elsif ch>255 then
      ftc=ch-256        -- new text color
   else
      c = ch-Fc[cf]+1
      l = length(Font[cf][c][1])
      if fp[1] and (fp[2] or fp[3]) then
	 co=fsc
	 x=fx+fp[2]
	 y=fy+fp[3]
	 if l>0 then
	    for i = 1 to l do
	       pixel(co,{Font[cf][c][1][i]+x,Font[cf][c][2][i]+y})
	    end for
	 end if
	 if fp[6] and fp[8]>0 then
	    for j=0 to fp[8]-1 do
	       for i=0 to W[cf][c]+fp[5]-1 do
		  pixel(co,{x+i,y+H[cf]+fp[7]+j})
	       end for
	    end for
	 end if
      end if
      if l>0 then
	 for i = 1 to l do
	    pixel(ftc,{Font[cf][c][1][i]+fx,Font[cf][c][2][i]+fy})
	 end for
      end if
      if fp[6] and fp[8]>0 then
	 for j=0 to fp[8]-1 do
	    for i=0 to W[cf][c]+fp[5]-1 do
	       pixel(ftc,{fx+i,fy+H[cf]+fp[7]+j})
	    end for
	 end for
      end if
      fx = fx + W[cf][c]+fp[5]
   end if
end procedure  -- fchar

global procedure fprint(sequence s)     -- print string s
-- prints string s in currently selected font
  if length(s) then
    for i = 1 to length(s) do
      fchar(s[i])
    end for
  end if
end procedure  -- fprint

global procedure fcprint(integer x1, integer x2, sequence s)
-- print string s centred between x1 and x2
  fx = floor((x1+x2-flength(s))/2)
  if length(s) then
    for i = 1 to length(s) do
      fchar(s[i])
    end for
  end if
end procedure  -- fcprint

global function fgets(sequence s)
   -- screen input of a string
   -- it sort of works for any reasonable combination of shadow & underline
   -- parameters, except that backspace eats into underline shadow of preceding
   -- char - solution (short of re-printing char): do NOT use shadows AND
   -- underline AND backspace at same time

   integer key,c,dx,dy
   atom t
   fprint(s)
   while 1 do
      -- blinking cursor
      t = time()
      draw_line(ftc,{{fx,fy},{fx,fy+fheight()-1}})
      c = fpc
      while 1 do
	 if time()-t > 0.3 then
	    t = time()
	    draw_line(c,{{fx,fy},{fx,fy+fheight()-1}})
	    if c=fpc then
	       c=ftc
	    else
	       c=fpc
	    end if
	 end if
	 key = get_key()
	 if key != -1 then
	    -- erase current cursor if necessary
	    if c=fpc then
	       draw_line(c,{{fx,fy},{fx,fy+fheight()-1}})
	    end if
	    exit
	 end if
      end while
      if key=13 then    -- enter
	 exit
      elsif key>=32 and key<127 and length(s)<8 then
	 fchar(key)
	 s = s & key
      elsif key=8 and length(s) then   -- backspace
	 -- shift back one char
	 fx = fx - flength(s[length(s)..length(s)])
	 -- and erase the last char
	 dx=W[cf][s[length(s)]-Fc[cf]+1]+fp[5]-1
	 dy=fheight()-1
	 if fp[1] then
	    dx=dx+fp[2]
	    dy=dy+fp[3]
	 end if
	 if fp[6] then
	    dy=dy+fp[7]+fp[8]
	 end if
	 polygon(fpc,1,{{fx,fy},{fx+dx,fy},{fx+dx,fy+dy},{fx,fy+dy}})
	 s = s[1..length(s)-1]
      end if
   end while
   return s
end function   -- fgets

-- init variables..
Font={}
Name={}
Fc={}
Lc={}
Bl={}
H={}
W={}

-- initial defaults
fx=0
fy=0
ftc=15
fpc=0
fsc=0

fp = {0,          -- shadow flag
      0,          -- shadow offset: right
      0,          -- shadow offset: down
      0,          -- vertical justification: top
		  -- centre & bottom justification not yet implemented, sorry!
      0,          -- extra kerning space width
      0,          -- underline flag
      0,          -- extra underline drop
      0}          -- underline thickness
