# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# win32vc10.mak - Make file for Windows using Microsoft MSVC v10.0 development kit.
#
# Copyright (C) 2007-2024 JASSPA (www.jasspa.com)
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 675 Mass Ave, Cambridge, MA 02139, USA.
#
##############################################################################
#
# Synopsis:    The build version definition.
# Authors:     Jon Green & Steven Phillips
# 
# Notes:
#     These definitions have been moved out of evers.h into here so that makefiles
#     can also use the values, e.g. on Windows they are used in the creation of the
#     manifest file.
#
meVER_CN = 20
meVER_YR = 25
meVER_MN = 9
meVER_DY = 2
