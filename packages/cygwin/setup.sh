#!/bin/sh
#
# Rules to build the Sun package file. We build the executable only.
#
MKDIR=mkdir
DIRECTORIES="company doc icons macros spelling src bin"
SEARCH_PATH="/opt/jasspa/company:/opt/jasspa/macros:/opt/jasspa/spelling"
BASEDIR=.
TOPDIR=../..
VER_YEAR="05"
VER_MONTH="05"
VER_DAY="05"
VERSION="20${VER_YEAR}${VER_MONTH}${VER_DAY}"
METREE=jasspa-metree-${VERSION}.tar.gz
MESRC=jasspa-mesrc-${VERSION}.tar.gz
#MEBIN=jasspa-me-sun-sparc-56-${VERSION}.gz
BASEFILESET="${METREE} ${MEBIN} me.1"

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
#if [ ! -f ${MEBIN} ] ; then
#    if [ -f ${TOPDIR}/src/${MEBIN} ] ; then
#        cp ${TOPDIR}/src/${MEBIN} ${MEBIN}
#    elif [ -f ${TOPDIR}/src/me ] ; then
#        gzip -9 -c ${TOPDIR}/src/me > ${MEBIN}
#    fi
#fi
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
gunzip -c ${MESRC} | (cd usr/src; tar xvf - )
mv usr/src/me${VER_YEAR}${VER_MONTH}${VER_DAY} usr/src/jasspa-me-${VERSION}
tar cvf - ./usr |  bzip2 -9 -c - > jasspa-mesrc-cygwin-${VERSION}.tar.bz2
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
gunzip -c ${METREE} | (cd usr/share; tar xvf - )
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
tar cvf - ./usr |  bzip2 -9 -c - > jasspa-mepkg-cygwin-${VERSION}.tar.bz2
#
# Clean-up.
# 
if [ -d usr ] ; then
    rm -rf ./usr
fi    
