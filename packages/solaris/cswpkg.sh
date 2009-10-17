#!/bin/sh
set -x
#
# Rules to build the Sun CSW package file. We build the executable only.
#
MKDIR=mkdir
DIRECTORIES="company doc icons macros spelling src bin"
SEARCH_PATH="/opt/csw/share/jasspa/company:/opt/csw/share/jasspa/macros:/opt/csw/share/jasspa/spelling"
BASEDIR=.
TOPDIR=../..
#
# Base version
#
BVER_YEAR="09"
BVER_MONTH="09"
BVER_DAY="09"
BVERSION="20${BVER_YEAR}${BVER_MONTH}${BVER_DAY}"
#
# Macro version
#
MVER_YEAR="09"
MVER_MONTH="10"
MVER_DAY="17"
MVERSION="20${MVER_YEAR}${MVER_MONTH}${MVER_DAY}"
#
# Source version
# 
SVER_YEAR="09"
SVER_MONTH="10"
SVER_DAY="11"
SVERSION="20${SVER_YEAR}${SVER_MONTH}${SVER_DAY}"
#
# Compute the final version
# 
if [ ${MVERSION} -gt  ${BVERSION} ] ; then 
    if [ ${SVERSION} -gt  ${MVERSION} ] ; then     
        VERSION=${SVERSION}
        VER_YEAR=${SVER_YEAR}
        VER_MONTH=${SVER_MONTH}
        VER_DAY=${SVER_DAY}
    else
        VERSION=${MVERSION}                            
        VER_YEAR=${MVER_YEAR}
        VER_MONTH=${MVER_MONTH}
        VER_DAY=${MVER_DAY}
    fi        
else
    VERSION=${BVERSION}
    VER_YEAR=${BVER_YEAR}
    VER_MONTH=${BVER_MONTH}
    VER_DAY=${BVER_DAY}
fi            
#
# Processor type
PROCESSOR=`uname -p`
OSVERSION=`uname -r`
OSNAME=`uname -s`
# Package name
VERSTRING="20${VER_YEAR}.${VER_MONTH}.${VER_DAY},REV=20${VER_YEAR}.${VER_MONTH}.${VER_DAY}"
PKGNAME=jasspame
CSWNAME=CSW${PKGNAME}
CSWFILE="${PKGNAME}-${VERSTRING}-${OSNAME}${OSVERSION}-${PROCESSOR}-CSW"
#
MESRC=jasspa-mesrc-${SVERSION}.tar.gz
METREE=jasspa-metree-${MVERSION}.tar.gz
MEBIN=jasspa-me-sun-${PROCESSOR}-${OSVERSION}-${SVERSION}.gz
NEBIN=jasspa-ne-sun-${PROCESSOR}-${OSVERSION}-${SVERSION}.gz
BASEFILESET="${METREE} ${MESRC} me.1 jasspa-microemacs.desktop"
DOCFILESET="COPYING change.log faq.txt license.txt readme.txt jasspame.pdf"
#
JASSPACOM="www.jasspa.com"
#
# Pull the files over from the release and source areas.
#
for FILE in ${BASEFILESET} ; do 
    if [ ! -f ${FILE} ] ; then
         wget ${JASSPACOM}/release_${BVERSION}/${FILE}
    fi
    if [ ! -f ${FILE} ] ; then
         wget ${JASSPACOM}/development/${FILE}
    fi
    if [ ! -f ${FILE} ] ; then
        echo "Cannot find file ${FILE}"
        exit 1
    fi
done    
#
# Get the document files
#
for FILE in ${DOCFILESET} ; do
    if [ ! -f ${FILE} ] ; then
        wget ${JASSPACOM}/release_${BVERSION}/${FILE}
    fi
    if [ ! -f ${FILE} ] ; then
        echo "Cannot find file ${FILE}"
        exit 1
    fi
done
#
if [ ! -f ${MEBIN} ] ; then
    # Build me
    gunzip -c ${MESRC} | tar xf -
    cp suncsw.mak me${SVER_YEAR}${SVER_MONTH}${SVER_DAY}/src
    cp suncsw86.mak me${SVER_YEAR}${SVER_MONTH}${SVER_DAY}/src
    if [ ${PROCESSOR} = sparc ] ; then
        (cd me${SVER_YEAR}${SVER_MONTH}${SVER_DAY}/src; make -f suncsw.mak)
    else
        (cd me${SVER_YEAR}${SVER_MONTH}${SVER_DAY}/src; make -f suncsw86.mak)
    fi
    gzip -9 -c me${SVER_YEAR}${SVER_MONTH}${SVER_DAY}/src/me > ${MEBIN}
    rm -rf me${SVER_YEAR}${SVER_MONTH}${SVER_DAY}
fi
#
# Build the package
# 
if [ ! -f ${CSWFILE}.pkg.gz ] ; then 
    #
    # Unpack the executable.
    #
    mkdir -p ${BASEDIR}/csw/bin
    gunzip -v -c ${MEBIN} > ${BASEDIR}/csw/bin/me
    chmod a+rx ${BASEDIR}/csw/bin/me
    mkdir -p ${BASEDIR}/csw/share/man/man1
    cp me.1 ${BASEDIR}/csw/share/man/man1
    chmod a-wx ${BASEDIR}/csw/share/man/man1/me.1
    mkdir -p ${BASEDIR}/csw/share/doc/jasspa
    for FILE in ${DOCFILESET} ; do
        cp ${FILE} ${BASEDIR}/csw/share/doc/jasspa
    done
    #
    # Unpack the tree
    #
    mkdir -p ${BASEDIR}/csw/share
    gunzip -c ${METREE} | (cd csw/share; tar xf - )
    #
    # Remove the windows specifics
    #
    rm -f ${BASEDIR}/csw/share/jasspa/contrib/ME_4_all.reg
    rm -f ${BASEDIR}/csw/share/jasspa/contrib/mesetup.reg
    #
    # Build the desktop file
    # 
    mkdir -p ${BASEDIR}/csw/share/applications
    cat jasspa-microemacs.desktop | sed -e "s#/usr/share#/opt/csw/share#g" | sed -e "s#=me#=/opt/csw/bin/me#g"  | sed -e  "s/Development;TextEditor;/Application;Development;/g" > ${BASEDIR}/csw/share/applications/jasspa-microemacs.desktop
    chmod -R go-w ${BASEDIR}/csw/share/applications
    #
    # Build the proto file
    #
    rm -f prototype
    rm -f pkginfo
    rm -f depend
    echo "i pkginfo" > prototype
    echo "i depend"  >> prototype
    pkgproto csw=/opt/csw | sed -e "s/jon users/root bin/g" | sed -e "s/jon csw/root bin/g" >> prototype
    # Dependancies
    echo "P CSWlibxpm"  > depend
    #
    # Build the package info file
    #
    echo 'PKG="'${CSWNAME}'"' > pkginfo
    echo 'NAME="jasspame - JASSPA MicroEmacs"' >> pkginfo
    echo 'ARCH="'${PROCESSOR}'"' >> pkginfo
    echo 'VERSION="'${VERSTRING}'"'  >> pkginfo
    echo 'SUNW_PKGVERS="1.0"' >> pkginfo
    echo 'CATEGORY="application"' >> pkginfo
    echo 'VENDOR="http://www.jasspa.com packaged for CSW by Jon Green"' >> pkginfo
    echo 'HOTLINE=http://www.opencsw.org/bugtrack/' >> pkginfo
    echo 'EMAIL="jon@opencsw.com"' >> pkginfo
    echo 'PSTAMP="Jon Green"'  >> pkginfo
    #echo 'BASEDIR="/"' >> pkginfo
    #
    # Build the package.
    #
    rm -rf ./${CSWNAME}
    rm -f ${CSWFILE}.pkg
    /usr/bin/pkgmk -d . -f ./prototype ${CSWNAME}
    /usr/bin/pkgtrans -o -s . ${CSWFILE}.pkg ${CSWNAME}
    gzip -9 ${CSWFILE}.pkg
    #
    # Clean up
    #
    rm -f prototype
    rm -f pkginfo
    rm -f depend
    rm -rf ./jasspa
    rm -rf ./${CSWNAME}
    rm -rf ${BASEDIR}/csw
fi        
#rm -f ${PKGNAME}-sun-${PROCESSOR}-${OSVERSION}-${VERSION}.pkg
#
# Build the spelling packages.
#
SPELLFILESET="dede enus engb eses fifi frfr itit plpl ptpt ruye ruyo"
for FILE in ${SPELLFILESET} ; do
    # Name of the package
    CSWSPELLFILE=${PKGNAME}${FILE}-${VERSTRING}-${OSNAME}5.8-all-CSW
    if [ ! -f ${CSWSPELLFILE}.pkg.gz  ] ; then 
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
        rm -rf ${BASEDIR}/csw
        mkdir -p ${BASEDIR}/csw/share/jasspa/spelling
        gunzip -c ${SPELLFILE} | (cd ${BASEDIR}/csw/share/jasspa/spelling; tar xvf - )
        #
        # Build the proto file
        #
        rm -f prototype
        rm -f pkginfo
        rm -f depend
        echo "i pkginfo" > prototype
        echo "i depend"  >> prototype
        pkgproto csw=/opt/csw | sed -e "s/jon users/root bin/g" | sed -e "s/jon csw/root bin/g" >> prototype
        # Dependancies
        echo  "P CSWjasspame" > depend
        #
        # Build the package info file
        #
        echo 'PKG="'${CSWNAME}${FILE}'"' > pkginfo
        echo 'NAME="jasspame'${FILE}' - JASSPA MicroEmacs '${SPELLNAME}' Spelling Dictionary ('${FILE}')"' >> pkginfo
        echo 'ARCH="all"' >> pkginfo
        echo 'VERSION="'${VERSTRING}'"'  >> pkginfo
        echo 'SUNW_PKGVERS="1.0"' >> pkginfo
        echo 'CATEGORY="application"' >> pkginfo
        echo 'VENDOR="http://www.jasspa.com packaged for CSW by Jon Green"' >> pkginfo
        echo 'HOTLINE=http://www.opencsw.org/bugtrack/' >> pkginfo
        echo 'EMAIL="jon@opencsw.com"' >> pkginfo
        echo 'PSTAMP="Jon Green"'  >> pkginfo
        #echo 'BASEDIR="/"' >> pkginfo
        #
        # Build the package.
        #
        rm -rf ./${CSWNAME}${FILE}
        rm -f ${CSWSPELLFILE}.pkg
        /usr/bin/pkgmk -d . -f ./prototype ${CSWNAME}${FILE}
        /usr/bin/pkgtrans -o -s . ${CSWSPELLFILE}.pkg ${CSWNAME}${FILE}
        gzip -9 ${CSWSPELLFILE}.pkg
        rm -rf ./${CSWNAME}${FILE}
        rm -rf ${BASEDIR}/csw    
fi        
done
