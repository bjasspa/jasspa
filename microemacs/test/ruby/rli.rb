#!/usr/bin/ruby

require "language/lua"

print "Ruby Lua Interpreter !\n"
o = Language::Lua.new()
o.load( ARGV[0] )
