#!/bin/sh
BINPATH=/usr/local/bin
INSTPATH=/usr/local/share
MEBASEURL=https://github.com/bjasspa/jasspa

install_package(){
  echo "Installing: MicroEmacs_${MEVER}_$2..."
  curl -fsSL -o Jasspa_MicroEmacs_${MEVER}_$2.zip ${MEURL}/Jasspa_MicroEmacs_${MEVER}_$2.zip
  if [ $? -ne 0 ]; then
    echo "Error: Failed to download install package \"${MEURL}/Jasspa_MicroEmacs_${MEVER}_$2.zip\"."
    exit 1
  fi
  unzip -q -o Jasspa_MicroEmacs_${MEVER}_$2.zip -d ${INSTPATH}/jasspa
  if [ $? -ne 0 ]; then
    echo "Error: Failed to extract install package \"/tmp/Jasspa_MicroEmacs_${MEVER}_$2.zip\"."
    exit 1
  fi
  rm Jasspa_MicroEmacs_${MEVER}_$2.zip
  if grep -q $2 ${INSTPATH}/jasspa/meinfo$1; then
    :
  else
    echo $2 >> ${INSTPATH}/jasspa/meinfo$1
  fi
    
}

cd /tmp
PLATFORM=`uname`
    
if [ $PLATFORM = "Darwin" ] ; then
  VERSION=`uname -r | cut -f 1 -d .`
  if [ $VERSION -gt 15 ] ; then
    if [ `uname -p` = "i386" ] ; then
      MEPLATPKG=macos_intel
      MEPLATFUL=macos13-intel64
    else
      MEPLATPKG=macos_apple
      MEPLATFUL=macos14-apple64
    fi
  fi
elif [ $PLATFORM = "Linux" ] ; then
  MEPLATPKG=linux
  MEPLATFUL=linux5-intel64
fi

if [ -z "$MEPLATPKG" ] ; then
  echo "Error: Platform '${PLATFORM}' is not currently supported - please request suport."
  exit 1
fi

case $0 in
 *microemacs-update*) ISUPDT=1;;
 *) ISUPDT=0;;
esac

# Get the latest release version number
MELRL=`curl -isS $MEBASEURL/releases/latest/ | grep ^location:`
MEVER=`echo "$MELRL" | sed "sX.*/bjasspa/jasspa/releases/tag/me_\\([0-9]*\\)\rX\\1X"`
if [ ${#MEVER} -ne 8 ] ; then
  echo "Error: Failed to obtain version number of latest release (${MELRL})."
  exit 1
fi
MEURL=$MEBASEURL/releases/download/me_${MEVER}

if test -e ${INSTPATH}/jasspa ; then

  if [ ! -d ${INSTPATH}/jasspa ] ; then
    echo "Error: Install path \"${INSTPATH}/jasspa\" already exists but isn't a directory."
    exit 1
  fi
  
  if [ ! -w ${INSTPATH}/jasspa ] ; then
    echo "Error: Cannot write to install path \"${INSTPATH}/jasspa\", either change ownership/permissions or rerun with sudo."
    exit 1
  elif [ ! -e ${INSTPATH}/jasspa/meinfo ] ; then

    if [ $ISUPDT -ne 0 ] ; then
      echo "Error: Installation found at \"${INSTPATH}/jasspa\" was not created but the microemacs-install script, please re-install first."
      exit 1
    fi
    
    echo "Warning: Install path \"${INSTPATH}/jasspa\" was not created by this installer, continuing will overwrite existing installation."
    while true; do
      read -p "Do you want to continue? (y/n) " yn
      case $yn in 
	y) break;;
	n) exit 1;;
	*) echo Invalid response;;
      esac
    done
    echo ${MEVER} > ${INSTPATH}/jasspa/meinfo
  fi

elif [ $ISUPDT -ne 0 ] ; then

  echo "Error: Installation not found, expected to find directory \"${INSTPATH}/jasspa\", please install first."
  exit 1

else

  if [ ! -w ${INSTPATH} ] ; then
    echo "Error: Cannot write to path \"${INSTPATH}\", either create directory \"${INSTPATH}/jasspa\" or rerun with sudo."
    exit 1
  fi
  
  mkdir ${INSTPATH}/jasspa

  if [ ! -d ${INSTPATH}/jasspa ] ; then
    echo "Error: Failed to create install path \"${INSTPATH}/jasspa\", either create directory \"${INSTPATH}/jasspa\" or rerun with sudo."
    exit 1
  elif [ ! -w ${INSTPATH}/jasspa ]
  then
    echo "Error: Cannot write to install path \"${INSTPATH}/jasspa\", either change ownership/permissions or rerun with sudo."
    exit 1
  fi

  echo ${MEVER} > ${INSTPATH}/jasspa/meinfo

fi

if [ -z "$1" ] ; then

  if [ $ISUPDT -ne 0 ] ; then
  
    read -r MECRL < ${INSTPATH}/jasspa/meinfo
    if [ "${MECRL}" = "${MEVER}" ] ; then
      echo "Jasspa MicroEmacs - installation is ${MEVER} which is up-to-date."
      exit 0
    fi
  
    echo "Jasspa MicroEmacs - Updating installation from ${MECRL} to ${MEVER}"
  
    echo ${MEVER} > ${INSTPATH}/jasspa/meinfo.upd
    skipped1=0    
    while read ln; do
      if [ $skipped1 -ne 0 ] ; then
        install_package ".upd" $ln
      fi
      skipped1=1    
    done <${INSTPATH}/jasspa/meinfo
  
    mv ${INSTPATH}/jasspa/meinfo.upd ${INSTPATH}/jasspa/meinfo  
    curl -fsSL -o ${INSTPATH}/jasspa/bin/microemacs-update $MEBASEURL/releases/latest/download/microemacs-install
    if [ $? -ne 0 ]; then
      echo "Error: Failed to download latest update script \"$MEBASEURL/releases/latest/download/microemacs-install\"."
      exit 1
    fi
    chmod 755 ${INSTPATH}/jasspa/bin/microemacs-update
    
  else
  
    echo "Jasspa MicroEmacs - Installing version ${MEVER}"
    echo "This will install the core components to path \"${INSTPATH}/jasspa\"."
    while true; do
      read -p "Do you want to continue? (y/n) " yn
      case $yn in 
        y) break;;
        n) exit 1;;
        *) echo Invalid response;;
      esac
    done
    #install the core
    install_package "" bin_${MEPLATPKG}_binaries
    install_package "" macros
    install_package "" help_ehf
    # make the spelling folder so search-path with have it
    mkdir -p ${INSTPATH}/jasspa/spelling
    printf "#!/bin/sh\nMEINSTALLPATH=${INSTPATH}/jasspa\n${INSTPATH}/jasspa/bin/${MEPLATFUL}/mec $@\n" > ${INSTPATH}/jasspa/bin/mec
    chmod 755 ${INSTPATH}/jasspa/bin/mec
    printf "#!/bin/sh\nMEINSTALLPATH=${INSTPATH}/jasspa\n${INSTPATH}/jasspa/bin/${MEPLATFUL}/mew $@\n" > ${INSTPATH}/jasspa/bin/mew
    chmod 755 ${INSTPATH}/jasspa/bin/mew
    printf "#!/bin/sh\n${INSTPATH}/jasspa/bin/${MEPLATFUL}/tfs $@\n" > ${INSTPATH}/jasspa/bin/tfs
    chmod 755 ${INSTPATH}/jasspa/bin/tfs
    curl -fsSL -o ${INSTPATH}/jasspa/bin/microemacs-update $MEBASEURL/releases/latest/download/microemacs-install
    if [ $? -ne 0 ]; then
      echo "Error: Failed to download latest update script \"$MEBASEURL/releases/latest/download/microemacs-install\"."
      exit 1
    fi
    chmod 755 ${INSTPATH}/jasspa/bin/microemacs-update
  
    if [ ! -w ${BINPATH} ] ; then
      echo "Warning: Cannot create links in \"${BINPATH}\" to binaries in \"${INSTPATH}/jasspa/bin\", either add ${INSTPATH}/jasspa/bin to your PATH, or copy the scripts to somewhere in your PATH, or rerun with sudo."
      echo ""
    else
      rm -f ${BINPATH}/mec
      ln -s ${INSTPATH}/jasspa/bin/mec ${BINPATH}/mec
      rm -f ${BINPATH}/mew
      ln -s ${INSTPATH}/jasspa/bin/mew ${BINPATH}/mew
      rm -f ${BINPATH}/tfs
      ln -s ${INSTPATH}/jasspa/bin/tfs ${BINPATH}/tfs
      rm -f ${BINPATH}/microemacs-update
      ln -s ${INSTPATH}/jasspa/bin/microemacs-update ${BINPATH}/microemacs-update
    fi
  
    echo "Installation complete, to install spelling support for a language run:"
    echo ""
    echo "   ${INSTPATH}/jasspa/bin/microemacs-update <lang-id>"
    echo ""
    echo "where <lang-id> is the 4 character language code, such as \"enus\" or \"dede\" etc."
    echo "To add https support OpenSSL libraries are required, if not already available run:"
    echo ""
    echo "   ${INSTPATH}/jasspa/bin/microemacs-update openssl"
    echo ""
    
  fi
       
elif [ $1 = "openssl" ] ; then

  echo "Jasspa MicroEmacs - Installing OpenSSL libraries"
  install_package "" bin_${MEPLATPKG}_openssl

elif [ ${#1} -eq 4 ] ; then

  echo "Jasspa MicroEmacs - Installing spelling $1"
  install_package "" spelling_$1

else

  pkg=`echo "$1" | sed "s/-/_/g"`

  case $pkg in
    binaries )
      echo "Jasspa MicroEmacs - Installing package $pkg"
      install_package "" bin_${MEPLATPKG}_binaries;;
    help | macros | spelling_*)
      echo "Jasspa MicroEmacs - Installing package $pkg"
      install_package "" $pkg;;
    *)
      echo "Error: Unknown install package \"$1\"."
      exit 1;;
  esac

fi
