#!/bin/sh
set -x 
VER_YEAR="09"
VER_MONTH="09"
VER_DAY="09"
VERSION="20${VER_YEAR}${VER_MONTH}${VER_DAY}"
#
DOCFILESET="COPYING build.txt change.log patch.txt cygwin.txt faq.txt license.txt readme.txt infolist.txt"
EXEFILESET="jasspa-me-ms-win32-20090909.zip jasspa-mec-ms-win32-20090909.zip jasspa-mecw-ms-win32-20090909.zip jasspa-meicons-ms-win32-20090909.zip"
CHMFILESET="jasspa-mehlp-ms-win32-20090909.zip jasspa-mechm-ms-win32-20090909.zip"
#
JASSPACOM="www.jasspa.com"
#
# Get the HTML files
# 
if [ ! -f jasspa-mehtm-${VERSION}.zip ] ; then
    wget www.jasspa.com/release_${VERSION}/jasspa-mehtm-${VERSION}.zip
    # Unpackage
    unzip -o jasspa-mehtm-${VERSION}.zip
fi    
echo "Run me on me/amicr035.htm"
echo "Within me then execute *me-help-generate-index*"
echo "Save the dile as me.hhk"
# 
# Get the tree file
# 
if [ ! -f jasspa-metree-${VERSION}.zip ] ; then
    wget www.jasspa.com/release_${VERSION}/jasspa-metree-${VERSION}.zip
fi    
# Unpack
rm -rf ./jasspa
rm -rf ./JASSPA
#
# Create the new directories
# 
mkdir -p JASSPA
(cd JASSPA; unzip -o ../jasspa-metree-${VERSION}.zip)
mv JASSPA/jasspa JASSPA/MicroEmacs
#
# Get the document files
#
for FILE in ${DOCFILESET} ; do
    if [ ! -f ${FILE} ] ; then
        wget ${JASSPACOM}/release_${VERSION}/${FILE}
    fi
    unix2dos ${FILE} JASSPA/MicroEmacs/${FILE}
done
#
if [ ! -f jasspame.pdf ] ; then
    wget ${JASSPACOM}/release_${VERSION}/jasspame.pdf
fi
cp jasspame.pdf JASSPA/MicroEmacs/

#
# Get the Windows help files 
# 
for FILE in ${CHMFILESET} ; do
    if [ ! -f ${FILE} ] ; then
        wget ${JASSPACOM}/release_${VERSION}/${FILE}
    fi
    (cd JASSPA/MicroEmacs; unzip -o ../../${FILE})
done
#
# Get the executables 
# 
for FILE in ${EXEFILESET} ; do
    if [ ! -f ${FILE} ] ; then
        wget ${JASSPACOM}/release_${VERSION}/${FILE}
    fi
    (cd JASSPA/MicroEmacs; unzip -o ../../${FILE})
done
#
# Extra executables
#
if [ ! -f jasspa-meunixutils-ms-win32-20090909.zip ] ; then
    wget ${JASSPACOM}/release_${VERSION}/jasspa-meunixutils-ms-win32-20090909.zip
fi        
(cd JASSPA/MicroEmacs; unzip -o ../../jasspa-meunixutils-ms-win32-20090909.zip)
#
# Get the spelling dictionaries.
# 
SPELLFILESET="dede enus engb eses fifi frfr itit plpl ptpt ruye ruyo"
for FILE in ${SPELLFILESET} ; do
    SPELLFILE="ls_${FILE}.zip"
    if [ ! -f ${SPELLFILE} ] ; then
        wget ${JASSPACOM}/spelling/${SPELLFILE}
        if [ ! -f ${SPELLFILE} ] ; then
            echo "Cannot find file ${SPELLFILE}"
            exit 1
        fi
    fi
    (cd JASSPA/MicroEmacs/spelling; unzip -o ../../../${SPELLFILE})   
done        
