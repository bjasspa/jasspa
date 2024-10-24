#!/bin/sh
# Jasspa MicroEmacs install/update script - version <VERSION>
MEBASEURL=https://github.com/bjasspa/jasspa
INSTPATH=""
BINPATH=""
RMSG=""
RC=""

install_path_check(){

  INSTPATH="$1"
  BINPATH=""
  ERRMSG=""
  RC=0
  #echo "Checking install path: $1"
  if [ $ISUPDT -ne 0 ] ; then
    
    if [ ! -e $1/jasspa/meinfo ] ; then
      ERRMSG="\n${2}Error: Installation found at \"$1/jasspa\" was not created by the microemacs-install script, please re-install first."
      return 1
    elif [ ! -w $1/jasspa ] ; then
      ERRMSG="\n${2}Error: Cannot write to install path \"$1/jasspa\", either change ownership/permissions or rerun with sudo."
      return 1
    fi
    INSTPATH=`cd -- "$1" >/dev/null 2>&1 ; pwd -P`
    return 0

  elif [ -e $1/jasspa ] ; then
  
    if [ ! -d $1/jasspa ] ; then
      ERRMSG="\n${2}Error: Install path \"$1/jasspa\" already exists but isn't a directory."
      return 1
    elif [ ! -w $1/jasspa ] ; then
      ERRMSG="\n${2}Error: Cannot write to install path \"$1/jasspa\", either change ownership/permissions or rerun with sudo."
      return 1
    elif [ -e $1/jasspa/meinfo ] ; then
      ERRMSG="\n${2}Warning: MicroEmacs already installed to \"$1/jasspa\", consider using microemacs-update instead."
      RC=2
    elif [ -e $1/jasspa/macros -o -e $1/jasspa/bin ] ; then
      ERRMSG="\n${2}Warning: Install path \"$1/jasspa\" was not created by this installer, continuing will overwrite existing installation."
      RC=2
    fi
  
  elif [ -e $1 ] ; then

    if [ ! -d $1 ] ; then
      ERRMSG="\n${2}Error: Install path \"$1\" already exists but isn't a directory."
      return 1
    elif [ ! -w $1 ] ; then
      ERRMSG="\n${2}Error: Cannot write to path \"$1\", either create directory \"$1/jasspa\" or rerun with sudo."
      return 1
    fi

  else

    # dont like creating the dir here, but it could be a relative path so until it exists its abolute path cannot be obtained (sanely) 
    mkdir -p $1 >/dev/null 2>&1
    if [ ! -d $1 ] ; then
      ERRMSG="\n${2}Error: Cannot create install base path \"$1\", either create directory \"$1/jasspa\" or rerun with sudo."
      return 1
    elif [ ! -w $1 ] ; then
      ERRMSG="\n${2}Error: Cannot create install base path \"$1\", either create directory \"$1/jasspa\" or rerun with sudo."
      return 1
    fi
      
  fi
  # turn path into an absolute path
  INSTPATH=`cd -- "$1" >/dev/null 2>&1 ; pwd -P`
  # calc the non-jasspa bin path
  case "${INSTPATH}" in
    *share) BINPATH=`dirname "${INSTPATH}"`;;
    *) BINPATH="${INSTPATH}";;
  esac
  
  # as ${BINPATH} is ${INSTPATH} or its parent it must exist by now 
  if [ -e ${BINPATH}/bin ] ; then
  
    if [ ! -d ${BINPATH}/bin -o ! -w ${BINPATH}/bin ] ; then
      ERRMSG="${ERRMSG}\n${2}Warning: Cannot create links in \"${BINPATH}/bin\" to binaries in \"${INSTPATH}/jasspa/bin\"."
      RC=2
    fi
  
  elif [ ! -d ${BINPATH} -o ! -w ${BINPATH} ] ; then
    ERRMSG="${ERRMSG}\n${2}Warning: Cannot create links in \"${BINPATH}/bin\" to binaries in \"${INSTPATH}/jasspa/bin\"."
    RC=2
  fi
  return $RC
}
install_package(){
  case $2 in
  binaries) ip=bin_${MEPLATPKG}_$2;;
  openssl) ip=bin_${MEPLATPKG}_$2;;
  *) ip=$2;;
  esac
  echo "Installing: MicroEmacs_${MEVER}_$ip..."
  curl -fsSL -o Jasspa_MicroEmacs_${MEVER}_$ip.zip ${MEURL}/Jasspa_MicroEmacs_${MEVER}_$ip.zip
  if [ $? -ne 0 ]; then
    echo "Error: Failed to download install package \"${MEURL}/Jasspa_MicroEmacs_${MEVER}_$ip.zip\"."
    exit 1
  fi
  unzip -q -o Jasspa_MicroEmacs_${MEVER}_$ip.zip -d ${INSTPATH}/jasspa
  if [ $? -ne 0 ]; then
    echo "Error: Failed to extract install package \"/tmp/Jasspa_MicroEmacs_${MEVER}_$ip.zip\"."
    exit 1
  fi
  rm Jasspa_MicroEmacs_${MEVER}_$ip.zip
  if [ $ip != $2 ]; then
    mv ${INSTPATH}/jasspa/bin/${MEPLATFUL}/* ${INSTPATH}/jasspa/bin/
    rmdir ${INSTPATH}/jasspa/bin/${MEPLATFUL}
  fi
  if grep -q $2 ${INSTPATH}/jasspa/meinfo$1; then
    :
  else
    echo $2 >> ${INSTPATH}/jasspa/meinfo$1
  fi    
}

# First work out the platform and if this is an upgrade
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

# Go through the command line arguments
while [ -n "$1" ] ; do
  case $1 in 
  --help|-h)
      echo "Usage: $0 [-h | --help] [ --prefix=<path> ] [ <package> ]"
      echo "Where:"
      echo "  <path>     Is the install location, defaulting to /usr/local/share"
      echo "             or ~/.local/share depending on permissions."
      echo "  <package>  The package to install, e.g. engb, enus, dede etc for spelling"
      echo "             languages and openssl for https support."
      echo ""
      exit 1;;
  --prefix=*)
      INSTPATH=${1#"--prefix="}
      break;;
  -*) echo "Invalid option $1, use option -h for more information"
      exit 1;;
  *)  break;;
  esac
  shift
done

# Get the latest release version number - no point continuing if can't access github
MELRL=`curl -isS $MEBASEURL/releases/latest/ | grep -i ^location:`
MEVER=`echo "$MELRL" | sed "sX.*/bjasspa/jasspa/releases/tag/me_\\([0-9]*\\)\rX\\1X"`
if [ ${#MEVER} -ne 8 ] ; then
  echo "Error: Failed to obtain version number of latest release (${MELRL})."
  exit 1
fi
MEURL=$MEBASEURL/releases/download/me_${MEVER}

# Now work out where to install/upgrade
if [ -n "${INSTPATH}" ] ; then

  # Install path given by user - remove traliing '/jasspa/' '/' & check permissions etc
  case ${INSTPATH} in
  */jasspa/) INSTPATH=${INSTPATH%"/jasspa/"};;
  */jasspa) INSTPATH=${INSTPATH%"/jasspa"};;
  */) INSTPATH=${INSTPATH%"/"};;
  esac

  install_path_check "${INSTPATH}" ""
  icerr=$?
  if [ $icerr -eq 1 ] ; then
    printf "${ERRMSG}\n\n" | fold -s
    exit 1
  fi
  if [ $ISUPDT -ne 0 ] ; then
    printf "Jasspa MicroEmacs - Updating \"${INSTPATH}/jasspa\" to version ${MEVER}\n" | fold -s
  else
    printf "Jasspa MicroEmacs - Installing version ${MEVER} to \"${INSTPATH}/jasspa\"\n" | fold -s
    if [ $icerr -ne 0 ] ; then
      printf "${ERRMSG}\n\n" | fold -s
    fi
    while true; do
      read -p "Do you want to continue? (y/n) " yn
      case $yn in 
      y) break;;
      n) exit 1;;
      *) echo Invalid response...;;
      esac
    done
  fi
    
elif [ $ISUPDT -ne 0 ] ; then
  
  # No path given, this is an update so get the path from the location of the script
  inspth=`cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P`
  # script should be in a bin directory, remove that then try ${INSTPATH}/share first and then ${INSTPATH} 
  case ${inspth} in
  */bin) inspth=${inspth%"/bin"};;
  esac
  install_path_check "${inspth}/share" ""
  icerr=$?
  if [ $icerr -ne 0 ] ; then
    install_path_check "${inspth}" ""
    icerr=$?
    if [ $icerr -ne 0 ] ; then
      # script not run from within the release, check the 2 default places before giving up
      install_path_check "/usr/local/share" ""
      icerr=$?
      if [ $icerr -ne 0 ] ; then
        install_path_check "${HOME}/.local/share" ""
        icerr=$?
        if [ $icerr -ne 0 ] ; then
          printf "\nError: Failed to locate the Jasspa MicroEmacs installation directory, run with --prefix to set the location.\n\n" | fold -s
          exit 1
        fi
      fi
    fi
  fi
  printf "Jasspa MicroEmacs - Updating \"${INSTPATH}/jasspa\" to version ${MEVER}\n" | fold -s

else

  # No path given, this is an install, check /usr/local/share and ~/.local
  install_path_check "/usr/local/share" "    "
  aicerr=$?
  aermsg=$ERRMSG
  ainpth=$INSTPATH
  abnpth=$BINPATH
  install_path_check "${HOME}/.local/share" "    "
  uicerr=$?
  uermsg=$ERRMSG
  uinpth=$INSTPATH
  ubnpth=$BINPATH

  if [ $aicerr -eq 1 ] ; then
    printf "Cannot install to \"${ainpth}/jasspa\" for all users:\n${aermsg}\n\n" | fold -s

    if [ $uicerr -eq 1 ] ; then
      printf "Cannot install to \"${uinpth}/jasspa\" for current user:\n${uermsg}\n\n" | fold -s
      printf "Please resolve issues for one of the above or use --prefix option to set install location\n\n" | fold -s
      exit 1
    fi
    INSTPATH=${uinpth}
    BINPATH=${ubnpth}
    printf "Install Jasspa MicroEmacs version ${MEVER} to \"${INSTPATH}/jasspa\":\n" | fold -s
    if [ $uicerr -ne 0 ] ; then
      printf "${uermsg}\n\n" | fold -s
    fi
    while true; do
      read -p "Do you want to continue? (y/n) " yn
      case $yn in 
      y) break;;
      n) exit 1;;
      *) echo Invalid response...;;
      esac
    done

  elif [ $uicerr -eq 1 ] ; then
    printf "Cannot install to \"${uinpth}/jasspa\" for current user:\n${uermsg}\n\n" | fold -s
    INSTPATH=${ainpth}
    BINPATH=${abnpth}
    printf "Install Jasspa MicroEmacs version ${MEVER} to \"${INSTPATH}/jasspa\":\n" | fold -s
    if [ $aicerr -ne 0 ] ; then
      printf "${aermsg}\n\n" | fold -s
    fi
    while true; do
      read -p "Do you want to continue? (y/n) " yn
      case $yn in 
      y) break;;
      n) exit 1;;
      *) echo Invalid response...;;
      esac
    done

  else
  
    printf "Jasspa MicroEmacs v${MEVER} can be installed for all users or for just the current user:\n\n" | fold -s
    if [ $aicerr -ne 0 ] ; then
      printf "Select (a) to install to \"${ainpth}\" for all users, however note:\n${aermsg}\n\n" | fold -s
    else
      printf "Select (a) to install to \"${ainpth}\" for all users.\n\n" | fold -s
    fi
    if [ $uicerr -ne 0 ] ; then
      printf "Select (u) to install to \"${uinpth}\" for all users, however note:\n${uermsg}\n\n" | fold -s
    else
      printf "Select (u) to install to \"${uinpth}\" for all users.\n\n" | fold -s
    fi
    while true; do
      read -p "How do you want to continue or (q) to quit? (a/u/q) " auq
      case $auq in 
      a) INSTPATH=${ainpth}
         BINPATH=${abnpth}
         break;;
      u) INSTPATH=${uinpth}
         BINPATH=${ubnpth}
         break;;
      q) exit 1;;
      *) echo Invalid response...;;
      esac
    done
    
  fi
fi

cd /tmp

if [ -z "$1" ] ; then

  if [ $ISUPDT -ne 0 ] ; then
  
    read -r MECRL < ${INSTPATH}/jasspa/meinfo
    if [ "${MECRL}" = "${MEVER}" ] ; then
      printf "\nNote: Installation is already version ${MEVER} - nothing to do.\n\n" | fold -s
      exit 0
    fi
  
    echo "Updating Jasspa MicroEmacs installation from v${MECRL} to v${MEVER}"
  
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
      printf "Error: Failed to download latest update script:\n    \"$MEBASEURL/releases/latest/download/microemacs-install\".\n\n" | fold -s
      exit 1
    fi
    chmod 755 ${INSTPATH}/jasspa/bin/microemacs-update
    printf "Update to ${MEVER} complete and successful.\n\n"
  
  else
  
    printf "\nInstallating Jasspa MicroEmacs v${MEVER} to \"${INSTPATH}/jasspa\"\n" | fold -s
    mkdir -p "${INSTPATH}/jasspa"
    if [ ! -d ${INSTPATH}/jasspa ] ; then
      printf "\nError: Failed to create install path \"${INSTPATH}/jasspa\", either create directory \"${INSTPATH}/jasspa\" or rerun with sudo.\n\n" | fold -s
      return 1
    fi
    echo ${MEVER} > ${INSTPATH}/jasspa/meinfo
  
    #install the core
    install_package "" binaries
    install_package "" macros
    install_package "" help_ehf
    # make the spelling folder so search-path with have it and download latest install script as update
    mkdir -p ${INSTPATH}/jasspa/spelling
    curl -fsSL -o ${INSTPATH}/jasspa/bin/microemacs-update $MEBASEURL/releases/latest/download/microemacs-install
    if [ $? -ne 0 ]; then
      echo "Error: Failed to download latest update script \"$MEBASEURL/releases/latest/download/microemacs-install\"."
      exit 1
    fi
    chmod 755 ${INSTPATH}/jasspa/bin/microemacs-update
  
    if [ -z "${BINPATH}" ] ; then
      BINPATH=${INSTPATH}/jasspa
    else
      if [ ! -e ${BINPATH}/bin ] ; then
        mkdir -p "${BINPATH}/bin" >/dev/null 2>&1
      fi
      if [ ! -d ${BINPATH}/bin -o ! -w ${BINPATH}/bin ] ; then
        printf "\nWarning: Cannot create links in \"${BINPATH}\" to binaries in \"${INSTPATH}/jasspa/bin\", either add ${INSTPATH}/jasspa/bin to your PATH, or copy the scripts to somewhere in your PATH, or rerun with sudo.\n\n" | fold -s
        BINPATH=${INSTPATH}/jasspa
      else
        rm -f ${BINPATH}/bin/mec
        ln -s ${INSTPATH}/jasspa/bin/mec ${BINPATH}/bin/mec
        rm -f ${BINPATH}/bin/mew
        ln -s ${INSTPATH}/jasspa/bin/mew ${BINPATH}/bin/mew
        rm -f ${BINPATH}/bin/tfs
        ln -s ${INSTPATH}/jasspa/bin/tfs ${BINPATH}/bin/tfs
        rm -f ${BINPATH}/bin/microemacs-update
        ln -s ${INSTPATH}/jasspa/bin/microemacs-update ${BINPATH}/bin/microemacs-update
      fi
    fi
    if [ `which mec` = "${BINPATH}/bin/mec" ] ; then
      echo "Installation complete and programmes are in your \$PATH."
      BINPATH=""
    else
      echo "Installation complete, but the programmes are not in your \$PATH. We recommend"
      echo "adding ${BINPATH}/bin to your \$PATH environment variable, e.g. add"
      echo "    PATH=\"${BINPATH}/bin:\$PATH\""
      echo "to the end of your ~/.profile (or .bash_profile, .zprofile etc) file."
      BINPATH="${BINPATH}/bin/"
    fi
    echo "To install spelling support for a language run:"
    echo ""
    echo "   ${BINPATH}microemacs-update <lang-id>"
    echo ""
    echo "where <lang-id> is the 4 character language code, such as \"enus\" or \"dede\" etc."
    echo "To add https support OpenSSL libraries are required, if not already available run:"
    echo ""
    echo "   ${BINPATH}microemacs-update openssl"
    echo ""
    echo "To start using Jasspa MicroEmacs run:"
    echo ""
    echo "   ${BINPATH}mec"
    echo ""
    echo "in a console/terminal, or run:"
    echo ""
    echo "   ${BINPATH}mew"
    echo ""
    echo "for the GUI version, requires a working X (install XQuartz on macOS)."
    echo ""
  
  fi
       
elif [ $1 = "openssl" ] ; then

  echo "Jasspa MicroEmacs - Installing OpenSSL libraries"
  install_package "" openssl

elif [ ${#1} -eq 4 ] ; then

  echo "Jasspa MicroEmacs - Installing spelling $1"
  install_package "" spelling_$1

else

  pkg=`echo "$1" | sed "s/-/_/g"`

  case $pkg in
    binaries )
      echo "Jasspa MicroEmacs - Installing package $pkg"
      install_package "" binaries;;
    help | macros | spelling_*)
      echo "Jasspa MicroEmacs - Installing package $pkg"
      install_package "" $pkg;;
    *)
      echo "Error: Unknown install package \"$1\"."
      exit 1;;
  esac

fi
