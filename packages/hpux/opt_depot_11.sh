#!/bin/sh
#
#  System        : MicroEmacs
#  Module        : Package Build script.
#  Object Name   : $RCSfile: opt_depot_11.sh,v $
#  Revision      : $Revision: 1.4 $
#  Date          : $Date: 2004-03-27 13:18:06 $
#  Author        : $Author: jon $
#  Created By    : <unknown>
#  Created       : Sun Aug 17 12:58:23 2003
#  Last Modified : <040326.2229>
#
# Rules to build the HPUX 11.xx depot file. We build the executable only.
#
MKDIR=mkdir
SEARCH_PATH="/opt/jasspa/company:/opt/jasspa/macros:/opt/jasspa/spelling"
BASEDIR=.
METREE=jasspa-metree-20040301.tar.gz
MEBIN=jasspa-me-hpux-pa-11.00-20040301.gz
BASEFILESET="${METREE} ${MEBIN}"
# Set to "mak" for native or "gmk" for GCC
PLATFORM=`uname`
MAKEBAS=hpux11
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
### #
### # Build the directories
### #
### if [ ! -d ${BASEDIR} ] ; then
###     mkdir ${BASEDIR}
### fi
### for FILE in $DIRECTORIES ; do
###     if [ ! -d ${BASEDIR}/${FILE} ] ; then
###         mkdir ${BASEDIR}/${FILE}
###     fi
### done
### #
### # Build me
### #
### if [ ! -f ${BASEDIR}/bin/me ] ; then
###     if [ ! -f ${BASEDIR}/src/${MAKEBAS}.${CCMAK} ] ; then
###         gunzip -c mesrc.tar.gz | (cd ${BASEDIR}/src;  tar xvf - )
###     fi
###     MAKECDEFS="-D_SEARCH_PATH=\\"'"'"${SEARCH_PATH}\\"'"'
###     (cd ${BASEDIR}/src; make -f ${MAKEBAS}.${CCMAK} MAKECDEFS=$MAKECDEFS mecw)
###     cp ${BASEDIR}/src/mecw ${BASEDIR}/bin/me
###     (cd ${BASEDIR}; rm -rf ./src)
###     mkdir ${BASEDIR}/src
### fi
### #
### # Build ne
### #
### if [ ! -f ${BASEDIR}/bin/ne ] ; then
###     if [ ! -f ${BASEDIR}/src/${MAKEBAS}.${CCMAK} ] ; then
###         gunzip -c mesrc.tar.gz | (cd ${BASEDIR}/src;  tar xvf - )
###     fi
###     MAKECDEFS="-D_SEARCH_PATH=\\"'"'"${SEARCH_PATH}\\"'"'
###     (cd ${BASEDIR}/src; make -f ${MAKEBAS}.${CCMAK} MAKECDEFS=$MAKECDEFS nec)
###     cp ${BASEDIR}/src/nec ${BASEDIR}/bin/ne
###     (cd ${BASEDIR}/src; make -f ${MAKEBAS}.${CCMAK} spotless)
###     (cd ${BASEDIR}; rm -rf ./src)
###     mkdir ${BASEDIR}/src
### fi
#
# Unpack the tree
#
gunzip -c ${METREE} | tar xf -
#
# Unpack the executable.
#
mkdir -p ${BASEDIR}/jasspa/bin
gunzip -v -c ${MEBIN} > ${BASEDIR}/jasspa/bin/me
chmod a+rx ${BASEDIR}/jasspa/bin/me
#
# Build the ".prj" file.
#
echo "As root run the following command to build the depot"
echo '/usr/sbin/swpackage -d "| /usr/contrib/bin/gzip -c > jasspa-mepkg-hpux-pa-11.00-20040301.depot.gz" -x target_type=tape -s opt_depot_11.psf'
echo ""
echo "To install:-"
echo "/usr/contrib/bin/gunzip -c jasspa-mepkg-hpux-pa-11.00-20040301.depot.gz > jasspa-mepkg-hpux-pa-11.00-20040301.depot"
echo "/usr/sbin/swinstall -s `pwd`/jasspa-mepkg-hpux-pa-11.00-20040301.depot"
echo ""
echo "To subsequently remove:-"
echo "/usr/sbin/swremove jasspa-me"
