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
#  Last Modified : <240602.0836>
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
VERSION=202406
ifeq (ubuntu,$(findstring ubuntu,$(PLATFORM)))
   MEDIR := .linux32gcc-release
endif
ifeq (fedora,$(findstring fedora,$(PLATFORM)))
   MEDIR := .linux32gcc-release
endif
#ifeq (macos,$findstring macos,$(PLATFORM))
#   MEDIR := .macos64cc-release
#endif
ifeq ($(PLATFORM),macos11)
   MEDIR := .macos64cc-release
endif
ifeq ($(PLATFORM),macos14)
   MEDIR := .macos64cc-release
endif

default:
	cd microemacs/3rdparty/tfs && bash build.sh
	cd microemacs/src && bash build.sh -t w
	cd microemacs/src && bash build.sh -t c	
	cd microemacs/src && bash build.sh -t cw		
	-mkdir -p MicroEmacs_$(VERSION)_$(PLATFORM)_mecs/bin
	-mkdir -p MicroEmacs_$(VERSION)_$(PLATFORM)_mews/bin
	-mkdir -p MicroEmacs_$(VERSION)_$(PLATFORM)_mecws/bin
	-mkdir -p MicroEmacs_$(VERSION)_$(PLATFORM)_mec/bin
	-mkdir -p MicroEmacs_$(VERSION)_$(PLATFORM)_mew/bin	
	-mkdir -p MicroEmacs_$(VERSION)_$(PLATFORM)_mecw/bin		
	-mkdir -p tfs_$(VERSION)_$(PLATFORM)/bin
	ln -sf `pwd`/microemacs/macros macros_$(VERSION)
	cp microemacs/3rdparty/tfs/$(MEDIR)/tfs tfs_$(VERSION)_$(PLATFORM)/bin/
	cp microemacs/3rdparty/tfs/readme.txt tfs_$(VERSION)_$(PLATFORM)/
	cp microemacs/src/$(MEDIR)-mew/mew MicroEmacs_$(VERSION)_$(PLATFORM)_mew/bin/
	cp microemacs/src/$(MEDIR)-mec/mec MicroEmacs_$(VERSION)_$(PLATFORM)_mec/bin/
	cp microemacs/license.txt microemacs/src/readme.txt MicroEmacs_$(VERSION)_$(PLATFORM)_mew/
	cp microemacs/license.txt microemacs/src/readme.txt MicroEmacs_$(VERSION)_$(PLATFORM)_mews/	
	cp microemacs/license.txt microemacs/src/readme.txt MicroEmacs_$(VERSION)_$(PLATFORM)_mec/
	cp microemacs/license.txt microemacs/src/readme.txt MicroEmacs_$(VERSION)_$(PLATFORM)_mecs/
	cp microemacs/license.txt microemacs/src/readme.txt MicroEmacs_$(VERSION)_$(PLATFORM)_mecw/		
	cp microemacs/license.txt microemacs/src/readme.txt MicroEmacs_$(VERSION)_$(PLATFORM)_mecws/	
	export PATH=`pwd`/microemacs/src/$(MEDIR)-mec/:$PATH
	export PATH=`pwd`/microemacs/3rdparty/tfs/$(MEDIR)/:$PATH
	PATH=`pwd`/microemacs/src/$(MEDIR)-mec/:`pwd`/microemacs/3rdparty/tfs/$(MEDIR)/:$$PATH && \
		cd microemacs/mesingle && bash mesgen.sh -d -p ../src/$(MEDIR)-mew/mew -o ../../MicroEmacs_$(VERSION)_$(PLATFORM)_mews/bin/mews
	PATH=`pwd`/microemacs/src/$(MEDIR)-mec/:`pwd`/microemacs/3rdparty/tfs/$(MEDIR)/:$$PATH && \
		cd microemacs/mesingle && bash mesgen.sh -d -p ../src/$(MEDIR)-mec/mec -o ../../MicroEmacs_$(VERSION)_$(PLATFORM)_mecs/bin/mecs
	PATH=`pwd`/microemacs/src/$(MEDIR)-mec/:`pwd`/microemacs/3rdparty/tfs/$(MEDIR)/:$$PATH && \
		cd microemacs/mesingle && bash mesgen.sh -d -p ../src/$(MEDIR)-mec/mecw -o ../../MicroEmacs_$(VERSION)_$(PLATFORM)_mecs/bin/mecws
test:
	echo "MEDIR=$(MEDIR) PLATFORM=$(PLATFORM)"
	echo $(findstring fedora,$(PLATFORM))
