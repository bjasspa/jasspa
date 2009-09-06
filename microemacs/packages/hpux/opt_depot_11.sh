#!/bin/sh
#
#  System        : MicroEmacs
#  Module        : Package Build script.
#  Object Name   : $RCSfile: opt_depot_11.sh,v $
#  Revision      : $Revision: 1.12 $
#  Date          : $Date: 2009-09-06 20:00:48 $
#  Author        : $Author: jon $
#  Created By    : <unknown>
#  Created       : Sun Aug 17 12:58:23 2003
#  Last Modified : <090906.2100>
#
# Rules to build the HPUX 11.xx depot file. We build the executable only.
#
MKDIR=mkdir
BASEDIR=.
TOPDIR=../..
VER_YEAR="09"
VER_MONTH="09"
VER_DAY="09"
VERSION="20${VER_YEAR}${VER_MONTH}${VER_DAY}"
METREE=jasspa-metree-${VERSION}.tar.gz
MESRC=jasspa-mesrc-${VERSION}.tar.gz
MEBIN=jasspa-me-hpux-pa-11.00-${VERSION}.gz
NEBIN=jasspa-ne-hpux-pa-11.00-${VERSION}.gz
BASEFILESET="${METREE} ${MEBIN}"
#
DOCFILESET="COPYING change.log faq.txt license.txt readme.txt jasspame.pdf"
SPELLFILESET="enus"
#SPELLFILESET="dede enus engb eses fifi frfr itit plpl ptpt ruye ruyo"
# Set to "mak" for native or "gmk" for GCC
PLATFORM=`uname`
MAKEBAS=hpux11
CCMAK=gmk
DEPOTFILE=opt_depot_11.psf
#
JASSPACOM="www.jasspa.com"
#JASSPACOM="194.164.217.96"
#
# Pull the files over from the release and source areas.
#
if [ ! -f ${METREE} ] ; then
     wget ${JASSPACOM}/release_${VERSION}/${METREE}
    if [ ! -f ${METREE} ] ; then
        echo "Cannot find file ${METREE}"
        exit 1
    fi
fi
# Get the source
if [ ! -f ${MESRC} ] ; then
     wget ${JASSPACOM}/release_${VERSION}/${MESRC}
    if [ ! -f ${MESRC} ] ; then
        echo "Cannot find file ${METREE}"
        exit 1
    fi
fi
# Get the manual page
if [ ! -f me.1 ] ; then
     wget ${JASSPACOM}/release_${VERSION}/me.1
    if [ ! -f me.1 ] ; then
        echo "Cannot find file me.1"
        exit 1
    fi
fi
# Build the binary
if [ ! -f ${MEBIN} ] ; then
    # Build me
    gunzip -c ${MESRC} | tar xf -
    (cd me${VER_YEAR}${VER_MONTH}${VER_DAY}/src; sh build -ne -m hpux11.gmk)
    gzip -9 -c me${VER_YEAR}${VER_MONTH}${VER_DAY}/src/ne > ${NEBIN}
    (cd me${VER_YEAR}${VER_MONTH}${VER_DAY}/src; sh build -m hpux11.gmk)
    gzip -9 -c me${VER_YEAR}${VER_MONTH}${VER_DAY}/src/me > ${MEBIN}
    rm -rf me${VER_YEAR}${VER_MONTH}${VER_DAY}
fi
#
# Get the document files
#
for FILE in ${DOCFILESET} ; do
    if [ ! -f ${FILE} ] ; then
        wget ${JASSPACOM}/release_${VERSION}/${FILE}
    fi        
    if [ ! -f ${FILE} ] ; then
        echo "Cannot find file ${FILE}"
        exit 1
    fi
done
#
# Get the spelling files
#
for FILE in ${SPELLFILESET} ; do
    SPELLFILE="ls_${FILE}.tar.gz"
    if [ ! -f ${SPELLFILE} ] ; then
        wget ${JASSPACOM}/spelling/${SPELLFILE}
    fi        
    if [ ! -f ${SPELLFILE} ] ; then
        echo "Cannot find file ${SPELLFILE}"
        exit 1
    fi
done
#
# Test for the starting files.
#
for FILE in $BASEFILESET ${DOCFILESET} ; do
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
# Remove the windows specifics
#
rm -f ./jasspa/contrib/ME_4_all.reg
rm -f ./jasspa/contrib/mesetup.reg
#
# Expand the documentation files
#
mkdir -p ./jasspa/doc
for FILE in ${DOCFILESET} ; do
    cp ${FILE} ./jasspa/doc
done
chmod -R go-w ./jasspa
#
# Unpack the spelling file
for FILE in ${SPELLFILESET} ; do
    SPELLFILE="ls_${FILE}.tar.gz"
    gunzip -c ${SPELLFILE} | (cd ./jasspa/spelling; tar xvf - )
done    
#
# Unpack the executable.
#
mkdir -p ${BASEDIR}/jasspa/bin
gunzip -v -c ${MEBIN} > ${BASEDIR}/jasspa/bin/me
chmod a+rx ${BASEDIR}/jasspa/bin/me
#
# Build the man directory.
#
mkdir -p ${BASEDIR}/jasspa/man/man1
cp me.1 ${BASEDIR}/jasspa/man/man1
chmod a-wx ${BASEDIR}/jasspa/man/man1/me.1
#
# Build the depot file
#
echo "vendor" > ${DEPOTFILE}
echo "    tag jasspa" >> ${DEPOTFILE}
echo "    description \"JASSPA MicroEmacs text editor www.jasspa.com\"" >> ${DEPOTFILE}
echo "    title \"JASSPA MicroEmacs\"" >> ${DEPOTFILE}
echo "end" >> ${DEPOTFILE}
echo "product" >> ${DEPOTFILE}
echo "    tag jasspa-me" >> ${DEPOTFILE}
echo "    title \"JASSPA MicroEmacs\"" >> ${DEPOTFILE}
echo "    description \"JASSPA MicroEmacs text editor www.jasspa.com\"" >> ${DEPOTFILE}
echo "    # Revision, do not include a leading 0 on minor. i.e. YYYY.[M]MDD" >> ${DEPOTFILE}
echo "    revision 2009.909" >> ${DEPOTFILE}
echo "    #" >> ${DEPOTFILE}
echo "    architecture HP-UX_B.11.00_32/64" >> ${DEPOTFILE}
echo "    machine_type 9000/[78]*:*" >> ${DEPOTFILE}
echo "    os_name HP-UX" >> ${DEPOTFILE}
echo "    os_release ?.11.*" >> ${DEPOTFILE}
echo "    os_version *" >> ${DEPOTFILE}
echo "    copyright \"Copyright (c) 1988-2009 JASSPA - www.jasspa.com\"" >> ${DEPOTFILE}
echo "    #" >> ${DEPOTFILE}
echo "    fileset" >> ${DEPOTFILE}
echo "        tag jasspa" >> ${DEPOTFILE}
echo "        title \"JASSPA MicroEmacs\"" >> ${DEPOTFILE}
echo "        revision 2009.909" >> ${DEPOTFILE}
echo "        postinstall jasspame.postinstall" >> ${DEPOTFILE}
echo "        postremove  jasspame.postremove" >> ${DEPOTFILE}
echo "        #" >> ${DEPOTFILE}
echo "        directory jasspa=/opt/jasspa" >> ${DEPOTFILE}
echo "        file_permissions -u 022 -g bin -o bin" >> ${DEPOTFILE}
echo "        file *" >> ${DEPOTFILE}
echo "    end" >> ${DEPOTFILE}
echo "end" >> ${DEPOTFILE}

#
# Build the documentation directory.
# 
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
