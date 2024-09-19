##-*- makefile -*-############################################################
#
#  Copyright (c) 2024 Detlef Groth.
# 
#
##############################################################################

## just for testing of a local install
pkg=microemacs-mec
version=20240903
## or reinstall
action=install
shafile=Jasspa_MicroEmacs_$(version)-sha256.txt
default:
	brew $(action) --build-from-source  $(pkg).rb

shafile:
	@if [ ! -f $(shafile) ]; then wget -O $(shafile) https://github.com/bjasspa/jasspa/releases/download/me_$(version)/Jasspa_MicroEmacs_$(version)-sha256.txt ; fi
### create receipe (rb files) from templates (tpl)
tpl2rb: shafile
	bash tpl2rb.sh mec2rb  $(shafile) microemacs-mec.tpl
	bash tpl2rb.sh mec2rb  $(shafile) microemacs.tpl
	bash tpl2rb.sh mec2rb  $(shafile) microemacs-mew.tpl	
	bash tpl2rb.sh mec2rb  $(shafile) microemacs-openssl.tpl
	bash tpl2rb.sh mec2rb  $(shafile) microemacs-mecs.tpl
	bash tpl2rb.sh mec2rb  $(shafile) microemacs-mews.tpl
	bash tpl2rb.sh lang2rb $(shafile) microemacs-spelling-template.tpl
	bash tpl2rb.sh emf2rb  $(shafile) microemacs-help.tpl
	bash tpl2rb.sh emf2rb  $(shafile) microemacs-macros.tpl
readme:	
	# We need a dummy file for microemacs.rb
	-mkdir Jasspa_MicroEmacs_$(version)_readme
	cp microemacs/release_readme.txt Jasspa_MicroEmacs_$(version)_readme/readme.txt
	cp microemacs/COPYING.txt Jasspa_MicroEmacs_$(version)_readme/license.txt
