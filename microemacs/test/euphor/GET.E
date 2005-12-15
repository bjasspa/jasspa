		------------------------------------
		-- Input and Conversion Routines: --
		-- get()                          --
		-- value()                        --
		-- wait_key()                     --
		------------------------------------

constant M_WAIT_KEY = 26

constant DIGITS = "0123456789",
	 HEX_DIGITS = DIGITS & "ABCDEF",
	 START_NUMERIC = DIGITS & "-+.#"

global function wait_key()
-- Get the next key pressed by the user.
-- Wait until a key is pressed.
    return machine_func(M_WAIT_KEY, 0)
end function


-- error status values returned from get() and value():
global constant GET_SUCCESS = 0,
		GET_EOF = -1,
		GET_FAIL = 1

constant UNDEFINED_CHAR = -2

constant TRUE = 1

type natural(integer x)
    return x >= 0
end type

type char(integer x)
    return x >= UNDEFINED_CHAR and x <= 255
end type

natural input_file  -- file to be read from

object input_string -- string to be read from
natural string_next

char ungot_char     -- can "unget" one char

function get_char()
-- read next logical char in input stream (either string or file)
    char next_char

    if ungot_char = UNDEFINED_CHAR then
	if sequence(input_string) then
	    if string_next <= length(input_string) then
		next_char = input_string[string_next]
		string_next = string_next + 1
		return next_char
	    else
		return GET_EOF
	    end if
	else    
	    return getc(input_file)
	end if
    else
	next_char = ungot_char
	ungot_char = UNDEFINED_CHAR
	return next_char
    end if
end function

procedure unget(char c)
-- "unget" a character - push it back on the input stream
    ungot_char = c
end procedure

procedure skip_blanks()
-- skip white space
    char c

    while TRUE do
	c = get_char()
	if not find(c, " \t\n") then
	    exit
	end if
    end while
    unget(c)
end procedure

constant ESCAPE_CHARS = "nt'\"\\r",
	 ESCAPED_CHARS = "\n\t'\"\\\r"

function escape_char(char c)
-- return escape character
    natural i

    i = find(c, ESCAPE_CHARS)
    if i = 0 then
	return GET_FAIL
    else
	return ESCAPED_CHARS[i]
    end if
end function

function get_qchar()
-- get a single-quoted character

    char c

    c = get_char()
    if c = '\\' then
	c = escape_char(get_char())
	if c = GET_FAIL then
	    return {GET_FAIL, 0}
	end if
    end if
    if get_char() != '\'' then
	return {GET_FAIL, 0}
    else
	return {GET_SUCCESS, c}
    end if
end function


function get_string()
-- get a double-quoted character string
    sequence text
    char c

    text = ""
    while TRUE do
	c = get_char()
	if c = GET_EOF or c = '\n' then
	    return {GET_FAIL, 0}
	end if
	if c = '"' then
	    exit
	elsif c = '\\' then
	    c = escape_char(get_char())
	    if c = GET_FAIL then
		return {GET_FAIL, 0}
	    end if
	end if
	text = text & c
    end while
    return {GET_SUCCESS, text}
end function

type plus_or_minus(integer x)
    return x = -1 or x = +1
end type

function get_number()
-- read a number
    char c
    plus_or_minus sign, e_sign
    natural ndigits
    integer hex_digit
    atom mantissa, dec, e_mag, exponent

    sign = +1
    mantissa = 0
    e_sign = +1
    e_mag = 0
    ndigits = 0

    c = get_char()

    -- process sign
    if c = '-' then
	sign = -1
    elsif c != '+' then
	unget(c)
    end if

    -- get mantissa
    c = get_char()
    if c = '#' then
	-- process hex integer and return
	while TRUE do
	    c = get_char()
	    hex_digit = find(c, HEX_DIGITS)-1
	    if hex_digit >= 0 then
		ndigits = ndigits + 1
		mantissa = mantissa * 16 + hex_digit
	    else
		unget(c)
		if ndigits > 0 then
		    return {GET_SUCCESS, sign * mantissa}
		else
		    return {GET_FAIL, 0}
		end if
	    end if
	end while       
    end if
    -- decimal integer or floating point
    while c >= '0' and c <= '9' do
	ndigits = ndigits + 1
	mantissa = mantissa * 10 + (c - '0')
	c = get_char()
    end while
    if c = '.' then
	-- get fraction
	c = get_char()
	dec = 10
	while c >= '0' and c <= '9' do
	    ndigits = ndigits + 1
	    mantissa = mantissa + (c - '0') / dec
	    dec = dec * 10
	    c = get_char()
	end while
    end if
    if ndigits = 0 then
	return {GET_FAIL, 0}
    end if
    if c = 'e' or c = 'E' then
	-- get exponent sign
	c = get_char()
	if c = '-' then
	    e_sign = -1
	elsif c != '+' then
	    unget(c)
	end if
	-- get exponent magnitude 
	c = get_char()
	if c >= '0' and c <= '9' then
	    e_mag = c - '0'
	    c = get_char()
	    while c >= '0' and c <= '9' do
		e_mag = e_mag * 10 + c - '0'
		c = get_char()                          
	    end while
	    unget(c)
	else
	    return {GET_FAIL, 0} -- no exponent
	end if
    else
	unget(c)
    end if
    exponent = 1
    if e_sign >= 0 then
	for i = 1 to e_mag do
	    exponent = exponent * 10
	end for
    else
	for i = 1 to e_mag do
	    exponent = exponent * 0.1
	end for
    end if
    return {GET_SUCCESS, sign * mantissa * exponent}
end function

function Get()
-- read a Euphoria data object as a string of characters
-- and return {error_flag, value}
    char c
    sequence s, e

    skip_blanks()
    c = get_char()

    if find(c, START_NUMERIC) then
	unget(c)
	return get_number()

    elsif c = '{' then
	-- process a sequence
	s = {}
	while TRUE do
	    skip_blanks()
	    c = get_char()
	    if c = '}' then
		return {GET_SUCCESS, s}
	    else
		unget(c)
	    end if
	    e = Get()
	    if e[1] != GET_SUCCESS then
		return e
	    end if
	    s = append(s, e[2])
	    skip_blanks()
	    c = get_char()
	    if c = '}' then
		return {GET_SUCCESS, s}
	    elsif c != ',' then
		return {GET_FAIL, 0}
	    end if
	end while

    elsif c = '\"' then
	return get_string()

    elsif c = '\'' then
	return get_qchar()

    elsif c = -1 then
	return {GET_EOF, 0}

    else
	return {GET_FAIL, 0}

    end if
end function

global function get(integer file)
-- Read the string representation of a Euphoria object 
-- from a file. Convert to the value of the object.
-- Return {error_status, value}.
    input_file = file
    input_string = 0
    ungot_char = UNDEFINED_CHAR
    return Get()
end function

global function value(sequence string)
-- Read the representation of a Euphoria object
-- from a sequence of characters. Convert to the value of the object.
-- Return {error_status, value).
    input_string = string
    string_next = 1
    ungot_char = UNDEFINED_CHAR
    return Get()
end function

