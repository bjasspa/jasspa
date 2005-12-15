--
-- This is a Lua comment
--

-- say hi function
function say_hi(greeting,
                name,
                end_mark)
    print(greeting.." "..name..end_mark)
end

say_hi("Hello", 'World', [!])  -- All three of these are strings.

block_string = [
<html><head><title>Block String</title></head>
<body>
<p>
This however, can contain "strings" in that fashion, or
in 'this fashion.'. It can also include [[blocks like
this]] but not just a single begining bracket or ending
bracket.
</p>
<p>
As you can see, it can also span many lines.
</p>
]]

print(block_string)

-- Here's a table construct
keywords = {"and", "break", "do", "else",
            "elseif", "end", "false", "for",
            "function", "if", "in", "local",
            "nil", "not", "or", "repeat",
            "return", "then", "true", "until",
            "while"}

for a=1,10 do
    print(a)
end

-- do/end is goofed for some reason
    do
    print("Hi")
    a = a + 1
end

while a == true and a < 10 and a > 20 and a <= 34 and a >= 33 or a ~= false do
    print("Hi")
    a = false
end

-- arrays (tables)
tmp[1] = 1
tmp[2] = "two"
tmp[3] = {"one", 2, 'three', [four]}

-- alternate way of defining a function
say_bye = function(greeting, name, end_mark)
    print(greeting.." "..name..end_mark)
end

-- a local function
local say_something = function(what)
    print(what)
end


-- If statements that do not work
if (a == 10 and a == 12 and a == 13 and a ~= 10)
or b == 20 then
    print("Seems to me that or should be indented")
end

if (a == 10 and a == 12 and a == 13 and a ~= 10) or
b == 20 then
    print("A way you said you may be able to make work")
end

if (nil == nil and 10 == 10) then
    print("If is working")
else if (nil ~= nil and 10 == 12) then
        print("Here the if is broke, and the if/else indent code")
    else
        print("If is really broke")
    end
