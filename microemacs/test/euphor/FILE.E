		-- Random Access File Operations --
		-- plus directory functions      --

constant M_SEEK  = 19,
	 M_WHERE = 20,
	 M_DIR   = 22,
	 M_CURRENT_DIR = 23

type file_number(integer f)
    return f >= 0
end type

type file_position(integer p)
    return p >= -1
end type

global function seek(file_number fn, file_position pos)
-- seeks to a byte position in the file, 
-- or to end of file if pos is -1
    return machine_func(M_SEEK, {fn, pos})
end function

global function where(file_number fn)
-- returns the current byte position in the file
    return machine_func(M_WHERE, fn)
end function

global constant 
	D_NAME = 1,
	D_ATTRIBUTES = 2,
	D_SIZE = 3,

	D_YEAR = 4,
	D_MONTH = 5,
	D_DAY = 6,

	D_HOUR = 7,
	D_MINUTE = 8,
	D_SECOND = 9
global function dir(sequence name)
-- returns directory information, given the name
-- of a file or directory. Format returned is:
-- {
--  {"name1", attributes, size, year, month, day, hour, minute, second},
--  {"name2", ...                                                     },
-- }
    return machine_func(M_DIR, name)
end function

global function current_dir()
-- returns name of current working directory
    return machine_func(M_CURRENT_DIR, 0)
end function
	
