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
MEBIN=jasspa-me-sun-sparc-58-${VERSION}.gz
NEBIN=jasspa-ne-sun-sparc-58-${VERSION}.gz
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
if [ ! -f ${MEBIN} ] ; then
    if [ -f ${TOPDIR}/src/${MEBIN} ] ; then
        cp ${TOPDIR}/src/${MEBIN} ${MEBIN}
    elif [ -f ${TOPDIR}/src/me ] ; then
        gzip -9 -c ${TOPDIR}/src/me > ${MEBIN}
    else
        # Build me
        gunzip -c ${MESRC} | tar xf -
        (cd me${VER_YEAR}${VER_MONTH}${VER_DAY}/src; sh build -ne)
        gzip -9 -c me${VER_YEAR}${VER_MONTH}${VER_DAY}/src/ne > ${NEBIN}
        (cd me${VER_YEAR}${VER_MONTH}${VER_DAY}/src; sh build)
        gzip -9 -c me${VER_YEAR}${VER_MONTH}${VER_DAY}/src/me > ${MEBIN}
        rm -rf me${VER_YEAR}${VER_MONTH}${VER_DAY}
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
echo "  zip -9 jasspa-mepkg-sun-sparc-58-${VERSION}.zip jasspa-me"
echo "To test:-"
echo "  mkdir jasspa"
echo "  cd ./jasspa"
echo "  unzip ../jasspa-mepkg-sun-sparc-58-${VERSION}.zip"
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
