#!/bin/sh
#
# Rules to build the Sun package file. We build the executable only.
#
MKDIR=mkdir
DIRECTORIES="company doc icons macros spelling src bin"
SEARCH_PATH="/opt/jasspa/company:/opt/jasspa/macros:/opt/jasspa/spelling"
BASEDIR="jasspa"
BASEFILESET="me.ehf.gz meicons.tar.gz memacros.tar.gz mesrc.tar.gz"
# Set to "mak" for native or "gmk" for GCC
CCMAK=mak
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
    if [ ! -f ${BASEDIR}/src/sunos56.${CCMAK} ] ; then
        gunzip -c mesrc.tar.gz | (cd ${BASEDIR}/src;  tar xvf - )
    fi
    MAKECDEFS="-D_SEARCH_PATH=\\"'"'"${SEARCH_PATH}\\"'"'
    (cd ${BASEDIR}/src; make -f sunos56.${CCMAK} MAKECDEFS=$MAKECDEFS mecw)
    cp ${BASEDIR}/src/mecw ${BASEDIR}/bin/me
    (cd ${BASEDIR}; rm -rf ./src)
    mkdir ${BASEDIR}/src
fi
#
# Build ne
#
if [ ! -f ${BASEDIR}/bin/ne ] ; then
    if [ ! -f ${BASEDIR}/src/sunos56.${CCMAK} ] ; then
        gunzip -c mesrc.tar.gz | (cd ${BASEDIR}/src;  tar xvf - )
    fi
    MAKECDEFS="-D_SEARCH_PATH=\\"'"'"${SEARCH_PATH}\\"'"'
    (cd ${BASEDIR}/src; make -f sunos56.${CCMAK} MAKECDEFS=$MAKECDEFS nec)
    cp ${BASEDIR}/src/nec ${BASEDIR}/bin/ne
    (cd ${BASEDIR}/src; make -f sunos56.${CCMAK} spotless)
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
gunzip -c memacros.tar.gz | (cd ${BASEDIR}/macros; tar xvf - )
gunzip -c me.ehf.gz > ${BASEDIR}/macros/me.ehf
#
# Build the readme for source files.
#
echo "For source files go to www.jasspa.com" > ${BASEDIR}/src/readme.txt
#
# Build the readme for spelling files.
#
echo "For spelling files go to www.jasspa.com" > ${BASEDIR}/spelling/readme.txt
#
# Build the readme for the company files
#
echo "Place company wide common files in here" > ${BASEDIR}/company/readme.txt
#
# Build the readme file in here
#
echo "For sunos then the search path is defined as" > ${BASEDIR}/doc/readme.txt
echo ${SEARCH_PATH} >> ${BASEDIR}/doc/readme.txt
#
# Build the icons
#
gunzip -c meicons.tar.gz | (cd ${BASEDIR}/icons; tar xvf - )
#
# Build the proto file
#
echo "i pkginfo=./pkginfo" > prototype
find ./${BASEDIR} -print | pkgproto | sed -e "s/jon users/bin bin/g" >> prototype
#
# Build the package info file
#
echo 'PKG="jasspa"' > pkginfo
echo 'NAME="MicroEmacs"' >> pkginfo
echo 'ARCH="sparc"' >> pkginfo
echo 'VERSION="02.12.16"'  >> pkginfo
echo 'CATEGORY="application"' >> pkginfo
echo 'VENDOR="www.jasspa.com"' >> pkginfo
echo 'EMAIL="support@jasspa.com"' >> pkginfo
echo 'PSTAMP=Jon Green"'  >> pkginfo
echo 'BASEDIR="/opt"' >> pkginfo
echo 'CLASSES="none"' >> pkginfo
#
# Build all of the information into the /tmp directory.
#
if [ -d /tmp/jasspapkg ] ; then
    (cd /tmp; rm -rf jasspapkg)
fi
mkdir /tmp/jasspapkg
# Copy information.
cp pkginfo /tmp/jasspapkg
cp prototype /tmp/jasspapkg
cp -r jasspa /tmp/jasspapkg
#
# Now echo what we have to do.
#
echo "Now contine the rest as root...."
echo "  cd /tmp/jasspapkg"
echo "  pkgmk -r /tmp/jasspapkg"
echo "  cd /var/spool/pkg"
echo "  pkgtrans -s /var/spool/pkg /tmp/jasspame-20021216"
echo "  rm -rf ./jasspa"
echo "  cd /tmp"
echo "  rm -rf ./jasspapkg"
echo "  zip -9 sun-sparc-9-jasspame-20021216.zip jasspame-20021216"
echo "To test:-"
echo "  mkdir jasspa"
echo "  cd ./jasspa"
echo "  unzip ../sun-sparc-9-jasspame-20021216.zip"
echo "  pkgadd -d jasspame-20021216"
echo "To subsequently remove"
echo "  pkgrm jasspa"
echo ""
