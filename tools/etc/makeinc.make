# -*- Makefile -*-
#
# Get the platform name
#
UNAME:sh =uname | tr '[A-Z]' '[a-z]'
#
# Find out the compiler.
#
WHICHCC:sh =which cc
