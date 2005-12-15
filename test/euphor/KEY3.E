constant
    KEY_BUFFER      = 1054,     -- keyboard buffer; circular queue
    FIRST_INDEX     = 1050,     -- index to first key in buffer
    LAST_INDEX      = 1052      -- index to last key in buffer

----------------------------------------------------------------------------
global function GetScanCode()

    integer scan_code, key_code

    -- return key currently being pressed
    -- return zero if no key is pressed
    -- buffer is empty if indexes point to each other

    if peek( FIRST_INDEX ) = peek( LAST_INDEX ) then
	    -- nothing in the buffer
	    return 0
    end if

    -- get the key code and the alt key code
    scan_code = peek( KEY_BUFFER + peek( FIRST_INDEX ) - 29 )
    key_code = peek( KEY_BUFFER + peek( FIRST_INDEX ) - 30 )

    -- clear the buffer
    poke( FIRST_INDEX, peek( LAST_INDEX ) )

    -- return the scan code
    return scan_code

end function

--- CODE ENDS HERE ---

-- david cuny
