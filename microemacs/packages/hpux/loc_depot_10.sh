#!/bin/sh
#
#  System        : MicroEmacs
#  Module        : Package Build script.
#  Object Name   : $RCSfile: loc_depot_10.sh,v $
#  Revision      : $Revision: 1.1 $
#  Date          : $Date: 2004-02-06 23:39:20 $
#  Author        : $Author: jon $
#  Created By    : <unknown>
#  Created       : Sun Aug 17 12:58:23 2003
#  Last Modified : <030817.1453>
#
# Rules to build the HPUX 10.xx depot file. We build the executable only.
#
MKDIR=mkdir
DIRECTORIES="microemacs bin"
SEARCH_PATH="/usr/local/microemacs"
BASEDIR="loc10"
BASEFILESET="me.ehf.gz meicons.tar.gz memacros.tar.gz mesrc.tar.gz mehpux10.gz nehpux10.gz"
# Set to "mak" for native or "gmk" for GCC
PLATFORM=`uname`
MAKEBAS=hpux10
CCMAK=gmk
#
# Test for the starting files.
#
for FILE in $BASEFILESET ; do
    if [ ! -f ${FILE} ] ; then
        echo "Cannot find file ${FILE}"
        exit 1
    fi
done
#
# Build the directories
#
if [ ! -d ${BASEDIR} ] ; then
    mkdir ${BASEDIR}
fi
for FILE in $DIRECTORIES ; do
    if [ ! -d ${BASEDIR}/${FILE} ] ; then
        mkdir ${BASEDIR}/${FILE}
    fi
done
#
# Build me
#
if [ ! -f ${BASEDIR}/bin/me ] ; then
    gunzip -c mehpux10.gz > ${BASEDIR}/bin/me
    chmod 0555 ${BASEDIR}/bin/me
fi
#
# Build ne
#
if [ ! -f ${BASEDIR}/bin/ne ] ; then
    gunzip -c nehpux10.gz > ${BASEDIR}/bin/ne
    chmod 0555 ${BASEDIR}/bin/ne
fi
#
# Build the HTML
#
# gunzip -c mehtml.tar.gz | (cd ${BASEDIR}/doc;  tar xvf - )
#
# Build the macros
#
if [ ! -f ${BASEDIR}/microemacs/me.emf ] ; then
    gunzip -c memacros.tar.gz | (cd ${BASEDIR}/microemacs; tar xvf - )
fi    
if [ ! -f ${BASEDIR}/microemacs/me.ehf ] ; then
    gunzip -c me.ehf.gz > ${BASEDIR}/microemacs/me.ehf
fi    
#
# Build the icons
#
if [ ! -f ${BASEDIR}/microemacs/me.xpm ] ; then
    gunzip -c meicons.tar.gz | (cd ${BASEDIR}/microemacs; tar xvf - )
fi
#
# Build the ".prj" file.
#
echo "As root run the following command to build the depot"
echo '/usr/sbin/swpackage -d "| /usr/contrib/bin/gzip -c > jasspame-loc-0212-10.20.depot.gz" -x target_type=tape -s loc_depot_10.psf'
echo ""
echo "To install:-"
echo "/usr/contrib/bin/gunzip -c jasspame-loc-0212-10.20.depot.gz > jasspame-loc-0212-10.20.depot"
echo "/usr/sbin/swinstall -s `pwd`/jasspame-loc-0212-10.20.depot jasspame"
echo ""
echo "To subsequently remove:-"
echo "/usr/sbin/swremove jasspame"
