##-*- makefile -*-############################################################
#
#  Copyright (c) 2024 Detlef Groth.
# 
#
##############################################################################

## just for testing of a local install
pkg=microemacs-mec
version=20240902
## or reinstall
action=install
default:
	brew $(action) --build-from-source  $(pkg).rb

### create receipe (rb files) from templates (tpl)
tpl2rb:
	bash tpl2rb.sh mec2rb  Jasspa_MicroEmacs_$(version)-sha265.txt microemacs-mec.tpl
	bash tpl2rb.sh mec2rb  Jasspa_MicroEmacs_$(version)-sha265.txt microemacs-mew.tpl
	bash tpl2rb.sh mec2rb  Jasspa_MicroEmacs_$(version)-sha265.txt microemacs-openssl.tpl
	bash tpl2rb.sh mec2rb  Jasspa_MicroEmacs_$(version)-sha265.txt microemacs-mecs.tpl
	bash tpl2rb.sh mec2rb  Jasspa_MicroEmacs_$(version)-sha265.txt microemacs-mews.tpl
	bash tpl2rb.sh lang2rb Jasspa_MicroEmacs_$(version)-sha265.txt microemacs-spelling-template.tpl
	bash tpl2rb.sh emf2rb  Jasspa_MicroEmacs_$(version)-sha265.txt microemacs-help.tpl
	bash tpl2rb.sh emf2rb  Jasspa_MicroEmacs_$(version)-sha265.txt microemacs-macros.tpl
