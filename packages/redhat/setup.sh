#!/bin/sh
set -x
##############################################################################
#
#  Copyright 2004-2006 JASSPA.
#
#  Created By    : Jon Green
#  Created       : Fri Feb 6 22:33:31 2004
#  Last Modified : <091011.2244>
#  Description   : Creates the RedHat directory structure for building a RPM
#                  file.
#
##############################################################################
# name of the release.
VER_YEAR="09"
VER_MONTH="09"
VER_DAY="09"
VERSION="20${VER_YEAR}${VER_MONTH}${VER_DAY}"
# name of the latest release.
LVER_YEAR="09"
LVER_MONTH="10"
LVER_DAY="10"
LATEST="20${LVER_YEAR}${LVER_MONTH}${LVER_DAY}"
#
METREE="jasspa-metree-${LATEST}.tar.gz"
MESRC="jasspa-mesrc-${LATEST}.tar.gz"
KERNEL_MAJOR=`uname -r | cut -c 1-1`
KERNEL_MINOR=`uname -r | cut -c 3-3`
ARCH=`uname -m`
#
KERNEL_VERSION=${KERNEL_MAJOR}.${KERNEL_MINOR}
MEBIN=jasspa-me-linux-${KERNEL_VERSION}-${ARCH}-${LATEST}.gz
MECBIN=jasspa-mec-linux-${KERNEL_VERSION}-${ARCH}-${LATEST}.gz
NEBIN=jasspa-ne-linux-${KERNEL_VERSION}-${ARCH}-${LATEST}.gz
# Website
JASSPACOM=www.jasspa.com
JASSPAFILES="${METREE} ${MESRC} me.1 ne.1 jasspa-microemacs.desktop jasspame.pdf change.log"
SPELLFILESET="dede enus engb eses fifi frfr itit plpl ptpt ruye ruyo"
#
# Create the file ~/.rpmmacro this is required to run as a user rather than
# root.
if [ ! -f ~/.rpmmacros ] ; then
    cp .rpmmacros ~/.rpmmacros
fi
#
# Delete any pre-constructed RPMs
#
rm -f *.rpm
#
# Create the directory RedHat directory structure
#
rm -rf ~/rpmbuild
mkdir -p ~/rpmbuild
mkdir -p ~/rpmbuild/BUILD
mkdir -p ~/rpmbuild/RPMS
mkdir -p ~/rpmbuild/RPMS/${ARCH}
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
for FILE in ${JASSPAFILES} ; do
    if [ ! -f ${FILE} ] ; then
        wget ${JASSPACOM}/release_${VERSION}/${FILE}
    fi
    if [ ! -f ${FILE} ] ; then
        echo "Cannot download or find file ${FILE}"
        exit 1
    fi
done
#
# Build me
#
if [ ! -f ${MEBIN} ] ; then
    rm -rf me${LVER_YEAR}${LVER_MONTH}${LVER_DAY}
    tar zxvf ${MESRC}
    (cd me${LVER_YEAR}${LVER_MONTH}${LVER_DAY}/src; sh build)
    gzip -9 -c me${LVER_YEAR}${LVER_MONTH}${LVER_DAY}/src/me > ${MEBIN}
    rm -rf me${LVER_YEAR}${LVER_MONTH}${LVER_DAY}
fi
#
if [ ! -f ${NEBIN} ] ; then
    rm -rf me${LVER_YEAR}${LVER_MONTH}${LVER_DAY}
    tar zxvf ${MESRC}
    (cd me${LVER_YEAR}${LVER_MONTH}${LVER_DAY}/src; sh build -ne)
    gzip -9 -c me${LVER_YEAR}${LVER_MONTH}${LVER_DAY}/src/ne > ${NEBIN}
    rm -rf me${LVER_YEAR}${LVER_MONTH}${LVER_DAY}
fi
#
if [ ! -f ${MECBIN} ] ; then
    rm -rf me${LVER_YEAR}${LVER_MONTH}${LVER_DAY}
    tar zxvf ${MESRC}
    (cd me${LVER_YEAR}${LVER_MONTH}${LVER_DAY}/src; sh build -t c)
    gzip -9 -c me${LVER_YEAR}${LVER_MONTH}${LVER_DAY}/src/mec > ${MECBIN}
    rm -rf me${LVER_YEAR}${LVER_MONTH}${LVER_DAY}
fi
#
# Pull over the spelling files.
#
for FILE in ${SPELLFILESET} ; do
    # Get the name of the file.
    SPELLFILE="ls_${FILE}.tar.gz"
    # Download from the server.
    if [ ! -f ${SPELLFILE} ] ; then
        wget ${JASSPACOM}/spelling/${SPELLFILE}
    fi
    if [ ! -f ${SPELLFILE} ] ; then
        echo "Cannot find file ${SPELLFILE}"
        exit 1
    fi
done
#
# Copy the release TAR files to the SOURCES directory
#
cp ./${MESRC} ~/rpmbuild/SOURCES/
gzip -9 -c ./me.1 > ~/rpmbuild/SOURCES/me.1.gz
gzip -9 -c ./ne.1 > ~/rpmbuild/SOURCES/ne.1.gz
cp ./jasspa-microemacs.desktop ~/rpmbuild/SOURCES/
cp change.log ~/rpmbuild/SOURCES/
#
# Now make the RPM
#
(cd ~/rpmbuild/SPECS; rpmbuild -ba jasspa.spec)
cp ~/rpmbuild/RPMS/${ARCH}/me-jasspa-${LATEST}-1.${ARCH}.rpm .
cp ~/rpmbuild/RPMS/${ARCH}/me-jasspa-nox-${LATEST}-1.${ARCH}.rpm .
cp ~/rpmbuild/RPMS/${ARCH}/ne-jasspa-${LATEST}-1.${ARCH}.rpm .
cp ~/rpmbuild/SRPMS/me-jasspa-${LATEST}-1.src.rpm .
#
##############################################################################
# Build the data RPM
##############################################################################
#
# Create the directory RedHat directory structure
#
rm -rf ~/rpmbuild
mkdir -p ~/rpmbuild
mkdir -p ~/rpmbuild/BUILD
mkdir -p ~/rpmbuild/RPMS
mkdir -p ~/rpmbuild/RPMS/${ARCH}
mkdir -p ~/rpmbuild/RPMS/noarch
mkdir -p ~/rpmbuild/SOURCES
mkdir -p ~/rpmbuild/SPECS
mkdir -p ~/rpmbuild/SRPMS
#
# Copy the spec into the SPECS directory
#
for FILE in ${SPELLFILESET} ; do
    # Get the name of the file.
    SPELLFILE="ls_${FILE}.tar.gz"
    # Download from the server.
    if [ ! -f ${SPELLFILE} ] ; then
        wget ${JASSPACOM}/spelling/${SPELLFILE}
    fi
    if [ ! -f ${SPELLFILE} ] ; then
        echo "Cannot find file ${SPELLFILE}"
        exit 1
    else
        cp ./${SPELLFILE} ~/rpmbuild/SOURCES/
    fi
done
#
cp ./${METREE} ~/rpmbuild/SOURCES/
cp jasspame.pdf ~/rpmbuild/SOURCES/
cp change.log ~/rpmbuild/SOURCES/
cp jasspadata.spec ~/rpmbuild/SPECS/
#
# Now make the noarch RPM
#
(cd ~/rpmbuild/SPECS; rpmbuild -bb jasspadata.spec)
cp ~/rpmbuild/RPMS/noarch/me-jasspa-data-${LATEST}-1.noarch.rpm .
cp ~/rpmbuild/RPMS/noarch/me-jasspa-dede-${LATEST}-1.noarch.rpm .
cp ~/rpmbuild/RPMS/noarch/me-jasspa-engb-${LATEST}-1.noarch.rpm .
cp ~/rpmbuild/RPMS/noarch/me-jasspa-enus-${LATEST}-1.noarch.rpm .
cp ~/rpmbuild/RPMS/noarch/me-jasspa-eses-${LATEST}-1.noarch.rpm .
cp ~/rpmbuild/RPMS/noarch/me-jasspa-fifi-${LATEST}-1.noarch.rpm .
cp ~/rpmbuild/RPMS/noarch/me-jasspa-frfr-${LATEST}-1.noarch.rpm .
cp ~/rpmbuild/RPMS/noarch/me-jasspa-itit-${LATEST}-1.noarch.rpm .
cp ~/rpmbuild/RPMS/noarch/me-jasspa-ptpt-${LATEST}-1.noarch.rpm .
cp ~/rpmbuild/RPMS/noarch/me-jasspa-plpl-${LATEST}-1.noarch.rpm .
cp ~/rpmbuild/RPMS/noarch/me-jasspa-ruye-${LATEST}-1.noarch.rpm .
cp ~/rpmbuild/RPMS/noarch/me-jasspa-ruyo-${LATEST}-1.noarch.rpm .
#cp ~/rpmbuild/SRPMS/me-jasspa-data-${LATEST}-1.src.rpm .
