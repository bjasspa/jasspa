#!/bin/sh
#
#  System        : MicroEmacs
#  Module        : Package Build script.
#  Object Name   : $RCSfile: opt_depot_11.sh,v $
#  Revision      : $Revision: 1.1 $
#  Date          : $Date: 2004-02-06 23:39:20 $
#  Author        : $Author: jon $
#  Created By    : <unknown>
#  Created       : Sun Aug 17 12:58:23 2003
#  Last Modified : <030817.1442>
#
# Rules to build the HPUX 11.xx depot file. We build the executable only.
#
MKDIR=mkdir
DIRECTORIES="company doc icons macros spelling src bin"
SEARCH_PATH="/opt/jasspa/company:/opt/jasspa/macros:/opt/jasspa/spelling"
BASEDIR="opt11"
BASEFILESET="me.ehf.gz meicons.tar.gz memacros.tar.gz mesrc.tar.gz"
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
#
# Build the directories
#
if [ ! -d ${BASEDIR} ] ; then
    mkdir ${BASEDIR}
fi
for FILE in $DIRECTORIES ; do
    if [ ! -d ${BASEDIR}/${FILE} ] ; then
        mkdir ${BASEDIR}/${FILE}
    fi
done
#
# Build me
#
if [ ! -f ${BASEDIR}/bin/me ] ; then
    if [ ! -f ${BASEDIR}/src/${MAKEBAS}.${CCMAK} ] ; then
        gunzip -c mesrc.tar.gz | (cd ${BASEDIR}/src;  tar xvf - )
    fi
    MAKECDEFS="-D_SEARCH_PATH=\\"'"'"${SEARCH_PATH}\\"'"'
    (cd ${BASEDIR}/src; make -f ${MAKEBAS}.${CCMAK} MAKECDEFS=$MAKECDEFS mecw)
    cp ${BASEDIR}/src/mecw ${BASEDIR}/bin/me
    (cd ${BASEDIR}; rm -rf ./src)
    mkdir ${BASEDIR}/src
fi
#
# Build ne
#
if [ ! -f ${BASEDIR}/bin/ne ] ; then
    if [ ! -f ${BASEDIR}/src/${MAKEBAS}.${CCMAK} ] ; then
        gunzip -c mesrc.tar.gz | (cd ${BASEDIR}/src;  tar xvf - )
    fi
    MAKECDEFS="-D_SEARCH_PATH=\\"'"'"${SEARCH_PATH}\\"'"'
    (cd ${BASEDIR}/src; make -f ${MAKEBAS}.${CCMAK} MAKECDEFS=$MAKECDEFS nec)
    cp ${BASEDIR}/src/nec ${BASEDIR}/bin/ne
    (cd ${BASEDIR}/src; make -f ${MAKEBAS}.${CCMAK} spotless)
    (cd ${BASEDIR}; rm -rf ./src)
    mkdir ${BASEDIR}/src
fi
#
# Build the HTML
#
# gunzip -c mehtml.tar.gz | (cd ${BASEDIR}/doc;  tar xvf - )
#
# Build the macros
#
if [ ! -f ${BASEDIR}/macros/me.emf ] ; then
    gunzip -c memacros.tar.gz | (cd ${BASEDIR}/macros; tar xvf - )
fi    
if [ ! -f ${BASEDIR}/macros/me.ehf ] ; then
    gunzip -c me.ehf.gz > ${BASEDIR}/macros/me.ehf
fi    
#
# Build the readme for source files.
#
if [ ! -f ${BASEDIR}/src/readme.txt ] ; then
    echo "For source files go to www.jasspa.com" > ${BASEDIR}/src/readme.txt
fi    
#
# Build the readme for spelling files.
#
if [ ! -f ${BASEDIR}/spelling/readme.txt ] ; then
    echo "For spelling files go to www.jasspa.com" > ${BASEDIR}/spelling/readme.txt
fi    
#
# Build the readme for the company files
#
if [ ! -f ${BASEDIR}/company/readme.txt ] ; then
    echo "Place company wide common files in here" > ${BASEDIR}/company/readme.txt
fi
#
# Build the readme file in here
#
if [ ! -f ${BASEDIR}/doc/readme.txt ] ; then
    echo "For HP-UX then the search path is defined as" > ${BASEDIR}/doc/readme.txt
    echo ${SEARCH_PATH} >> ${BASEDIR}/doc/readme.txt
fi    
#
# Build the icons
#
if [ ! -f ${BASEDIR}/icons/me.xpm ] ; then
    gunzip -c meicons.tar.gz | (cd ${BASEDIR}/icons; tar xvf - )
fi
#
# Build the ".prj" file.
#
echo "As root run the following command to build the depot"
echo '/usr/sbin/swpackage -d "| /usr/contrib/bin/gzip -c > jasspame-opt-0212-11.00.depot.gz" -x target_type=tape -s opt_depot_11.psf'
echo ""
echo "To install:-"
echo "/usr/contrib/bin/gunzip -c jasspame-opt-0212-11.00.depot.gz > jasspame-opt-0212-11.00.depot"
echo "/usr/sbin/swinstall -s `pwd`/jasspame-opt-0212-11.00.depot jasspame"
echo ""
echo "To subsequently remove:-"
echo "/usr/sbin/swremove jasspame"
