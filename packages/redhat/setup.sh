#!/bin/sh
##############################################################################
#
#  Copyright 2004 JASSPA.
#
#  Created By    : Jon Green
#  Created       : Fri Feb 6 22:33:31 2004
#  Last Modified : <040206.2336>
#  Description   : Creates the RedHat directory structure for building a RPM
#                  file.
#
##############################################################################
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
# Copy the release TAR files to the SOURCES directory
#
if [ ! -f ./jasspa-mesrc-20040206.tar.gz ] ; then
    echo "Cannot find file jasspa-mesrc-20040206.tar.gz"
    exit 1
fi
cp ./jasspa-mesrc-20040206.tar.gz ~/rpmbuild/SOURCES/
#
if [ ! -f ./jasspa-metree-20040206.tar.gz ] ; then
    echo "Cannot find file jasspa-metree-20040206.tar.gz"
    exit 1
fi
cp ./jasspa-metree-20040206.tar.gz ~/rpmbuild/SOURCES/
#
# Now make the RPM
#
(cd ~/rpmbuild/SPECS; rpmbuild -ba jasspa.spec)
if [ -f ~/rpmbuild/RPMS/i386/jasspa-me-20040206-1.i386.rpm ] ; then
    cp ~/rpmbuild/RPMS/i386/jasspa-me-20040206-1.i386.rpm .
fi
if [ -f ~/rpmbuild/SRPMS/jasspa-me-20040206-1.src.rpm ] ; then
    cp ~/rpmbuild/SRPMS/jasspa-me-20040206-1.src.rpm .
fi
