#!/bin/sh
set -x
#
# Rules to build the Sun package file. We build the executable only.
#
MKDIR=mkdir
DIRECTORIES="company doc icons macros spelling src bin"
SEARCH_PATH="/opt/jasspa/company:/opt/jasspa/macros:/opt/jasspa/spelling"
BASEDIR="fakeroot"
TOPDIR=../..
VER_YEAR="09"
VER_MONTH="09"
VER_DAY="09"
VERSION="20${VER_YEAR}${VER_MONTH}${VER_DAY}"
#
LVER_YEAR="09"
LVER_MONTH="10"
LVER_DAY="11"
LATEST="20${LVER_YEAR}${LVER_MONTH}${LVER_DAY}"
# Processor type
PROCESSOR=`uname -p`
OSVERSION=`uname -r`
OSNAME=`uname -s`
USERNAME=`/usr/xpg4/bin/id -u -n`
GROUPNAME=`/usr/xpg4/bin/id -g -n`
# Package name
VERSTRING="20${LVER_YEAR}.${LVER_MONTH}.${LVER_DAY}"
PKGNAME=jasspa-me
PKGBASENAME=${PKGNAME}-${OSNAME}${OSVERSION}-${PROCESSOR}-${LATEST}
PKGFILENAME=${PKGBASENAME}.pkg.gz
#
MESRC=jasspa-mesrc-${LATEST}.tar.gz
METREE=jasspa-metree-${LATEST}.tar.gz
MEBIN=jasspa-me-${OSNAME}${OSVERSION}-${PROCESSOR}-${LATEST}.gz
NEBIN=jasspa-ne-${OSNAME}${OSVERSION}-${PROCESSOR}-${LATEST}.gz
BASEFILESET="${METREE} ${MEBIN} me.1"
DOCFILESET="COPYING change.log faq.txt license.txt readme.txt jasspame.pdf jasspa-microemacs.desktop"
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
    (cd me${LVER_YEAR}${LVER_MONTH}${LVER_DAY}/src; sh build -ne)
    gzip -9 -c me${LVER_YEAR}${LVER_MONTH}${LVER_DAY}/src/ne > ${NEBIN}
    (cd me${LVER_YEAR}${LVER_MONTH}${LVER_DAY}/src; sh build)
    gzip -9 -c me${LVER_YEAR}${LVER_MONTH}${LVER_DAY}/src/me > ${MEBIN}
    rm -rf me${LVER_YEAR}${LVER_MONTH}${LVER_DAY}
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
    mkdir -p ${BASEDIR}
    mkdir -p ${BASEDIR}/opt
    mkdir -p ${BASEDIR}/usr/share/applications
    gunzip -c ${METREE} | tar xf -
    mv jasspa ${BASEDIR}/opt
    chmod -R go-w ${BASEDIR}/opt/jasspa
    #
    # Remove the windows specifics
    #
    rm -f ${BASEDIR}/opt/jasspa/contrib/ME_4_all.reg
    rm -f ${BASEDIR}/opt/jasspa/contrib/mesetup.reg
    #
    # Unpack the executable.
    #
    mkdir -p ${BASEDIR}/opt/jasspa/bin
    gunzip -v -c ${MEBIN} > ${BASEDIR}/opt/jasspa/bin/me
    chmod a+rx ${BASEDIR}/opt/jasspa/bin/me
    mkdir -p ${BASEDIR}/opt/jasspa/man/man1
    cp me.1 ${BASEDIR}/opt/jasspa/man/man1
    chmod a-wx ${BASEDIR}/opt/jasspa/man/man1/me.1
    #
    # Build the desktop file
    # 
    cat jasspa-microemacs.desktop | sed -e "s#/usr/share#/opt#g" | sed -e "s#=me#=/opt/jasspa/bin/me#g" > ${BASEDIR}/usr/share/applications/jasspa-microemacs.desktop
    chmod -R go-w ${BASEDIR}/usr
    #
    # Build the proto file
    #
    rm -f ${BASEDIR}/prototype
    rm -f ${BASEDIR}/pkginfo
    echo "i pkginfo=./pkginfo" > ${BASEDIR}/prototype
    (cd ${BASEDIR}; pkgproto usr=/usr opt=/opt | sed -e "s/${USERNAME} ${GROUPNAME}/root bin/g" >> prototype)
    #
    # Build the package info file
    #
    echo 'PKG="'${PKGNAME}'"' > ${BASEDIR}/pkginfo
    echo 'NAME="JASSPA MicroEmacs"' >> ${BASEDIR}/pkginfo
    echo 'ARCH="'${PROCESSOR}'"' >> ${BASEDIR}/pkginfo
    echo 'VERSION="'${LVER_YEAR}.${LVER_MONTH}.${LVER_DAY}'"'  >> ${BASEDIR}/pkginfo
    echo 'SUNW_PKGVERS="1.0"' >> ${BASEDIR}/pkginfo
    echo 'CATEGORY="application"' >> ${BASEDIR}/pkginfo
    echo 'VENDOR="www.jasspa.com"' >> ${BASEDIR}/pkginfo
    echo 'EMAIL="support@jasspa.com"' >> ${BASEDIR}/pkginfo
    echo 'PSTAMP="Jon Green"'  >> ${BASEDIR}/pkginfo
    echo 'CLASSES="none"' >> ${BASEDIR}/pkginfo
    #
    # Build the package.
    #
    rm -rf ${BASEDIR}/${PKGNAME}
    rm -f ${BASEDIR}/${PKGFILENAME}
    rm -f ${BASEDIR}/${PKGNAME}pkg-sun-${PROCESSOR}-${OSVERSION}-${LATEST}.zip
    (cd ${BASEDIR}; /usr/bin/pkgmk -d . -f ./prototype ${PKGNAME})
    (cd ${BASEDIR}; /usr/bin/pkgtrans -o -s . ${PKGBASENAME}.pkg ${PKGNAME})
    gzip -9 -c ${BASEDIR}/${PKGBASENAME}.pkg > ${PKGBASENAME}.pkg.gz
    #
    # Clean up
    #
    rm -f ${BASEDIR}
    rm -rf ./${PKGNAME}
fi        
exit 0
#
# Build the spelling packages.
#
SPELLFILESET="dede enus engb eses fifi frfr itit plpl ptpt ruye ruyo"
for FILE in ${SPELLFILESET} ; do
    # Name of the package
    PKGSPELLFILE=jasspa-${FILE}-${OSNAME}-all-${LATEST}
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
