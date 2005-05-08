#!/bin/sh
##############################################################################
#
#  Copyright 2004 JASSPA.
#
#  Created By    : Jon Green
#  Created       : Fri Feb 6 22:33:31 2004
#  Last Modified : <050508.1703>
#  Description   : Creates the RedHat directory structure for building a RPM
#                  file.
#
##############################################################################
TOPDIR=../..
VER_YEAR="05"
VER_MONTH="05"
VER_DAY="05"
VERSION="20${VER_YEAR}${VER_MONTH}${VER_DAY}"
METREE=jasspa-metree-${VERSION}.tar.gz
MESRC=jasspa-mesrc-${VERSION}.tar.gz
#
# Create the file ~/.rpmmacro this is required to run as a user rather than
# root.
if [ ! -f ~/.rpmmacros ] ; then
    cp .rpmmacros ~/.rpmmacros
fi
#
# Create the directory RedHat directory structure
#
mkdir -p ~/rpmbuild
mkdir -p ~/rpmbuild/BUILD
mkdir -p ~/rpmbuild/RPMS
mkdir -p ~/rpmbuild/RPMS/athlon
mkdir -p ~/rpmbuild/RPMS/i386
mkdir -p ~/rpmbuild/RPMS/i486
mkdir -p ~/rpmbuild/RPMS/i586
mkdir -p ~/rpmbuild/RPMS/i686
mkdir -p ~/rpmbuild/RPMS/noarch
mkdir -p ~/rpmbuild/SOURCES
mkdir -p ~/rpmbuild/SPECS
mkdir -p ~/rpmbuild/SRPMS
#
# Copy the spec into the SPECS directory
#
cp jasspa.spec ~/rpmbuild/SPECS/
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
if [ ! -f ./me.1.gz ] ; then
    if [ -f ${TOPDIR}/release/www/me.1.gz ] ; then
        cp ${TOPDIR}/release/www/me.1.gz me.1.gz
    fi
fi
#
# Copy the release TAR files to the SOURCES directory
#
if [ ! -f ${MESRC} ] ; then
    echo "Cannot find file ${MESRC}"
    exit 1
fi
cp ./${MESRC} ~/rpmbuild/SOURCES/
#
if [ ! -f ${METREE} ] ; then
    echo "Cannot find file ${METREE}"
    exit 1
fi
cp ./${METREE} ~/rpmbuild/SOURCES/
#
if [ ! -f ./me.1.gz ] ; then
    echo "Cannot find file me.1.gz"
    exit 1
fi
cp ./me.1.gz ~/rpmbuild/SOURCES/
#
# Now make the RPM
#
(cd ~/rpmbuild/SPECS; rpmbuild -ba jasspa.spec)
if [ -f ~/rpmbuild/RPMS/i386/jasspa-me-${VERSION}-1.i386.rpm ] ; then
    cp ~/rpmbuild/RPMS/i386/jasspa-me-${VERSION}-1.i386.rpm .
fi
if [ -f ~/rpmbuild/SRPMS/jasspa-me-${VERSION}-1.src.rpm ] ; then
    cp ~/rpmbuild/SRPMS/jasspa-me-${VERSION}-1.src.rpm .
fi
