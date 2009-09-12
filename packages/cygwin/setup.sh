#!/bin/sh
set -x
#
# Rules to build the Sun package file. We build the executable only.
#
MKDIR=mkdir
DIRECTORIES="company doc icons macros spelling src bin"
SEARCH_PATH="/opt/jasspa/company:/opt/jasspa/macros:/opt/jasspa/spelling"
BASEDIR=.
TOPDIR=../..
VER_YEAR="09"
VER_MONTH="09"
VER_DAY="09"
VERSION="20${VER_YEAR}${VER_MONTH}${VER_DAY}"
METREE=jasspa-metree-${VERSION}.tar.gz
MESRC=jasspa-mesrc-${VERSION}.tar.gz
MEBIN=jasspa-me-cygwin-i386-${VERSION}.gz
MEXBIN=jasspa-mex-cygwin-i386-${VERSION}.gz
NEBIN=jasspa-ne-cygwin-i386-${VERSION}.gz
BASEFILESET="${METREE} ${MEBIN} me.1.gz"
DOCFILESET="build.txt change.log license.txt readme.txt cygwin.txt faq.txt COPYING jasspame.pdf"
#
JASSPACOM="www.jasspa.com"
#
# Pull the files over from the release and source areas.
#
if [ ! -f ${METREE} ] ; then
     wget ${JASSPACOM}/release_${VERSION}/${METREE}
    if [ ! -f ${METREE} ] ; then
        exit 1
    fi
fi
if [ ! -f ${MESRC} ] ; then
     wget ${JASSPACOM}/release_${VERSION}/${MESRC}
    if [ ! -f ${MESRC} ] ; then
        exit 1
    fi
fi
if [ ! -f me.1.gz ] ; then
     wget ${JASSPACOM}/release_${VERSION}/me.1.gz
    if [ ! -f me.1.gz ] ; then
        exit 1
    fi
fi
# Build me
if [ ! -f ${MEBIN} ] ; then
    rm -rf me${VER_YEAR}${VER_MONTH}${VER_DAY}
    gunzip -c ${MESRC} | tar xf -
    # Override the makefile if specified.
    if [ -f cygwin.gmk ] ; then
        cp cygwin.gmk me${VER_YEAR}${VER_MONTH}${VER_DAY}/src/
    fi
    # Build the executable
    (cd me${VER_YEAR}${VER_MONTH}${VER_DAY}/src; sh build -S)
    (cd me${VER_YEAR}${VER_MONTH}${VER_DAY}/src; sh build -t c)
    rm -f ${MEBIN}
    gzip -9 -c me${VER_YEAR}${VER_MONTH}${VER_DAY}/src/mec.exe > ${MEBIN}
fi
# Build me
if [ ! -f ${NEBIN} ] ; then
    rm -rf me${VER_YEAR}${VER_MONTH}${VER_DAY}
    gunzip -c ${MESRC} | tar xf -
    # Override the makefile if specified.
    if [ -f cygwin.gmk ] ; then
        cp cygwin.gmk me${VER_YEAR}${VER_MONTH}${VER_DAY}/src/
    fi
    # Build the executable
    (cd me${VER_YEAR}${VER_MONTH}${VER_DAY}/src; sh build -ne)
    rm -f ${NEBIN}
    gzip -9 -c me${VER_YEAR}${VER_MONTH}${VER_DAY}/src/ne.exe > ${NEBIN}
fi
# Build mex
if [ ! -f ${MEXBIN}} ] ; then
    if [ -f /usr/include/X11/Xpm.h ] ; then
        rm -rf me${VER_YEAR}${VER_MONTH}${VER_DAY}
        gunzip -c ${MESRC} | tar xf -
        # Override the makefile if specified.
        if [ -f cygwin.gmk ] ; then
            cp cygwin.gmk me${VER_YEAR}${VER_MONTH}${VER_DAY}/src/
        fi
        # Build the executable
        (cd me${VER_YEAR}${VER_MONTH}${VER_DAY}/src; sh build -t cw)
        rm -f ${MEXBIN}
        gzip -9 -c me${VER_YEAR}${VER_MONTH}${VER_DAY}/src/mecw.exe > ${MEXBIN}
    fi
fi
#
# Get the document files
#
for FILE in ${DOCFILESET} ; do
    if [ ! -f ${FILE} ] ; then
        wget ${JASSPACOM}/release_${VERSION}/${FILE}
    fi
done
#
# Test for the starting files.
#
for FILE in ${BASEFILESET} ${DOCFILESET} ; do
    echo checking for ${FILE}
    if [ ! -f ${FILE} ] ; then
        echo "Cannot find file ${FILE}"
        exit 1
    fi
done
#
# Source file package.
#
if [ -d usr ] ; then
    rm -rf ./usr
fi
if [ ! -f jasspa-${VERSION}-src.tar.bz2 ] ; then
    #
    # Build the source bundle.
    #
    mkdir -p ./usr/src
    gunzip -c ${MESRC} | (cd ./usr/src; tar xf - )
    mv ./usr/src/me${VER_YEAR}${VER_MONTH}${VER_DAY} ./usr/src/jasspa-me-${VERSION}
    tar cf - ./usr |  bzip2 -9 -c - > jasspa-${VERSION}-src.tar.bz2
fi
if [ ! -f jasspa-${VERSION}.tar.bz2 ] ; then
    #
    # Build the documentation directory
    #
    if [ -d ./usr ] ; then
        rm -rf ./usr
    fi
    mkdir -p ./usr/doc/jasspa
    mkdir -p ./usr/share
    gunzip -c ${METREE} | (cd ./usr/share; tar xf - )
    for NAME in ${DOCFILESET} ; do
        cp ${NAME} ./usr/doc/jasspa/${NAME}
    done
    #
    # Build the cygwin directory
    #
    mkdir -p ./usr/doc/Cygwin
    cp cygwin.txt ./usr/doc/Cygwin/jasspa-${VERSION}.README
    #
    # Build the man directory
    #
    mkdir -p ./usr/share/man/cat1
    cp me.1.gz ./usr/share/man/cat1
    #
    # Build the binary
    #
    mkdir -p ./usr/bin
    gunzip -c ${MEBIN} > ./usr/bin/me.exe
    chmod a+rx ./usr/bin/me.exe
    chmod a-w ./usr/bin/me.exe
    if [ -f ${MEXBIN} ] ; then
        if [ -f /usr/include/X11/Xpm.h ] ; then
            gunzip -c ${MEXBIN} > ./usr/bin/mex.exe
            chmod a+rx ./usr/bin/mex.exe
            chmod a-w ./usr/bin/mex.exe
        fi
    fi
    #
    # Construct the archive
    #
    tar cf - ./usr |  bzip2 -9 -c - > jasspa-${VERSION}.tar.bz2
    #
    # Clean-up.
    #
    if [ -d ./usr ] ; then
        rm -rf ./usr
    fi
fi
