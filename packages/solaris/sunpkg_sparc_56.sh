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
MEBIN=jasspa-me-sun-sparc-56-${VERSION}.gz
BASEFILESET="${METREE} ${MEBIN} me.1"

#
# Pull the files over from the release and source areas.
#
if [ ! -f ${METREE} ] ; then
    if [ -f ${TOPDIR}/release/www/${METREE} ] ; then
        cp ${TOPDIR}/release/www/${METREE} .
    fi
fi
if [ ! -f me.1 ] ; then
    if [ -f ${TOPDIR}/release/www/me.1.gz ] ; then
        gunzip -c ${TOPDIR}/release/www/me.1.gz > me.1
    fi
fi
if [ ! -f ${MEBIN} ] ; then
    if [ -f ${TOPDIR}/src/${MEBIN} ] ; then
        cp ${TOPDIR}/src/${MEBIN} ${MEBIN}
    elif [ -f ${TOPDIR}/src/me ] ; then
        gzip -9 -c ${TOPDIR}/src/me > ${MEBIN}
    fi
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
### #
### # Build me
### #
### if [ ! -f ${BASEDIR}/bin/me ] ; then
###     if [ ! -f ${BASEDIR}/src/sunos56.${CCMAK} ] ; then
###         gunzip -c mesrc.tar.gz | (cd ${BASEDIR}/src;  tar xvf - )
###     fi
###     MAKECDEFS="-D_SEARCH_PATH=\\"'"'"${SEARCH_PATH}\\"'"'
###     (cd ${BASEDIR}/src; make -f sunos56.${CCMAK} MAKECDEFS=$MAKECDEFS mecw)
###     cp ${BASEDIR}/src/mecw ${BASEDIR}/bin/me
###     (cd ${BASEDIR}; rm -rf ./src)
###     mkdir ${BASEDIR}/src
### fi
### #
### # Build ne
### #
### if [ ! -f ${BASEDIR}/bin/ne ] ; then
###     if [ ! -f ${BASEDIR}/src/sunos56.${CCMAK} ] ; then
###         gunzip -c mesrc.tar.gz | (cd ${BASEDIR}/src;  tar xvf - )
###     fi
###     MAKECDEFS="-D_SEARCH_PATH=\\"'"'"${SEARCH_PATH}\\"'"'
###     (cd ${BASEDIR}/src; make -f sunos56.${CCMAK} MAKECDEFS=$MAKECDEFS nec)
###     cp ${BASEDIR}/src/nec ${BASEDIR}/bin/ne
###     (cd ${BASEDIR}/src; make -f sunos56.${CCMAK} spotless)
###     (cd ${BASEDIR}; rm -rf ./src)
###     mkdir ${BASEDIR}/src
### fi
#
# Unpack the tree
#
gunzip -c ${METREE} | tar xf -
#
# Unpack the executable.
#
mkdir -p ${BASEDIR}/jasspa/bin
gunzip -v -c ${MEBIN} > ${BASEDIR}/jasspa/bin/me
chmod a+rx ${BASEDIR}/jasspa/bin/me
mkdir -p ${BASEDIR}/jasspa/man/cat1
cp me.1 ${BASEDIR}/jasspa/man/cat1
chmod a-wx ${BASEDIR}/jasspa/man/cat1/me.1
#
# Build the proto file
#
rm -f prototype
rm -f pkginfo
echo "i pkginfo=./pkginfo" > prototype
find ${BASEDIR}/jasspa -print | pkgproto | sed -e "s/jon users/bin bin/g" >> prototype
#
# Build the package info file
#
echo 'PKG="jasspa-me"' > pkginfo
echo 'NAME="JASSPA MicroEmacs"' >> pkginfo
echo 'ARCH="sparc"' >> pkginfo
echo 'VERSION="'${VER_YEAR}.${VER_MONTH}.${VER_DAY}'"'  >> pkginfo
echo 'SUNW_PKGVERS="1.0"' >> pkginfo
echo 'CATEGORY="application"' >> pkginfo
echo 'VENDOR="www.jasspa.com"' >> pkginfo
echo 'EMAIL="support@jasspa.com"' >> pkginfo
echo 'PSTAMP="Jon Green"'  >> pkginfo
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
echo "  pkgtrans -s /var/spool/pkg /tmp/jasspa-me"
echo "  rm -rf ./jasspa-me"
echo "  cd /tmp"
echo "  rm -rf ./jasspapkg"
echo "  zip -9 jasspa-mepkg-sun-sparc-56-${VERSION}.zip jasspa-me"
echo "To test:-"
echo "  mkdir jasspa"
echo "  cd ./jasspa"
echo "  unzip ../jasspa-mepkg-sun-sparc-56-${VERSION}.zip"
echo "  pkgadd -d jasspa-me"
echo "To subsequently remove"
echo "  pkgrm jasspa-me"
echo ""
#
# Clean up
#
rm -f prototype
rm -f pkginfo
rm -rf ./jasspa