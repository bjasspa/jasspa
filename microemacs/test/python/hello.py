#!/usr/bin/env python3
# -*- coding: ISO-8859-15 -*-
"""Modulename

Usage: {0} ?-h,-?,--help?

Arguments:
"""

__author__ = "first last"
__version__ = "0.1"
import sys, os, re


def help(argv):
    print(__doc__.format(argv[1]))


def usage(argv):
    print(f"Usage: {argv[0]} args")


def main(argv):
    if (len(argv)) == 1:
        usage(argv)

    elif "-h" in argv or "--help" in argv:
        help(argv)


if __name__ == "__main__":
    main(sys.argv)
