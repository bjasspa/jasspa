#!/usr/bin/env python3
# -*- coding: ISO-8859-15 -*-
"""Modulename

Usage: {0} ?-h,-?,--help? INFILE OUTFILE

Arguments:
    INFILE    - UTF-8 encoded text input file
    OUTFILE   - ASCII / ISO / CP1252 encoded  output file
"""
__author__ = "first last"
__version__ = "0.1"
import sys, os, re

def help(argv):
    print(__doc__.format(argv[1]))
    
def usage(argv):
    print(f"Usage: {argv[0]} ?-h, --help? INFILE OUTFILE ")
    
def main(argv):
    if (len(argv)) != 3:
        usage(argv)
        
    elif "-h" in argv or "--help" in argv:
        help(argv)
    else:
        with open(argv[1],"r",encoding="utf-8") as f: text = f.read()
    
        out_bytes = bytearray()
    
        for ch in text:
            code = ord(ch)
    
            if code <= 0xFF:
                out_bytes.append(code)
            else:
                esc = r"\u%04X" % code
                out_bytes.extend(esc.encode("ascii"))
    
        with open(argv[2],"wb") as f: f.write(out_bytes)
               
        
if __name__ == "__main__":
    main(sys.argv)
    


