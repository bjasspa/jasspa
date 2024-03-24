##-*- makefile -*-############################################################
#
#  System        : 
#  Module        : 
#  Object Name   : $RCSfile$
#  Revision      : $Revision$
#  Date          : $Date$
#  Author        : $Author$
#  Created By    : <unknown>
#  Created       : Sun Mar 24 17:52:35 2024
#  Last Modified : <240324.1905>
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
#  Copyright (c) 2024 <unknown>.
# 
#  All Rights Reserved.
# 
#  This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
#  reproduced,  translated,  or  reduced to any  electronic  medium or machine
#  readable form without prior written consent from <unknown>.
#
##############################################################################

PLATFORM=ubuntu20
VERSION=202403
ifeq ($(PLATFORM),ubuntu20)
   MEDIR := .linux32gcc-release
endif
ifeq ($(PLATFORM),ubuntu22)
   MEDIR := .linux32gcc-release
endif
ifeq ($(PLATFORM),fedora38)
   MEDIR := .linux32gcc-release
endif
ifeq ($(PLATFORM),macos11)
   MEDIR := .macos64cc-release
endif

default:
	cd microemacs/src && bash build.sh -t w
	cd microemacs/src && bash build.sh -t c	
	-mkdir -p MicroEmacs_$(VERSION)_$(PLATFORM)_mecs/bin
	-mkdir -p MicroEmacs_$(VERSION)_$(PLATFORM)_mews/bin
	-mkdir -p MicroEmacs_$(VERSION)_$(PLATFORM)_mec/bin
	-mkdir -p MicroEmacs_$(VERSION)_$(PLATFORM)_mew/bin	
	cp microemacs/src/$(MEDIR)-mew/mew MicroEmacs_$(VERSION)_$(PLATFORM)_mew/bin/
	cp microemacs/src/$(MEDIR)-mec/mec MicroEmacs_$(VERSION)_$(PLATFORM)_mec/bin/
	cp microemacs/license.txt microemacs/src/readme.txt MicroEmacs_$(VERSION)_$(PLATFORM)_mew/
	cp microemacs/license.txt microemacs/src/readme.txt MicroEmacs_$(VERSION)_$(PLATFORM)_mews/	
	cp microemacs/license.txt microemacs/src/readme.txt MicroEmacs_$(VERSION)_$(PLATFORM)_mec/
	cp microemacs/license.txt microemacs/src/readme.txt MicroEmacs_$(VERSION)_$(PLATFORM)_mecs/
	export PATH=`pwd`/microemacs/src/$(MEDIR)-mec/:$PATH
	export PATH=`pwd`/microemacs/3rdparty/tfs/$(MEDIR)/:$PATH
	PATH=`pwd`/microemacs/src/$(MEDIR)-mec/:`pwd`/microemacs/3rdparty/tfs/$(MEDIR)/:$$PATH && \
		cd microemacs/mesingle && bash mesgen.sh -d -p ../src/$(MEDIR)-mew/mew -o ../../MicroEmacs_$(VERSION)_$(PLATFORM)_mews/bin/mews
	PATH=`pwd`/microemacs/src/$(MEDIR)-mec/:`pwd`/microemacs/3rdparty/tfs/$(MEDIR)/:$$PATH && \
		cd microemacs/mesingle && bash mesgen.sh -d -p ../src/$(MEDIR)-mec/mec -o ../../MicroEmacs_$(VERSION)_$(PLATFORM)_mecs/bin/mecs

