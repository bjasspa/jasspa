##-*- makefile -*-############################################################
#
#  System        : 
#  Module        : 
#  Object Name   : $RCSfile$
#  Revision      : $Revision$
#  Date          : $Date$
#  Author        : $Author$
#  Created By    : Detlef Groth
#  Created       : Wed Sep 11 08:13:58 2024
#  Last Modified : <241201.2344>
#
#  Description	
#
#  Notes
#
#  History
#	
#  $Log$
#
##############################################################################
#
#  Copyright (c) 2024 Detlef Groth.
# 
#  All Rights Reserved.
# 
#  This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
#  reproduced,  translated,  or  reduced to any  electronic  medium or machine
#  readable form without prior written consent from Detlef Groth.
#
##############################################################################

version=20240902

default:
	if [ ! -f Jasspa_MicroEmacs_$(version)_bin_windows_binaries.zip ]; then wget https://github.com/bjasspa/jasspa/releases/download/me_20240902/Jasspa_MicroEmacs_$(version)_abin_windows_mesc.zip ; fi
	unzip Jasspa_MicroEmacs_$(version)_bin_windows_binaries.zip -d microemacs-package
	cp microemacs-package/bin/*/mec.exe microemacs-package
	cp microemacs-package/bin/*/mew.exe microemacs-package	
	sha5sum microemacs-package/mec.exe  microemacs-package && rm *.zst
	
	#cd microemacs-package && makepkg -si
	#pacman -Qlp package_file.pkg.tar.zst
