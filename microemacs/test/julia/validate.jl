#!/usr/bin/env julia


function validatesyntax(fname)
    inp = read(fname, String)
    ast = ccall(:jl_parse_all, Any, (Cstring, Int64, Cstring, Int64), inp, length(inp), fname, length(fname))
    return if length(ast.args) > 1 && (ast.args[end].head in [:incomplete, :error])
        println("syntax error $(ast.args[end])")
    else
        println("No syntax errors.")
    end
end
if length(ARGS) < 1
    println("Usage: julia validate.jl <filename>")
else
    validatesyntax(ARGS[1])
end
