#!/bin/sh
#
# Rules to build the Sun package file. We build the executable only.
#
MKDIR=mkdir
DIRECTORIES="company doc icons macros spelling src bin"
SEARCH_PATH="/opt/jasspa/company:/opt/jasspa/macros:/opt/jasspa/spelling"
BASEDIR=.
TOPDIR=../..
VER_YEAR="06"
VER_MONTH="09"
VER_DAY="09"
VERSION="20${VER_YEAR}${VER_MONTH}${VER_DAY}"
METREE=jasspa-metree-${VERSION}.tar.gz
MESRC=jasspa-mesrc-${VERSION}.tar.gz
MEBIN=jasspa-me-cygwin-i386-${VERSION}.gz
MEXBIN=jasspa-mex-cygwin-i386-${VERSION}.gz
NEBIN=jasspa-ne-cygwin-i386-${VERSION}.gz
BASEFILESET="${METREE} ${MEBIN} me.1"

#
## Pull the files over from the release and source areas.
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
    # Build me
    rm -rf me${VER_YEAR}${VER_MONTH}${VER_DAY} 
    gunzip -c ${MESRC} | tar xf -
    (cd me${VER_YEAR}${VER_MONTH}${VER_DAY}/src; sh build -ne)
    rm -f ${NEBIN}
    gzip -9 -c me${VER_YEAR}${VER_MONTH}${VER_DAY}/src/ne.exe > ${NEBIN}
    (cd me${VER_YEAR}${VER_MONTH}${VER_DAY}/src; sh build -S)
    (cd me${VER_YEAR}${VER_MONTH}${VER_DAY}/src; sh build -t cw)
    rm -f ${MEXBIN}
    gzip -9 -c me${VER_YEAR}${VER_MONTH}${VER_DAY}/src/mecw.exe > ${MEXBIN}
    (cd me${VER_YEAR}${VER_MONTH}${VER_DAY}/src; sh build -S)
    (cd me${VER_YEAR}${VER_MONTH}${VER_DAY}/src; sh build -t c)
    rm -f ${MEBIN}
    gzip -9 -c me${VER_YEAR}${VER_MONTH}${VER_DAY}/src/mec.exe > ${MEBIN}
    rm -rf me${VER_YEAR}${VER_MONTH}${VER_DAY}
fi
#
# Test for the starting files.
#
for FILE in $BASEFILESET ; do
    echo checking for ${FILE}
    if [ ! -f ${FILE} ] ; then
        echo "Cannot find file ${FILE}"
        exit 1
    fi
done
#
# Clean-up.
# 
if [ -d usr ] ; then
    rm -rf ./usr
fi    
#
# Build the source bundle.
#
mkdir -p usr/src
gunzip -c ${MESRC} | (cd usr/src; tar xf - )
mv usr/src/me${VER_YEAR}${VER_MONTH}${VER_DAY} usr/src/jasspa-me-${VERSION}
tar cf - ./usr |  bzip2 -9 -c - > jasspa-${VERSION}-src.tar.bz2
#
# Clean-up.
# 
if [ -d usr ] ; then
    rm -rf ./usr
fi    
#
# Build the documentation directory
# 
mkdir -p usr/doc/jasspa
mkdir -p usr/share
gunzip -c ${METREE} | (cd usr/share; tar xf - )
for NAME in build.txt change.log license.txt readme.txt cygwin.txt faq.txt
do
    # Copy the file
    if [ ! -f ${TOPDIR}/release/www/doc/${NAME} ] ; then
        echo "Cannot find ${TOPDIR}/release/www/doc/${NAME}"
        exit 1
    fi
    cp ${TOPDIR}/release/www/doc/${NAME} usr/doc/jasspa/${NAME}
done
#
# Build the cygwin directory    
#
mkdir -p usr/doc/Cygwin
if [ ! -f ${TOPDIR}/release/www/doc/cygwin.txt ] ; then
    echo "Cannot find ${TOPDIR}/release/www/doc/cygwin.txt"
    exit 1
fi
cp ${TOPDIR}/release/www/doc/cygwin.txt usr/doc/Cygwin/jasspa-${VERSION}.README
#
# Build the man directory
#
mkdir -p usr/share/man/cat1
cp me.1 usr/share/man/cat1
#
# Build the binary
# 
mkdir -p usr/bin
gunzip -c ${MEBIN} > usr/bin/me.exe
chmod a+rx usr/bin/me.exe
chmod a-w usr/bin/me.exe
gunzip -c ${MEXBIN} > usr/bin/mex.exe
chmod a+rx usr/bin/mex.exe
chmod a-w usr/bin/mex.exe
#
# Construct the archive
#
tar cf - ./usr |  bzip2 -9 -c - > jasspa-${VERSION}.tar.bz2
#
# Clean-up.
# 
if [ -d usr ] ; then
    rm -rf ./usr
fi    
