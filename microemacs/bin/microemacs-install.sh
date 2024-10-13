#!/bin/sh
BINPATH=/usr/local/bin
INSTPATH=/usr/local/share
MEVER=20240903
MEURL=https://github.com/bjasspa/jasspa/releases/download/me_${MEVER}

install_package(){
  echo "Installing: MicroEmacs_${MEVER}_$1..."
  curl -fsSL -o Jasspa_MicroEmacs_${MEVER}_$1.zip ${MEURL}/Jasspa_MicroEmacs_${MEVER}_$1.zip
  if [ $? -ne 0 ]; then
    echo "Error: Failed to download install package \"${MEURL}/Jasspa_MicroEmacs_${MEVER}_$1.zip\"."
    exit 1
  fi
  unzip -q -o Jasspa_MicroEmacs_${MEVER}_$1.zip -d ${INSTPATH}/jasspa
  if [ $? -ne 0 ]; then
    echo "Error: Failed to extract install package \"/tmp/Jasspa_MicroEmacs_${MEVER}_$1.zip\"."
    exit 1
  fi
  rm Jasspa_MicroEmacs_${MEVER}_$1.zip
  if grep -q $1 ${INSTPATH}/jasspa/meinfo; then
    :
  else
    echo $1 >> ${INSTPATH}/jasspa/meinfo
  fi
    
}

cd /tmp
PLATFORM=`uname`
    
if [ $PLATFORM = "Darwin" ] ; then
  VERSION=`uname -r | cut -f 1 -d .`
  if [ $VERSION -gt 15 ] ; then
    MEPLATPKG=macos
  fi
elif [ $PLATFORM = "Linux" ] ; then
  MEPLATPKG=linux
  MEPLATFUL=linux5-intel64
fi

if [ -z "$MEPLATPKG" ] ; then
  echo "Error: Platform '${PLATFORM}' is not currently supported - please request suport."
  exit 1
fi

if test -e ${INSTPATH}/jasspa ; then

  if [ ! -d ${INSTPATH}/jasspa ] ; then
    echo "Error: Install path \"${INSTPATH}/jasspa\" already exists but isn't a directory."
    exit 1
  fi
  
  if [ ! -w ${INSTPATH}/jasspa ] ; then
    echo "Error: Cannot write to install path \"${INSTPATH}/jasspa\", either change ownership/permissions or run 'sudo $0'."
    exit 1
  elif [ ! -e ${INSTPATH}/jasspa/meinfo ] ; then
    echo "Warning: Install path \"${INSTPATH}/jasspa\" was not created by this installer, continuing will overwrite existing installation."
    while true; do
      read -p "Do you want to continue? (y/n) " yn
      case $yn in 
	y ) break;;
	n ) exit 1;;
	* ) echo Invalid response;;
      esac
    done
    echo ${MEVER} > ${INSTPATH}/jasspa/meinfo
  fi

else

  if [ ! -w ${INSTPATH} ] ; then
    echo "Error: Cannot write to path \"${INSTPATH}\", either create directory \"${INSTPATH}/jasspa\" or run 'sudo $0'."
    exit 1
  fi
  
  mkdir ${INSTPATH}/jasspa

  if [ ! -d ${INSTPATH}/jasspa ] ; then
    echo "Error: Failed to create install path \"${INSTPATH}/jasspa\", either create directory \"${INSTPATH}/jasspa\" or run 'sudo $0'."
    exit 1
  elif [ ! -w ${INSTPATH}/jasspa ]
  then
    echo "Error: Cannot write to install path \"${INSTPATH}/jasspa\", either change ownership/permissions or run 'sudo $0'."
    exit 1
  fi

  echo ${MEVER} > ${INSTPATH}/jasspa/meinfo

fi

if [ -z "$1" ] ; then

  #install the core
  install_package bin_${MEPLATPKG}_binaries
  install_package macros
  install_package help_ehf
  printf "#!/bin/sh\nMEINSTALLPATH=${INSTPATH}/jasspa\n${INSTPATH}/jasspa/bin/${MEPLATFUL}/mec $@\n" > ${INSTPATH}/jasspa/bin/mec
  chmod 755 ${INSTPATH}/jasspa/bin/mec
  printf "#!/bin/sh\nMEINSTALLPATH=${INSTPATH}/jasspa\n${INSTPATH}/jasspa/bin/${MEPLATFUL}/mew $@\n" > ${INSTPATH}/jasspa/bin/mew
  chmod 755 ${INSTPATH}/jasspa/bin/mew
  printf "#!/bin/sh\n${INSTPATH}/jasspa/bin/${MEPLATFUL}/tfs $@\n" > ${INSTPATH}/jasspa/bin/tfs
  chmod 755 ${INSTPATH}/jasspa/bin/tfs

  if [ ! -w ${BINPATH} ] ; then
    echo "Warning: Cannot create links in \"${BINPATH}\" to binaries in \"${INSTPATH}/jasspa/bin\", either add ${INSTPATH}/jasspa/bin to your PATH, run directly from this directory, copy the scripts to somewhere in your PATH or rerun 'sudo $0'."
    exit 1
  fi

  ln -s ${INSTPATH}/jasspa/bin/mec ${BINPATH}/mec
  ln -s ${INSTPATH}/jasspa/bin/mew ${BINPATH}/mew
  ln -s ${INSTPATH}/jasspa/bin/tfs ${BINPATH}/tfs

elif [ $1 = "openssl" ] ; then

  echo "install openssl"
  install_package bin_${MEPLATPKG}_openssl

elif [ ${#1} -eq 4 ] ; then

  echo "install spelling ${#1}"
  install_package spelling_$1

else

  pkg=$(echo "$1" | sed "s/-/_/g")

  case $pkg in
    binaries )
      install_package bin_${MEPLATPKG}_binaries;;
    help | macros | spelling_*)
      install_package $pkg;;
    *)
      echo "Error: Unknown install package \"$1\"."
      exit 1;;
  esac

fi
