#!/bin/sh
#
#  System        : MicroEmacs
#  Module        : Package Build script.
#  Object Name   : $RCSfile: opt_depot_11.sh,v $
#  Revision      : $Revision: 1.8 $
#  Date          : $Date: 2005-05-08 23:14:54 $
#  Author        : $Author: jon $
#  Created By    : <unknown>
#  Created       : Sun Aug 17 12:58:23 2003
#  Last Modified : <050509.0013>
#
# Rules to build the HPUX 11.xx depot file. We build the executable only.
#
MKDIR=mkdir
SEARCH_PATH="/opt/jasspa/company:/opt/jasspa/macros:/opt/jasspa/spelling"
BASEDIR=.
TOPDIR=../..
VER_YEAR="05"
VER_MONTH="05"
VER_DAY="05"
VERSION="20${VER_YEAR}${VER_MONTH}${VER_DAY}"
METREE=jasspa-metree-${VERSION}.tar.gz
MEBIN=jasspa-me-hpux-pa-11.00-${VERSION}.gz
BASEFILESET="${METREE} ${MEBIN}"
# Set to "mak" for native or "gmk" for GCC
PLATFORM=`uname`
MAKEBAS=hpux11
CCMAK=gmk
#
# Pull the files over from the release and source areas.
#
if [ ! -f ${METREE} ] ; then
    if [ -f ${TOPDIR}/release/www/${METREE} ] ; then
        cp ${TOPDIR}/release/www/${METREE} .
    fi
fi
if [ ! -f me.1 ] ; then
    if [ -f ${TOPDIR}/release/www/me.1.gz ] ; then
        gunzip -c ${TOPDIR}/release/www/me.1.gz > me.1
    fi
fi
if [ ! -f ${MEBIN} ] ; then
    if [ -f ${TOPDIR}/src/${MEBIN} ] ; then
        cp ${TOPDIR}/src/${MEBIN} ${MEBIN}
    elif [ -f ${TOPDIR}/src/me ] ; then
        gzip -9 -c ${TOPDIR}/src/me > ${MEBIN}
    fi
fi
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
# Remove the unpacking area.
#
if [ -d jasspa ] ; then
    rm -rf ./jasspa
fi    
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
# Build the man directory.
#
mkdir -p ${BASEDIR}/jasspa/man/cat1
cp me.1 ${BASEDIR}/jasspa/man/cat1
chmod a-wx ${BASEDIR}/jasspa/man/cat1/me.1
#
# Build the ".prj" file.
#
echo "As root run the following command to build the depot"
echo '/usr/sbin/swpackage -d "| /usr/contrib/bin/gzip -c > jasspa-mepkg-hpux-pa-11.00-'${VERSION}'.depot.gz" -x target_type=tape -s opt_depot_11.psf'
echo ""
echo "To install:-"
echo "/usr/contrib/bin/gunzip -c jasspa-mepkg-hpux-pa-11.00-${VERSION}.depot.gz > jasspa-mepkg-hpux-pa-11.00-${VERSION}.depot"
echo "/usr/sbin/swinstall -s `pwd`/jasspa-mepkg-hpux-pa-11.00-${VERSION}.depot"
echo ""
echo "To subsequently remove:-"
echo "/usr/sbin/swremove jasspa-me"
