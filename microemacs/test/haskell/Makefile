##-*- makefile -*-############################################################
#
# Copyright (C) 2025 MicroEmacs User.
#
# All rights reserved.
#
# Synopsis:    
# Authors:     MicroEmacs User
#
##############################################################################
file=hello
compile:
	ghc $(file).hs && ./hello
	
lint:
	hlint $(file).hs
	
format:
	ormolu -i $(file).hs 
