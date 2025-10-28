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
    if (len(argv)) < 3:
        usage(argv)
    elif "-h" in argv or "--help" in argv:
        help(argv)
    else:
        enc = "cp1252"
        if len(argv) == 4:
             enc = argv[1]
             argv=[argv[0],argv[2],argv[3]]
        with open(argv[1], "r", encoding=enc) as f:
            text = f.read()
            text = re.sub(r"\\u([0-9a-fA-F]{4})", lambda m: chr(int(m.group(1),16)), text)
        with open(argv[2], "w", encoding="utf-8") as g:
            g.write(text)
        
if __name__ == "__main__":
    main(sys.argv)
    


