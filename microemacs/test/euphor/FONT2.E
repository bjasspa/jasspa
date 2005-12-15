-- font.e
-- Jiri Babor
-- j.babor@gns.cri.nz

-- Version 2.00   96-10-01
-- Version 2.10   96-12-30
--    frprint() - right adjusted print added
-- Version 2.20   97-01-31
--    global variable fa (text angle) added
--    capability to print at any angle using fchar() & fprint() added
--       underline flag is ignored when text angle != zero
--    fci() & fsi() funtions added: (indirect) printing into image sequences
--       text angle, shadow & underline flags are ignored by these routines
--    ffont() and burp() added
--    fgets() now has optional max string length limit - new meaning of fp[4]
--    notion of vertical justification abandoned - now byte 5 also reserved
--    fheight() now obsolete - replaced by the last parameter of ffont()
--    change of defaults:  fpc from 7 to -1 ... paper color now detected
--                         fsc from 8 to  0 ... black shadows, more common
-- Version 2.21   97-02-05
--    now all ascii values less than 256 but outside the font range are simply
--       ignored - except zero (underline toggle) in fchar() based routines
--    flength() corrected to take into account only chars within the font span
--       all references to baseline values removed
--    cosmetic changes
-- Version 2.21s  97-02-16
--    special version for Michael Packard with 'safe' getkey in fgets

-- Font file format:
--    byte 1: ascii code of the first char      (a1)
--    byte 2: ascii code of the last char       (an)
--    byte 3: max width in bytes                (maxw)
--    byte 4: max height in pixels              (maxh)
--    byte 5: reserved                          (0)
--    byte 6: reserved                          (0)
--    next (an-a1+1) bytes: actual char width   (w)
--    next (an-a1+1) sets of maxw*maxh bytes: simple char bitmaps,
--                                            left-to-right, top-to-bottom

include image.e
include machine.e
include wildcard.e

integer
   cf             -- current font

global atom
   fa,            -- text angle, in degrees, clockwise, zero at 3 o'clock
   fx,            -- text pointer: left edge char space
   fy             -- text pointer: top  edge char space

global integer
   ftc,           -- text color
   fpc,           -- paper color
   fsc            -- shadow color

global sequence fp   -- font parameters

sequence
   Name,          -- font filenames
   Font,          -- char images
   maxW,          -- sequence of max font widths (in bytes)
   maxH,          -- sequence of max font heights
   W,             -- sequence of sequences of char widths
   Fc,            -- sequence of font first chars
   Lc             -- sequence of font last chars

type char(integer x)
  return x>=0
end type

procedure burp()
   atom tim
   sound(250)
   tim=time()
   while time()-tim<0.15 do
   end while
   sound(0)
end procedure

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
   integer n,fn,maxh,maxw,byte,x,y,fc,lc
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
      byte = getc(fn)
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
      Font = append(Font, font)
      W = append(W, w)
      maxW = maxW & maxw
      maxH = maxH & maxh
      cf = length(Font)
   end if
   return cf
end function  -- fload

global function ffont()
   -- returns a sequence with particulars of the currently selected font
   return {Name[cf],    -- font filename
	   Fc[cf],      -- first char ASCII code
	   Lc[cf],      -- last char ASCII code
	   maxW[cf],    -- max char width in bytes
	   maxH[cf]}    -- max char height in pixels
end function

global function flength(sequence s)
   -- calculates the length of string s in pixels
   integer l
   l = 0
   if length(s) then
      for i = 1 to length(s) do
	 if s[i]>=Fc[cf] and s[i]<=Lc[cf] then
	    l = l + W[cf][s[i]-Fc[cf]+1]+fp[5]
	 end if
      end for
   end if
   return l
end function   -- flength

global function fheight()
   -- fheight is now obsolete, retained for compatibility with the previous
   -- versions - replaced by the last element of sequence returned by ffont
   -- max font height
   return maxH[cf]
end function   -- fheight

global procedure fchar(char ch)
-- prints char ch in currently selected font
   atom a,ca,sa,x,y
   integer c,l,co
   sequence xs,xc,ys,yc
   if ch=0 then
      if fa=0 then
	 fp[6] = not fp[6]     -- toggle underlining
      end if
   elsif ch>255 then
      ftc=ch-256        -- new text color
   elsif ch>=Fc[cf] and ch<=Lc[cf] then
      c = ch-Fc[cf]+1
      l = length(Font[cf][c][1])
      if fa then        -- sloping text
	 a=0.0174533*fa -- degrees to radians
	 sa=sin(a)
	 ca=cos(a)
	 if l then
	    xs=Font[cf][c][1]*sa
	    xc=Font[cf][c][1]*ca
	    ys=Font[cf][c][2]*sa
	    yc=Font[cf][c][2]*ca
	    if fp[1] and (fp[2] or fp[3]) then  -- show shadows
	       co=fsc
	       x=fx+fp[2]
	       y=fy+fp[3]
	       for i = 1 to l do
		  pixel(co,{xc[i]-ys[i]+x,yc[i]+xs[i]+y})
	       end for
	    end if
	    for i = 1 to l do
	       pixel(ftc,{xc[i]-ys[i]+fx,yc[i]+xs[i]+fy})
	    end for
	 end if
	 fx = fx + (W[cf][c]+fp[5])*ca
	 fy = fy + (W[cf][c]+fp[5])*sa
      else
	 if fp[1] and (fp[2] or fp[3]) then     -- show shadows
	    co=fsc
	    x=fx+fp[2]
	    y=fy+fp[3]
	    if l>0 then
	       for i = 1 to l do
		  pixel(co,{Font[cf][c][1][i]+x,Font[cf][c][2][i]+y})
	       end for
	    end if
	    if fp[6] and fp[8]>0 then           -- underline
	       for j=0 to fp[8]-1 do
		  for i=0 to W[cf][c]+fp[5]-1 do
		     pixel(co,{x+i,y+maxH[cf]+fp[7]+j})
		  end for
	       end for
	    end if
	 end if
	 if l>0 then
	    for i = 1 to l do
	       pixel(ftc,{Font[cf][c][1][i]+fx,Font[cf][c][2][i]+fy})
	    end for
	 end if
	 if fp[6] and fp[8]>0 then              -- underline
	    for j=0 to fp[8]-1 do
	       for i=0 to W[cf][c]+fp[5]-1 do
		  pixel(ftc,{fx+i,fy+maxH[cf]+fp[7]+j})
	       end for
	    end for
	 end if
	 fx = fx + W[cf][c]+fp[5]
      end if
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

global procedure fcprint(atom x1, atom x2, sequence s)
   -- print string s centred between x1 and x2
   atom a
   a=fa  -- store text angle
   fa=0  -- let's do it only when horizontal ...
   if length(s) then
      fx = (x1+x2-flength(s))/2
      for i = 1 to length(s) do
	 fchar(s[i])
      end for
   end if
   fa=a        -- restore text angle variable
end procedure  -- fcprint

global procedure frprint(sequence s)
   -- right adjusted print
   atom a
   a=fa  -- store text angle
   fa=0  -- let's do it only when horizontal ...
   fx=fx-flength(s)
   fprint(s)
   fa=a        -- restore text angle variable
end procedure

global function fci(char ch)  -- returns char image sequence
   integer h,w,c,l
   sequence ci
   h=maxH[cf]
   if ch>255 then
      ftc=ch-256              -- new text color - on the fly
      ci=repeat({},h)
   elsif ch>=Fc[cf] and ch<=Lc[cf] then
      c = ch-Fc[cf]+1
      w=W[cf][c]+fp[5]
      if w then
	 if fpc<0 then        -- use screen at text pointer as char background
	    ci=save_image({fx,fy},{fx+w-1,fy+h-1})
	    fx=fx+w
	 else                 -- use current paper color as char background
	    ci=repeat(repeat(fpc,w),h)
	 end if
	 l = length(Font[cf][c][1])
	 if l>0 then
	    for i = 1 to l do
	       ci[Font[cf][c][2][i]+1][Font[cf][c][1][i]+1]=ftc
	    end for
	 end if
      else
	 ci=repeat({},h)      -- nil char width - return empty sequence
      end if
   else
      ci=repeat({},h)
   end if
   return ci
end function

global function fsi(sequence s)  -- returns string image sequence
   integer h,l
   sequence ci,si
   l=length(s)
   if l then
      si=fci(s[1])
      if l>1 then
	 h=length(si)
	 for i=2 to l do
	    ci=fci(s[i])
	    for j=1 to h do
	       si[j]=si[j]&ci[j]
	    end for
	 end for
      end if
   else
      si={}
   end if
   return si
end function

constant
   nextkey=#41A,     -- bios var - contains offset address of next key in buffer
   lastkey=#41C      -- bios var - contains offset address of last key in buffer

function getkey()
   integer nkey
   if peek(nextkey)=peek(lastkey) then    -- no key waiting
      return -1
   else
      nkey=peek(#400+peek(nextkey))       -- ascii code
      poke(lastkey,peek(nextkey))         -- empty keyboard buffer
      return nkey
   end if
end function

global function fgets(sequence s)
   -- screen input of a string
   -- it sort of works for any reasonable combination of shadow & underline
   -- parameters, except that backspace eats into underline shadow of preceding
   -- char - solution (short of re-printing char): do NOT use shadows AND
   -- underline AND backspace at same time
   integer key,c,dx,dy,pc
   atom a,t
   a=fa  -- store text angle
   fa=0  -- let's do it only when horizontal ...
   if fpc<0 then pc=get_pixel({fx,fy})
   else pc=fpc end if
   fprint(s)
   while 1 do
      -- blinking cursor
      t = time()
      draw_line(ftc,{{fx,fy},{fx,fy+maxH[cf]-1}})
      c = pc
      while 1 do
	 if time()-t > 0.3 then
	    t = time()
	    draw_line(c,{{fx,fy},{fx,fy+maxH[cf]-1}})
	    if c=pc then
	       c=ftc
	    else
	       c=pc
	    end if
	 end if
	 --key = get_key()
	 --if key != -1 then
	 key = getkey()
	 if key>0 then
	    -- erase current cursor if necessary
	    if c=pc then
	       draw_line(c,{{fx,fy},{fx,fy+maxH[cf]-1}})
	    end if
	    exit
	 end if
      end while
      if key=13 then    -- enter
	 exit
      elsif key>=32 and key<127 and length(s)<8 then
	 if flength(s&key)<=fp[4] then
	    fchar(key)
	    s = s&key
	 else
	    burp()
	 end if
      elsif key=8 then  -- backspace
	 if length(s) then
	    -- shift back one char
	    fx = fx-flength(s[length(s)..length(s)])
	    -- and erase the last char
	    dx=W[cf][s[length(s)]-Fc[cf]+1]+fp[5]-1
	    dy=maxH[cf]-1
	    if fp[1] then
	       dx=dx+fp[2]
	       dy=dy+fp[3]
	    end if
	    if fp[6] then
	       dy=dy+fp[7]+fp[8]
	    end if
	    polygon(pc,1,{{fx,fy},{fx+dx,fy},{fx+dx,fy+dy},{fx,fy+dy}})
	    s = s[1..length(s)-1]
	 else
	    burp()
	 end if
      else
	 burp()
      end if
   end while
   fp[4]=1024  -- reset max string length to default
   fa=a        -- restore text angle variable
   return s
end function   -- fgets

-- init variables
Font={}
Name={}
Fc={}
Lc={}
maxW={}
maxH={}
W={}

-- initial defaults
fa=0
fx=0
fy=0
fpc=-1
ftc=15
fsc=0

-- selectable global parameters
fp = {0,       -- 1: shadow flag
      1,       -- 2: shadow offset: right
      1,       -- 3: shadow offset: down
   1024,       -- 4: max sequence length (in pixels) returned by fgets()
	       -- it is automatically reset to 1024 after each use of fgets()
      0,       -- 5: extra kerning space width
      0,       -- 6: underline flag
      0,       -- 7: extra underline drop
      1}       -- 8: underline thickness
