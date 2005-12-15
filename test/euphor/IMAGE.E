-- Graphical Image routines

include graphics.e

constant BMPFILEHDRSIZE = 14
constant OLDHDRSIZE = 12, NEWHDRSIZE = 40
constant EOF = -1

-- error codes returned by read_bitmap()
global constant BMP_OPEN_FAILED = 1,
		BMP_UNEXPECTED_EOF = 2,
		BMP_UNSUPPORTED_FORMAT = 3
	 
constant M_PIXEL = 4,
	 M_GET_PIXEL = 21

integer fn, error_code

function get_word()
    integer lower, upper
    
    lower = getc(fn)
    upper = getc(fn)
    if upper = EOF then
	error_code = BMP_UNEXPECTED_EOF
    end if
    return upper * 256 + lower
end function

function get_dword()
    integer lower, upper
    
    lower = get_word()
    upper = get_word()
    return upper * 65536 + lower
end function

function get_c_block(integer num_bytes)
    sequence s
    
    s = repeat(0, num_bytes)
    for i = 1 to num_bytes do
	s[i] = getc(fn)
    end for
    if s[length(s)] = EOF then
	error_code = BMP_UNEXPECTED_EOF
    end if
    return s
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
    return floor(((BitCount * Width) + 31) / 32) * 4
end function

function unpack(sequence image, integer BitCount, integer Width, integer Height)
-- unpack the 1-d byte sequence into a 2-d sequence of pixels
    sequence pic_2d, row, bits
    integer bytes, next_byte, byte
    
    pic_2d = {}
    bytes = row_bytes(BitCount, Width)
    next_byte = 1
    for i = 1 to Height do
	row = {}
	if BitCount = 1 then
	    for j = 1 to bytes do
		byte = image[next_byte]
		next_byte = next_byte + 1
		bits = repeat(0, 8)
		for k = 8 to 1 by -1 do
		    bits[k] = remainder(byte, 2)
		    byte = floor(byte/2)
		end for
		row = row & bits
	    end for
	elsif BitCount = 2 then
	    for j = 1 to bytes do
		byte = image[next_byte]
		next_byte = next_byte + 1
		bits = repeat(0, 4)
		for k = 4 to 1 by -1 do
		    bits[k] = remainder(byte, 4)
		    byte = floor(byte/4)
		end for
		row = row & bits
	    end for
	elsif BitCount = 4 then
	    for j = 1 to bytes do
		byte = image[next_byte]
		row = append(row, floor(byte/16))
		row = append(row, remainder(byte, 16))
		next_byte = next_byte + 1
	    end for
	elsif BitCount = 8 then
	    row = row & image[next_byte..next_byte+bytes-1]
	    next_byte = next_byte + bytes
	else
	    error_code = BMP_UNSUPPORTED_FORMAT
	    exit
	end if
	pic_2d = prepend(pic_2d, row[1..Width])
    end for
    return pic_2d
end function

without warning
global function read_bitmap(sequence file_name)
-- read a bitmap (.BMP) file into a 2-d sequence of sequences (image)
-- return {palette,image}   
    atom Size 
    integer Type, Xhot, Yhot, Planes, BitCount
    atom Width, Height, Compression, OffBits, SizeHeader, 
	 SizeImage, XPelsPerMeter, YPelsPerMeter, ClrUsed,
	 ClrImportant, NumColors
    sequence Palette, Bits, two_d_bits

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
	    return BMP_UNSUPPORTED_FORMAT
	end if
	SizeImage = get_dword()
	XPelsPerMeter = get_dword()
	YPelsPerMeter = get_dword()
	ClrUsed = get_dword()
	ClrImportant = get_dword()
	NumColors = (OffBits - SizeHeader - BMPFILEHDRSIZE) / 4
	if NumColors < 2 or NumColors > 256 then
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
	return BMP_UNSUPPORTED_FORMAT
    end if
    if Planes != 1 or Height <= 0 or Width <= 0 then
	return BMP_UNSUPPORTED_FORMAT
    end if
    Bits = get_c_block(row_bytes(BitCount, Width) * Height)
    close(fn)
    two_d_bits = unpack(Bits, BitCount, Width, Height)
    if error_code then
	return error_code 
    end if
    return {Palette, two_d_bits}
end function
with warning

type graphics_point(sequence p)
    return length(p) = 2 and p[1] >= 0 and p[2] >= 0
end type

type text_point(sequence p)
    return length(p) = 2 and p[1] >= 1 and p[2] >= 1 
	   and p[1] <= 200 and p[2] <= 500 -- rough sanity check
end type

global procedure display_image(graphics_point xy, sequence pixels)
-- display a 2-d sequence of pixels at location xy
-- N.B. coordinates are {x, y} with {0,0} at top left of screen
-- and x values increasing towards the right, 
-- and y values increasing towards the bottom of the screen
    for i = 1 to length(pixels) do
	machine_proc(M_PIXEL, {pixels[i], xy})
	xy[2] = xy[2] + 1
    end for
end procedure

global function save_image(graphics_point top_left, graphics_point bottom_right)
-- Save a rectangular region on a graphics screen,
-- given the {x, y} coordinates of the top-left and bottom-right 
-- corner pixels. The result is a 2-d sequence of pixels suitable 
-- for use in display_image() above.
    integer x, width
    sequence save
    
    x = top_left[1]
    width = bottom_right[1] - x + 1
    save = {}
    for y = top_left[2] to bottom_right[2] do
	save = append(save, machine_func(M_GET_PIXEL, {x, y, width}))
    end for
    return save
end function

constant COLOR_TEXT_MEMORY = #B8000,
	  MONO_TEXT_MEMORY = #B0000

constant M_GET_DISPLAY_PAGE = 28,
	 M_SET_DISPLAY_PAGE = 29,
	 M_GET_ACTIVE_PAGE = 30,
	 M_SET_ACTIVE_PAGE = 31

constant BYTES_PER_CHAR = 2

type page_number(integer p)
    return p >= 0 and p <= 7
end type

global function get_display_page()
-- return current page# mapped to the monitor   
    return machine_func(M_GET_DISPLAY_PAGE, 0)
end function

global procedure set_display_page(page_number page)
-- select a page to be displayed
    machine_proc(M_SET_DISPLAY_PAGE, page)
end procedure

global function get_active_page()
-- return current page# that screen output is sent to
    return machine_func(M_GET_ACTIVE_PAGE, 0)
end function

global procedure set_active_page(page_number page)
-- select a page for screen output
    machine_proc(M_SET_ACTIVE_PAGE, page)
end procedure

global procedure display_text_image(text_point xy, sequence text)
-- Display a text image at line xy[1], column xy[2] in any text mode.
-- N.B. coordinates are {line, column} with {1,1} at the top left of screen
-- Displays to the active text page. Image must fit on screen.
    atom screen_memory, scr_addr
    integer screen_width, extra_col2, extra_lines
    sequence vc
    
    vc = video_config()
    if vc[VC_MODE] = 7 then
	screen_memory = MONO_TEXT_MEMORY
    else
	screen_memory = COLOR_TEXT_MEMORY
    end if
    screen_width = vc[VC_COLUMNS]
    if screen_width = 40 then
	screen_memory = screen_memory + get_active_page() * 2048
    else
	screen_memory = screen_memory + get_active_page() * 4096
    end if
    
    if xy[1] < 1 or xy[2] < 1 then
	return -- bad starting point
    end if
    extra_lines = vc[VC_LINES] - xy[1] + 1 
    if length(text) > extra_lines then
	if extra_lines <= 0 then
	    return -- nothing to display
	end if
	text = text[1..extra_lines] -- truncate
    end if
    scr_addr = screen_memory + (xy[1]-1) * screen_width * BYTES_PER_CHAR
			     + (xy[2]-1) * BYTES_PER_CHAR
    extra_col2 = 2 * (vc[VC_COLUMNS] - xy[2] + 1) 
    for row = 1 to length(text) do
	if length(text[row]) > extra_col2 then
	    if extra_col2 <= 0 then
		return -- nothing to display
	    end if
	    text[row] = text[row][1..extra_col2] -- truncate
	end if
	poke(scr_addr, text[row])
	scr_addr = scr_addr + screen_width * BYTES_PER_CHAR
    end for
end procedure

global function save_text_image(text_point top_left, text_point bottom_right)
-- Copy a rectangular block of text out of screen memory,
-- given the coordinates of the top-left and bottom-right corners.
-- Reads from the active text page.
    sequence image, row_chars, vc
    atom scr_addr, screen_memory
    integer screen_width, image_width
    
    vc = video_config()
    if vc[VC_MODE] = 7 then
	screen_memory = MONO_TEXT_MEMORY
    else
	screen_memory = COLOR_TEXT_MEMORY
    end if
    screen_width = vc[VC_COLUMNS]
    if screen_width = 40 then
	screen_memory = screen_memory + get_active_page() * 2048
    else
	screen_memory = screen_memory + get_active_page() * 4096
    end if
    scr_addr = screen_memory + 
	       (top_left[1]-1) * screen_width * BYTES_PER_CHAR + 
	       (top_left[2]-1) * BYTES_PER_CHAR
    image = {}
    image_width = bottom_right[2] - top_left[2] + 1
    for row = top_left[1] to bottom_right[1] do
	row_chars = peek({scr_addr, image_width*BYTES_PER_CHAR})
	image = append(image, row_chars)
	scr_addr = scr_addr + screen_width * BYTES_PER_CHAR
    end for
    return image
end function

