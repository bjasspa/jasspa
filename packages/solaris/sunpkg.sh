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
# Processor type
PROCESSOR=`uname -p`
OSVERSION=`uname -r`
OSNAME=`uname -s`
USERNAME=`/usr/xpg4/bin/id -u -n`
GROUPNAME=`/usr/xpg4/bin/id -g -n`
# Package name
VERSTRING="20${VER_YEAR}.${VER_MONTH}.${VER_DAY}"
PKGNAME=jasspa-me
PKGBASENAME=${PKGNAME}-${OSNAME}${OSVERSION}-${PROCESSOR}-${VERSION}
PKGFILENAME=${PKGBASENAME}.pkg.gz
#
MESRC=jasspa-mesrc-${VERSION}.tar.gz
METREE=jasspa-metree-${VERSION}.tar.gz
MEBIN=jasspa-me-${OSNAME}${OSVERSION}-${PROCESSOR}-${VERSION}.gz
NEBIN=jasspa-ne-${OSNAME}${OSVERSION}-${PROCESSOR}-${VERSION}.gz
BASEFILESET="${METREE} ${MEBIN} me.1"
DOCFILESET="COPYING change.log faq.txt license.txt readme.txt jasspame.pdf"
#
JASSPACOM="www.jasspa.com"
#
# Pull the files over from the release and source areas.
#
if [ ! -f ${METREE} ] ; then
     wget ${JASSPACOM}/release_${VERSION}/${METREE}
fi
if [ ! -f ${MESRC} ] ; then
     wget ${JASSPACOM}/release_${VERSION}/${MESRC}
fi
if [ ! -f me.1 ] ; then
     wget ${JASSPACOM}/release_${VERSION}/me.1
fi
if [ ! -f ${MEBIN} ] ; then
    # Build me
    gunzip -c ${MESRC} | tar xf -
    (cd me${VER_YEAR}${VER_MONTH}${VER_DAY}/src; sh build -ne)
    gzip -9 -c me${VER_YEAR}${VER_MONTH}${VER_DAY}/src/ne > ${NEBIN}
    (cd me${VER_YEAR}${VER_MONTH}${VER_DAY}/src; sh build)
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
done
#
# Test for the starting files.
#
for FILE in $BASEFILESET ${DOCFILESET} ; do
    echo checking for ${FILE}
    if [ ! -f ${FILE} ] ; then
        echo "Cannot find file ${FILE}"
        exit 1
    fi
done
#
# Build the package
# 
if [ ! -f ${PKGFILENAME} ] ; then 
    #
    # Unpack the tree
    #
    gunzip -c ${METREE} | tar xf -
    chmod -R go-w ${BASEDIR}/jasspa
    #
    # Remove the windows specifics
    #
    rm -f ${BASEDIR}/jasspa/contrib/ME_4_all.reg
    rm -f ${BASEDIR}/jasspa/contrib/mesetup.reg
    #
    # Unpack the executable.
    #
    mkdir -p ${BASEDIR}/jasspa/bin
    gunzip -v -c ${MEBIN} > ${BASEDIR}/jasspa/bin/me
    chmod a+rx ${BASEDIR}/jasspa/bin/me
    mkdir -p ${BASEDIR}/jasspa/man/man1
    cp me.1 ${BASEDIR}/jasspa/man/man1
    chmod a-wx ${BASEDIR}/jasspa/man/man1/me.1
    #
    # Build the proto file
    #
    rm -f prototype
    rm -f pkginfo
    echo "i pkginfo=./pkginfo" > prototype
    pkgproto jasspa=/opt/jasspa | sed -e "s/${USERNAME} ${GROUPNAME}/root bin/g" >> prototype
    #
    # Build the package info file
    #
    echo 'PKG="'${PKGNAME}'"' > pkginfo
    echo 'NAME="JASSPA MicroEmacs"' >> pkginfo
    echo 'ARCH="'${PROCESSOR}'"' >> pkginfo
    echo 'VERSION="'${VER_YEAR}.${VER_MONTH}.${VER_DAY}'"'  >> pkginfo
    echo 'SUNW_PKGVERS="1.0"' >> pkginfo
    echo 'CATEGORY="application"' >> pkginfo
    echo 'VENDOR="www.jasspa.com"' >> pkginfo
    echo 'EMAIL="support@jasspa.com"' >> pkginfo
    echo 'PSTAMP="Jon Green"'  >> pkginfo
    echo 'CLASSES="none"' >> pkginfo
    #
    # Build the package.
    #
    rm -rf ./${PKGNAME}
    rm -f ${PKGFILENAME}
    rm -f ${PKGNAME}pkg-sun-${PROCESSOR}-${OSVERSION}-${VERSION}.zip
    /usr/bin/pkgmk -d . -f ./prototype ${PKGNAME}
    /usr/bin/pkgtrans -o -s . ${PKGBASENAME}.pkg ${PKGNAME}
    gzip -9 ${PKGBASENAME}.pkg
    #
    # Clean up
    #
    rm -f prototype
    rm -f pkginfo
    rm -rf ./jasspa
    rm -rf ./${PKGNAME}
fi        
#
# Build the spelling packages.
#
SPELLFILESET="dede enus engb eses fifi frfr itit plpl ptpt ruye ruyo"
for FILE in ${SPELLFILESET} ; do
    # Name of the package
    PKGSPELLFILE=jasspa-${FILE}-${OSNAME}-all-${VERSION}
    if [ ! -f ${PKGSPELLFILE}.pkg.gz  ] ; then 
        # Get the name of the file.
        SPELLFILE="ls_${FILE}.tar.gz"
        if [ ${FILE} = "enus" ] ; then
            SPELLNAME="English (American)"
        elif [ ${FILE} = "engb" ] ; then
            SPELLNAME="English (British)"
        elif [ ${FILE} = "dede" ] ; then
            SPELLNAME="German"
        elif [ ${FILE} = "eses" ] ; then
            SPELLNAME="Spanish"
        elif [ ${FILE} = "fifi" ] ; then
            SPELLNAME="Finnish"
        elif [ ${FILE} = "frfr" ] ; then
            SPELLNAME="French"
        elif [ ${FILE} = "itit" ] ; then
            SPELLNAME="Italian"
        elif [ ${FILE} = "plpl" ] ; then
            SPELLNAME="Polish"
        elif [ ${FILE} = "ptpt" ] ; then
            SPELLNAME="Potuguese"
        elif [ ${FILE} = "ruye" ] ; then
            SPELLNAME="Russian (YE)"
        elif [ ${FILE} = "ruyo" ] ; then
            SPELLNAME="Russian (YO)"
        fi        
        # Download from the server.
        if [ ! -f ${SPELLFILE} ] ; then
            wget ${JASSPACOM}/spelling/${SPELLFILE}
            if [ ! -f ${SPELLFILE} ] ; then
                echo "Cannot find file ${SPELLFILE}"
                exit 1
            fi
        fi
        # Create the directory.
        rm -rf ${BASEDIR}/jasspa
        mkdir -p ${BASEDIR}/jasspa/spelling
        gunzip -c ${SPELLFILE} | (cd ${BASEDIR}/jasspa/spelling; tar xvf - )
        #
        # Build the proto file
        #
        rm -f prototype
        rm -f pkginfo
        rm -f depend
        echo "i pkginfo" > prototype
        echo "i depend"  >> prototype
        pkgproto jasspa=/opt/jasspa | sed -e "s/${USERNAME} ${GROUPNAME}/root bin/g" >> prototype
        # Dependancies
        echo  "P jasspa-me" > depend
        #
        # Build the package info file
        #
        echo 'PKG="jasspa-'${FILE}'"' > pkginfo
        echo 'NAME="jasspa-'${FILE}' - JASSPA MicroEmacs '${SPELLNAME}' Spelling Dictionary ('${FILE}')"' >> pkginfo
        echo 'ARCH="all"' >> pkginfo
        echo 'VERSION="'${VERSTRING}'"'  >> pkginfo
        echo 'SUNW_PKGVERS="1.0"' >> pkginfo
        echo 'CATEGORY="application"' >> pkginfo
        echo 'VENDOR="www.jasspa.com packaged by Jon Green"' >> pkginfo
        echo 'EMAIL="support@jasspa.com"' >> pkginfo
        echo 'PSTAMP="Jon Green"'  >> pkginfo
        echo 'CLASSES="none"' >> pkginfo
        #
        # Build the package.
        #
        rm -rf ${BASEDIR}/jasspa-${FILE}
        rm -f ${PKGSPELLFILE}.pkg
        /usr/bin/pkgmk -d . -f ./prototype jasspa-${FILE}
        /usr/bin/pkgtrans -o -s . ${PKGSPELLFILE}.pkg jasspa-${FILE}
        gzip -9 ${PKGSPELLFILE}.pkg
        rm -rf ./jasspa-${FILE}
        rm -rf ${BASEDIR}/jasspa
    fi        
done
