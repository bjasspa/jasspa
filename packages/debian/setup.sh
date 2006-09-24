#!/bin/sh
##############################################################################
#
#  Copyright 2004 JASSPA.
#
#  Created By    : Jon Green
#  Created       : Fri Feb 6 22:33:31 2004
#  Last Modified : <060924.1413>
#  Description   : Creates the RedHat directory structure for building a RPM
#                  file.
#
##############################################################################
TOPDIR=../..
VER_YEAR="06"
VER_MONTH="09"
VER_DAY="09"
VERSION="20${VER_YEAR}${VER_MONTH}${VER_DAY}"
RELDIR=${TOPDIR}/../jasspacom/release_${VERSION}
METREE=jasspa-metree-${VERSION}.tar.gz
MESRC=jasspa-mesrc-${VERSION}.tar.gz
MEPDF=jasspame.pdf
KERNEL_MAJOR=`uname -r | cut -c 1-1`
KERNEL_MINOR=`uname -r | cut -c 3-3`
KERNEL_VERSION=${KERNEL_MAJOR}.${KERNEL_MINOR}
MEBIN=jasspa-me-linux-i386-${KERNEL_VERSION}-${VERSION}.gz
NEBIN=jasspa-ne-linux-i386-${KERNEL_VERSION}-${VERSION}.gz
#
# Pull the files over from the release and source areas.
#
if [ ! -f ${METREE} ] ; then
    if [ -f ${RELDIR}/${METREE} ] ; then
        cp ${RELDIR}/${METREE} .
    fi
fi
if [ ! -f ${MESRC} ] ; then
    if [ -f ${RELDIR}/${MESRC} ] ; then
        cp ${RELDIR}/${MESRC} .
    fi
fi
if [ ! -f ${MEPDF} ] ; then 
    cp ${RELDIR}/doc/COPYING .
    cp ${RELDIR}/doc/change.log .
    cp ${RELDIR}/doc/readme.txt .
    if [ -f ${RELDIR}/doc/${MEPDF} ] ; then
        cp ${RELDIR}/doc/${MEPDF} .
    fi
fi            
if [ ! -f ./me.1.gz ] ; then
    if [ -f ${RELDIR}/me.1.gz ] ; then
        cp ${RELDIR}/me.1.gz me.1.gz
    fi
fi
#
# Copy the release TAR files to the SOURCES directory
#
if [ ! -f ${MESRC} ] ; then
    echo "Cannot find file ${MESRC}"
    exit 1
fi
#
# Build me
# 
if [ ! -f ${MEBIN} ] ; then
    rm -rf me${VER_YEAR}${VER_MONTH}${VER_DAY}
    gunzip -c ${MESRC} | tar xf -
    rm -f ${NEBIN}
    (cd me${VER_YEAR}${VER_MONTH}${VER_DAY}/src; sh build -ne)
    gzip -9 -c me${VER_YEAR}${VER_MONTH}${VER_DAY}/src/ne > ${NEBIN}
    rm -f ${MEBIN}
    (cd me${VER_YEAR}${VER_MONTH}${VER_DAY}/src; sh build)
    gzip -9 -c me${VER_YEAR}${VER_MONTH}${VER_DAY}/src/me > ${MEBIN}
    rm -rf me${VER_YEAR}${VER_MONTH}${VER_DAY}
fi
#
# Clean up
#    
rm -rf ./package
#
# Build the package directory
# 
mkdir -p package
#
# make the version
# 
# echo ${VERSION} > package/debian-binary
#
# Make the binary directory.
#
mkdir -p package/usr/bin
gunzip -c ${MEBIN} > package/usr/bin/me
chmod a+x package/usr/bin/me
#
# Make the tree.
# 
mkdir -p package/usr/share
(cd package/usr/share; tar zxvf ../../../jasspa-metree-${VERSION}.tar.gz)
#
# Make the manual pages
#
if [ ! -f ./me.1.gz ] ; then
    echo "Cannot find file me.1.gz"
    exit 1
fi
mkdir -p package/usr/share/man/cat1
cp me.1.gz package/usr/share/man/cat1
# 
# Make the documentation
#
mkdir -p package/usr/share/doc/jasspa
cp ${MEPDF} package/usr/share/doc/jasspa/
gzip -9 -c COPYING > package/usr/share/doc/jasspa/COPYING.gz
gzip -9 -c change.log > package/usr/share/doc/jasspa/change.log.gz
gzip -9 -c readme.txt > package/usr/share/doc/jasspa/readme.txt.gz

#
# Permissions
#
chmod og-w ./package/usr
#

#############################################################################
# Control directory                                                         #
#############################################################################

#
# Build the control file 
#
RELSIZE=`du -sk package/usr/ | sed -e "s/[^0-9].*$//"`
mkdir -p package/DEBIAN
echo "Package: jasspa-me" > package/DEBIAN/control
echo "Version: ${VERSION}" >> package/DEBIAN/control
echo "Section: editors" >> package/DEBIAN/control
echo "Priority: optional"  >> package/DEBIAN/control
echo "Architecture: i386" >> package/DEBIAN/control
echo "Depends: libc6 (>= 2.2.4-4) , libncurses5 (>=5.2.20020112a-7) , xlibs (>> 4.1.0)"  >> package/DEBIAN/control
echo "Maintainer: Jon Green <support@jasspa.com>"  >> package/DEBIAN/control
echo "Installed-size: ${RELSIZE}"  >> package/DEBIAN/control
echo "Description: JASSPAs MicroEmacs text editor (X11)" >> package/DEBIAN/control
#
# Build the checksum
#
(cd package; md5sum -b `find usr -depth -type f -print` > DEBIAN/md5sums)
#
# Build postinst
# 
echo "#!/bin/sh" > package/DEBIAN/postinst
echo "set -e" >> package/DEBIAN/postinst
chmod a+x package/DEBIAN/postinst
#
# Build postrm
# 
echo "#!/bin/sh" > package/DEBIAN/postrm
echo "set -e" >> package/DEBIAN/postrm
chmod a+x package/DEBIAN/postrm
#
# Build prerm
# 
echo "#!/bin/sh" > package/DEBIAN/prerm
echo "set -e" >> package/DEBIAN/prerm
chmod a+x package/DEBIAN/prerm
#
# Build the package
# 
chown -R root package
chgrp -R root package
dpkg -b package jasspa-me_20050505_i386.deb 
