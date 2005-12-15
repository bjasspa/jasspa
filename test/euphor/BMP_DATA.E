
 --
 -- This file contains Euphoria code for the
 -- function 'get_bmp_data' which, when given
 -- the filename of a bitmap, opens the file,
 -- reads in the palette, and returns it along
 -- with the width and height of the image.
 --

 --
 -- Note that most of the code is from the
 -- Euphoria IMAGE.E include file
 --

 ----------

include graphics.e

 ----------

constant BMPFILEHDRSIZE = 14
constant OLDHDRSIZE = 12, NEWHDRSIZE = 40
constant EOF = -1

-- error codes returned by read_bitmap(), save_bitmap() and save_screen()
       constant BMP_OPEN_FAILED = 1,
		BMP_UNEXPECTED_EOF = 2,
		BMP_UNSUPPORTED_FORMAT = 3
	 
integer fn, error_code

function get_word()
-- read 2 bytes
    integer lower, upper
    
    lower = getc(fn)
    upper = getc(fn)
    if upper = EOF then
	error_code = BMP_UNEXPECTED_EOF
    end if
    return upper * 256 + lower
end function

function get_dword()
-- read 4 bytes
    integer lower, upper
    
    lower = get_word()
    upper = get_word()
    return upper * 65536 + lower
end function

function get_rgb(integer set_size)
-- get red, green, blue palette values
    integer red, green, blue
    
    blue = getc(fn)
    green = getc(fn)
    red = getc(fn)
    if set_size = 4 then
	if getc(fn) then
	end if
    end if
    return {red, green, blue}
end function

function get_rgb_block(integer num_dwords, integer set_size)
-- reads palette 
    sequence s

    s = {}
    for i = 1 to num_dwords do
	s = append(s, get_rgb(set_size))
    end for
    if s[length(s)][3] = EOF then
	error_code = BMP_UNEXPECTED_EOF
    end if
    return s
end function

function row_bytes(atom BitCount, atom Width)
-- number of bytes per row of pixel data
    return floor(((BitCount * Width) + 31) / 32) * 4
end function

global function get_bmp_data(sequence file_name)
-- read the palette from a bitmap (.BMP) file into
-- a sequence of color (R,G,B) codes, and returns
-- along with the bitmap's width and height
-- return {{color_code1, color_code2, etc...}, width, height}
    atom Size 
    integer Type, Xhot, Yhot, Planes, BitCount
    atom Width, Height, Compression, OffBits, SizeHeader, 
	 SizeImage, XPelsPerMeter, YPelsPerMeter, ClrUsed,
	 ClrImportant, NumColors
    sequence Palette

    error_code = 0
    fn = open(file_name, "rb")
    if fn = -1 then
	return BMP_OPEN_FAILED
    end if
    Type = get_word()
    Size = get_dword()
    Xhot = get_word()
    Yhot = get_word()
    OffBits = get_dword()
    SizeHeader = get_dword()

    if SizeHeader = NEWHDRSIZE then
	Width = get_dword()
	Height = get_dword()
	Planes = get_word()
	BitCount = get_word()
	Compression = get_dword()
	if Compression != 0 then
	    close(fn)
	    return BMP_UNSUPPORTED_FORMAT
	end if
	SizeImage = get_dword()
	XPelsPerMeter = get_dword()
	YPelsPerMeter = get_dword()
	ClrUsed = get_dword()
	ClrImportant = get_dword()
	NumColors = (OffBits - SizeHeader - BMPFILEHDRSIZE) / 4
	if NumColors < 2 or NumColors > 256 then
	    close(fn)
	    return BMP_UNSUPPORTED_FORMAT
	end if
	Palette = get_rgb_block(NumColors, 4) 
    
    elsif SizeHeader = OLDHDRSIZE then 
	Width = get_word()
	Height = get_word()
	Planes = get_word()
	BitCount = get_word()
	NumColors = (OffBits - SizeHeader - BMPFILEHDRSIZE) / 3
	SizeImage = row_bytes(BitCount, Width) * Height
	Palette = get_rgb_block(NumColors, 3) 
    else
	close(fn)
	return BMP_UNSUPPORTED_FORMAT
    end if
    if Planes != 1 or Height <= 0 or Width <= 0 then
	close(fn)
	return BMP_UNSUPPORTED_FORMAT
    end if
    --Bits = get_c_block(row_bytes(BitCount, Width) * Height)
    close(fn)
    --two_d_bits = unpack(Bits, BitCount, Width, Height)
    if error_code then
	return error_code 
    end if
    return {Palette, Width, Height}
end function


global function get_24bit_bmp_data(sequence file_name)
-- returns the bitmap's width and height
-- return {width, height}
    atom Size 
    integer Type, Xhot, Yhot, Planes, BitCount
    atom Width, Height, Compression, OffBits, SizeHeader, 
	 SizeImage, XPelsPerMeter, YPelsPerMeter, ClrUsed,
	 ClrImportant, NumColors

    error_code = 0
    fn = open(file_name, "rb")
    if fn = -1 then
	return BMP_OPEN_FAILED
    end if
    Type = get_word()
    Size = get_dword()
    Xhot = get_word()
    Yhot = get_word()
    OffBits = get_dword()
    SizeHeader = get_dword()

    if SizeHeader = NEWHDRSIZE then
	Width = get_dword()
	Height = get_dword()
	Planes = get_word()
	BitCount = get_word()
	Compression = get_dword()
	if Compression != 0 then
	    close(fn)
	    return BMP_UNSUPPORTED_FORMAT
	end if
	SizeImage = get_dword()
	XPelsPerMeter = get_dword()
	YPelsPerMeter = get_dword()
	ClrUsed = get_dword()
	ClrImportant = get_dword()
	NumColors = (OffBits - SizeHeader - BMPFILEHDRSIZE) / 4
	if NumColors != 0 and NumColors <= 256 then
	    close(fn)
	    return BMP_UNSUPPORTED_FORMAT
	end if
    
    elsif SizeHeader = OLDHDRSIZE then 
	Width = get_word()
	Height = get_word()
	Planes = get_word()
	BitCount = get_word()
	NumColors = (OffBits - SizeHeader - BMPFILEHDRSIZE) / 3
	SizeImage = row_bytes(BitCount, Width) * Height

    else
	close(fn)
	return BMP_UNSUPPORTED_FORMAT
    end if

    if Planes != 1 or Height <= 0 or Width <= 0 then
	close(fn)
	return BMP_UNSUPPORTED_FORMAT
    end if

    close(fn)

    if error_code then
	return error_code 
    end if

    return {Width, Height}
end function


global function get_color_depth(sequence file_name)
-- returns the bitmap's color depth (number of bits per color)
-- return idepth
    atom Size 
    integer Type, Xhot, Yhot, Planes, BitCount
    atom Width, Height, Compression, OffBits, SizeHeader, 
	 SizeImage, XPelsPerMeter, YPelsPerMeter, ClrUsed,
	 ClrImportant, NumColors

    error_code = 0
    fn = open(file_name, "rb")
    if fn = -1 then
	return BMP_OPEN_FAILED
    end if
    Type = get_word()
    Size = get_dword()
    Xhot = get_word()
    Yhot = get_word()
    OffBits = get_dword()
    SizeHeader = get_dword()

    if SizeHeader = NEWHDRSIZE then
	Width = get_dword()
	Height = get_dword()
	Planes = get_word()
	BitCount = get_word()
	Compression = get_dword()
	if Compression != 0 then
	    close(fn)
	    return BMP_UNSUPPORTED_FORMAT
	end if
	SizeImage = get_dword()
	XPelsPerMeter = get_dword()
	YPelsPerMeter = get_dword()
	ClrUsed = get_dword()
	ClrImportant = get_dword()
	NumColors = (OffBits - SizeHeader - BMPFILEHDRSIZE) / 4
	if NumColors != 0 and NumColors <= 256 then
	    close(fn)
	    return BMP_UNSUPPORTED_FORMAT
	end if
    
    elsif SizeHeader = OLDHDRSIZE then 
	Width = get_word()
	Height = get_word()
	Planes = get_word()
	BitCount = get_word()

    else
	close(fn)
	return BMP_UNSUPPORTED_FORMAT
    end if

    close(fn)

    if error_code then
	return error_code 
    end if

    return BitCount
end function

