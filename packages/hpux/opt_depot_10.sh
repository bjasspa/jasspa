#!/bin/sh
#
#  System        : MicroEmacs
#  Module        : Package Build script.
#  Object Name   : $RCSfile: opt_depot_10.sh,v $
#  Revision      : $Revision: 1.8 $
#  Date          : $Date: 2006-09-24 13:21:41 $
#  Author        : $Author: jon $
#  Created By    : <unknown>
#  Created       : Sun Aug 17 12:58:23 2003
#  Last Modified : <060924.1414>
#
# Rules to build the HPUX 10.20 depot file. We build the executable only.
#
MKDIR=mkdir
SEARCH_PATH="/opt/jasspa/company:/opt/jasspa/macros:/opt/jasspa/spelling"
BASEDIR=.
TOPDIR=../..
VER_YEAR="06"
VER_MONTH="09"
VER_DAY="09"
VERSION="20${VER_YEAR}${VER_MONTH}${VER_DAY}"
METREE=jasspa-metree-${VERSION}.tar.gz
MESRC=jasspa-mesrc-${VERSION}.tar.gz
MEBIN=jasspa-me-hpux-pa-10.20-${VERSION}.gz
NEBIN=jasspa-ne-hpux-pa-10.20-${VERSION}.gz
BASEFILESET="${METREE} ${MEBIN}"
# Set to "mak" for native or "gmk" for GCC
PLATFORM=`uname`
MAKEBAS=hpux10
CCMAK=gmk
#
# Pull the files over from the release and source areas.
#
if [ ! -f ${METREE} ] ; then
    if [ -f ${TOPDIR}/release/www/${METREE} ] ; then
        cp ${TOPDIR}/release/www/${METREE} .
    fi
fi
if [ ! -f ${MESRC} ] ; then
    if [ -f ${TOPDIR}/release/www/${MESRC} ] ; then
        cp ${TOPDIR}/release/www/${MESRC} .
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
    else
        # Build me
        gunzip -c ${MESRC} | tar xf -
        (cd me${VER_YEAR}${VER_MONTH}${VER_DAY}/src; sh build -ne -m hpux10.gmk)
        gzip -9 -c me${VER_YEAR}${VER_MONTH}${VER_DAY}/src/ne > ${NEBIN}
        (cd me${VER_YEAR}${VER_MONTH}${VER_DAY}/src; sh build -m hpux10.gmk)
        gzip -9 -c me${VER_YEAR}${VER_MONTH}${VER_DAY}/src/me > ${MEBIN}
        rm -rf me${VER_YEAR}${VER_MONTH}${VER_DAY}
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
echo '/usr/sbin/swpackage -d "| /usr/contrib/bin/gzip -c > jasspa-mepkg-hpux-pa-10.20-'${VERSION}'.depot.gz" -x target_type=tape -s opt_depot_10.psf'
echo ""
echo "To install:-"
echo "/usr/contrib/bin/gunzip -c jasspa-mepkg-hpux-pa-10.20-${VERSION}.depot.gz > jasspa-mepkg-hpux-pa-10.20-${VERSION}.depot"
echo "/usr/sbin/swinstall -s `pwd`/jasspa-mepkg-hpux-pa-10.20-${VERSION}.depot"
echo ""
echo "To subsequently remove:-"
echo "/usr/sbin/swremove jasspa-me"
