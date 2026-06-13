#!/bin/sh
# Jasspa MicroEmacs install/update script - version <VERSION>
MEBASEURL=https://github.com/bjasspa/jasspa
INSTTYPE=""
INSTPATH=""
INSTRPTH=""
INSTBPTH="/bin"
BINPATH=""
ERRMSG=""
RC=""

install_path_check(){

  INSTPATH="$1"
  BINPATH=""
  ERRMSG=""
  RC=0
  #echo "Checking install path: $1"
  if [ $ISUPDT -ne 0 ] ; then
    if [ ! -e "${INSTPATH}" ] ; then
      ERRMSG="\n${2}Error: Installation path \"${INSTPATH}\" not found, please re-install first."
      return 1
    elif [ ! -e "${INSTPATH}${INSTRPTH}/meinfo" ] ; then
      ERRMSG="\n${2}Error: Installation found at \"${INSTPATH}${INSTRPTH}\" was not created by the microemacs-install script, please re-install first."
      return 2
    elif [ ! -w "${INSTPATH}" ] ; then
      ERRMSG="\n${2}Error: Cannot write to install path \"${INSTPATH}\", either change ownership/permissions or rerun with sudo."
      return 2
    fi
    INSTPATH=`cd -- "${INSTPATH}" >/dev/null 2>&1 ; pwd -P`
    return 0
  fi  

  INSPPATH=`dirname ${INSTPATH}`
  #echo "Checking install parent path: ${INSPPATH}"
  if [ -e "${INSTPATH}" ] ; then
  
    if [ ! -d "${INSTPATH}" ] ; then
      ERRMSG="\n${2}Error: Install path \"${INSTPATH}\" already exists but isn't a directory."
      return 1
    elif [ ! -w "${INSTPATH}" ] ; then
      ERRMSG="\n${2}Error: Cannot write to install path \"${INSTPATH}\", either change ownership/permissions or rerun with sudo."
      return 1
    elif [ -e "${INSTPATH}/meinfo" ] ; then
      ERRMSG="\n${2}Warning: MicroEmacs already installed to \"${INSTPATH}\", consider using microemacs-update instead."
      RC=2
    elif [ -e "${INSTPATH}/macros" -o -e "${INSTPATH}/bin" ] ; then
      ERRMSG="\n${2}Warning: Install path \"${INSTPATH}\" was not created by this installer, continuing will overwrite existing installation."
      RC=2
    fi
  
  elif [ -e "${INSPPATH}" ] ; then

    if [ ! -d "${INSPPATH}" ] ; then
      ERRMSG="\n${2}Error: Install path \"${INSPPATH}\" already exists but isn't a directory."
      return 1
    elif [ ! -w "${INSPPATH}" ] ; then
      ERRMSG="\n${2}Error: Cannot write to path \"${INSPPATH}\", either create directory \"${INSTPATH}\" or rerun with sudo."
      return 1
    fi

  else

    # dont like creating the dir here, but it could be a relative path so until it exists its abolute path cannot be obtained (sanely) 
    mkdir -p "${INSPPATH}" >/dev/null 2>&1
    if [ ! -d "${INSPPATH}" ] ; then
      ERRMSG="\n${2}Error: Cannot create install base path \"${INSPPATH}\", either create directory \"${INSTPATH}\" or rerun with sudo."
      return 1
    elif [ ! -w "${INSPPATH}" ] ; then
      ERRMSG="\n${2}Error: Cannot create install base path \"${INSPPATH}\", either create directory \"${INSTPATH}\" or rerun with sudo."
      return 1
    fi
    
  fi
  # turn path into an absolute path
  INSPPATH=`cd -- "${INSPPATH}" >/dev/null 2>&1 ; pwd -P`
  INSTPATH="${INSPPATH}/$(basename "${INSTPATH}")"
  
  # calc the non-jasspa bin path
  case "${INSPPATH}" in
    *share) BINPATH=`dirname "${INSPPATH}"`;;
    *) BINPATH="${INSPPATH}";;
  esac
  
  # as ${BINPATH} is ${INSTPATH} or its parent it must exist by now 
  if [ -e ${BINPATH}/bin ] ; then
  
    if [ ! -d ${BINPATH}/bin -o ! -w ${BINPATH}/bin ] ; then
      ERRMSG="${ERRMSG}\n${2}Warning: Cannot create links in \"${BINPATH}/bin\" to binaries in \"${INSTPATH}/bin\"."
      RC=2
    fi
  
  elif [ ! -d ${BINPATH} -o ! -w ${BINPATH} ] ; then
    ERRMSG="${ERRMSG}\n${2}Warning: Cannot create links in \"${BINPATH}/bin\" to binaries in \"${INSTPATH}/bin\"."
    RC=2
  fi
  return $RC
}
install_app(){
  ip=bin_${MEPLATPKG}_app
  echo "Installing: MicroEmacs_${MEVER}_$ip..."
  curl -fsSL -o Jasspa_MicroEmacs_${MEVER}_$ip.zip ${MEURL}/Jasspa_MicroEmacs_${MEVER}_$ip.zip
  if [ $? -ne 0 ]; then
    echo "Error: Failed to download install package \"${MEURL}/Jasspa_MicroEmacs_${MEVER}_$ip.zip\"."
    exit 1
  fi
  ${UNZIPCL} Jasspa_MicroEmacs_${MEVER}_$ip.zip ${UNZIPOP} ${INSTPATH}/../
  if [ $? -ne 0 ]; then
    echo "Error: Failed to extract install package \"/tmp/Jasspa_MicroEmacs_${MEVER}_$ip.zip\"."
    exit 1
  fi
  rm Jasspa_MicroEmacs_${MEVER}_$ip.zip
  INSTRPTH="/Contents/Resources"
  INSTBPTH="/Contents/MacOS"
  if [ ! -f "${INSTPATH}/meinfo$1" ] ; then
    echo ${MEVER} > "${INSTPATH}${INSTRPTH}/meinfo$1"
  fi
  if grep -q app "${INSTPATH}${INSTRPTH}/meinfo$1"; then
    :
  else
    echo app >> "${INSTPATH}${INSTRPTH}/meinfo$1"
  fi    
  
}
install_package(){
  case $2 in
  binaries|openssl)
     pid=bin_${MEPLATPKG}_$2
     ipth=${INSTPATH}${INSTBPTH};;
  *) pid=$2
     ipth=${INSTPATH}${INSTRPTH};;
  esac
  echo "Installing: MicroEmacs_${MEVER}_$pid..."
  curl -fsSL -o Jasspa_MicroEmacs_${MEVER}_$pid.zip ${MEURL}/Jasspa_MicroEmacs_${MEVER}_$pid.zip
  if [ $? -ne 0 ]; then
    echo "Error: Failed to download install package \"${MEURL}/Jasspa_MicroEmacs_${MEVER}_$pid.zip\"."
    exit 1
  fi
  mkdir -p "${ipth}"
  ${UNZIPCL} Jasspa_MicroEmacs_${MEVER}_$pid.zip ${UNZIPOP} ${ipth}
  if [ $? -ne 0 ]; then
    echo "Error: Failed to extract install package \"/tmp/Jasspa_MicroEmacs_${MEVER}_$pid.zip\"."
    exit 1
  fi
  rm Jasspa_MicroEmacs_${MEVER}_$pid.zip
  if [ $pid != $2 ]; then
    mv ${ipth}/bin/${MEPLATMSK}/* ${ipth}/
    rm -rf ${ipth}/bin
  fi
  if grep -q $2 ${INSTPATH}${INSTRPTH}/meinfo$1; then
    :
  else
    echo $2 >> ${INSTPATH}${INSTRPTH}/meinfo$1
  fi    
}

# First work out the platform and if this is an upgrade
PLATFORM=`uname`
case "$PLATFORM" in
Darwin)
  VERSION=`uname -r | cut -f 1 -d .`
  if [ $VERSION -gt 15 ] ; then
    if [ `uname -p` = "i386" ] ; then
      MEPLATPKG=macos_intel
      MEPLATMSK=macos*-intel64
    else
      MEPLATPKG=macos_apple
      MEPLATMSK=macos*-apple64
    fi
  fi;;
Linux)
  if [ `uname -m` = "x86_64" ] ; then
    if [ `uname -r | grep -Eo "^[0-9]"` = "5" ]; then
      MEPLATPKG=linux5_intel
      MEPLATMSK=linux*-intel64
    else
      MEPLATPKG=linux_intel
      MEPLATMSK=linux*-intel64
    fi
  else
    MEPLATPKG=linux_aarch
    MEPLATMSK=linux*-aarch64
  fi;;
CYGWIN_NT*)
  PLATFORM=CYGWIN
  MEPLATPKG=cygwin_intel
  MEPLATMSK=cygwin*-intel64
  ;;
MSYS_NT*)
  PLATFORM=MSYS
  MEPLATPKG=msyswin_intel
  MEPLATMSK=msyswin*-intel64
  ;;
esac
if [ -z "$MEPLATPKG" ] ; then
  echo "Error: Platform '${PLATFORM}' is not currently supported - please request suport."
  exit 1
fi
case $0 in
 *microemacs-update*) ISUPDT=1;;
 *) ISUPDT=0;;
esac

# Go through the command line arguments
INSTPKG=""
while [ -n "$1" ] ; do
  case "$1" in 
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
  *)  INSTPKG="$1"
      break;;
  esac
  shift
done

UNZIPCL=`which unzip 2>/dev/null`
if [ -z "$UNZIPCL" ] ; then
  UNZIPCL=`which bsdtar 2>/dev/null`
  if [ -z "$UNZIPCL" ] ; then
    echo "Error: Failed to locate unzip or bsdtar utility required for package extraction."
    exit 1
  fi
  UNZIPCL="bsdtar -xf"
  UNZIPOP="-C"
else    
  UNZIPCL="unzip -q -o"
  UNZIPOP="-d"
fi

# Get the latest release version number - no point continuing if can't access github
MEVER=`curl -s https://docs.jasspa.com/microemacs_release.txt | head -n 1`
if [ ${#MEVER} -ne 8 ] ; then
  echo "Error: Failed to obtain version number of latest release (${MELRL})."
  exit 1
fi
MEURL=$MEBASEURL/releases/download/me_${MEVER}
# Now work out where to install/upgrade
if [ -n "${INSTPATH}" ] ; then
  # Remove trailing '/' then check for standard install paths, e.g. .../microemacs, and if not present append /jasspa  
  INSTPATH="${INSTPATH%/}"
  case "${INSTPATH}" in
  */Applications)
    if [ $PLATFORM = "Darwin" ] ; then
      INSTTYPE="App "
      INSTPATH="${INSTPATH}/MicroEmacs.app"
    else
      INSTPATH="${INSTPATH}/jasspa"
    fi;;
  */Applications/MicroEmacs.app)
    if [ $PLATFORM = "Darwin" ] ; then
      INSTTYPE="App "
    else
      INSTPATH="${INSTPATH}/jasspa"
    fi;;
  */jasspa|*/Jasspa|*/MicroEmacs|*/microemacs|*/me) ;;
  *) INSTPATH="${INSTPATH}/jasspa";;
  esac
  install_path_check "${INSTPATH}" ""
  icerr=$?
  if [ $icerr -eq 1 ] ; then
    printf "${ERRMSG}\n\n" | fold -s
    exit 1
  fi
  if [ $ISUPDT -ne 0 ] ; then
    printf "Jasspa MicroEmacs - Updating \"${INSTPATH}\" to version ${MEVER}\n" | fold -s
  else
    printf "Jasspa MicroEmacs - Installing version ${MEVER} to \"${INSTPATH}\"\n" | fold -s
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
  inspth="$0"
  if [ -L "${inspth}" ]; then
    inspth=$(readlink "${inspth}")
  fi
  inspth=`cd -- "$(dirname "${inspth}")" >/dev/null 2>&1 ; pwd -P`
  
  # script should be in a bin directory, remove that then try ${INSTPATH} first and then ${INSTPATH}/share 
  case ${inspth} in
  */bin) inspth=${inspth%"/bin"};;
  */Contents/MacOS)
    if [ $PLATFORM = "Darwin" ] ; then
      inspth=${inspth%"/Contents/MacOS"}
      INSTRPTH="/Contents/Resources"
      INSTBPTH="/Contents/MacOS"
    fi
    ;;
  esac
  install_path_check "${inspth}" ""
  icerr=$?
  if [ $icerr -gt 1 ] ; then
    printf "${ERRMSG}\n\n"
    exit 1
  elif [ $icerr -ne 0 ] ; then
    install_path_check "${inspth}/share/jasspa" ""
    icerr=$?
    if [ $icerr -ne 0 ] ; then
      install_path_check "${inspth}/share/microemacs" ""
      icerr=$?
      if [ $icerr -ne 0 ] ; then
        printf "\nError: Failed to locate the Jasspa MicroEmacs installation directory, run with --prefix to set the location.\n\n" | fold -s
        exit 1
      fi
    fi
  fi
  if [ -z "${INSTPKG}" ] ; then
    printf "Jasspa MicroEmacs - Updating \"${INSTPATH}\" to version ${MEVER}\n" | fold -s
  fi
  
else

  # No path given, this is an install - on macOS we need to ask if app or standard
  if [ $PLATFORM = "Darwin" ] ; then
    printf "\nOn macOS Jasspa MicroEmacs supports a native app, or the standard mec & mew (terminal & X11 based) programmes. Which do you wish to install:\n\n" | fold -s
    while true; do
      read -p "Select (n) for native app, (s) for standard or (q) to quit? (n/s/q) " nsq
      case $nsq in 
      n) INSTTYPE="App "
         break;;
      s) break;;
      q) exit 1;;
      *) echo Invalid response...;;
      esac
    done
    echo
  fi
  
  if [ "${INSTTYPE}" = "App " ] ; then
    install_path_check "/Applications/MicroEmacs.app" "    "
    aicerr=$?
    aermsg=$ERRMSG
    ainpth=$INSTPATH
    abnpth=$BINPATH
    install_path_check "${HOME}/Applications/MicroEmacs.app" "    "
    uicerr=$?
    uermsg=$ERRMSG
    uinpth=$INSTPATH
    ubnpth=$BINPATH
  else
    # check /usr/local/share and ~/.local
    install_path_check "/usr/local/share/jasspa" "    "
    aicerr=$?
    aermsg=$ERRMSG
    ainpth=$INSTPATH
    abnpth=$BINPATH
    install_path_check "${HOME}/.local/share/jasspa" "    "
    uicerr=$?
    uermsg=$ERRMSG
    uinpth=$INSTPATH
    ubnpth=$BINPATH
  fi
  
  if [ $aicerr -eq 1 ] ; then
    printf "Cannot install to \"${ainpth}\" for all users:\n${aermsg}\n\n" | fold -s
    
    if [ $uicerr -eq 1 ] ; then
      printf "Cannot install to \"${uinpth}\" for current user:\n${uermsg}\n\n" | fold -s
      printf "Please resolve issues for one of the above or use --prefix option to set install location\n\n" | fold -s
      exit 1
    fi
    INSTPATH=${uinpth}
    BINPATH=${ubnpth}
    printf "Install Jasspa MicroEmacs version ${MEVER} to \"${INSTPATH}\":\n" | fold -s
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
    printf "Cannot install to \"${uinpth}\" for current user:\n${uermsg}\n\n" | fold -s
    INSTPATH=${ainpth}
    BINPATH=${abnpth}
    printf "Install Jasspa MicroEmacs version ${MEVER} to \"${INSTPATH}\":\n" | fold -s
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
      printf "Select (u) to install to \"${uinpth}\" for current user, however note:\n${uermsg}\n\n" | fold -s
    else
      printf "Select (u) to install to \"${uinpth}\" for current user.\n\n" | fold -s
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

if [ -z "${INSTPKG}" ] ; then

  if [ $ISUPDT -ne 0 ] ; then
  
    read -r MECRL < ${INSTPATH}${INSTRPTH}/meinfo
    if [ "${MECRL}" = "${MEVER}" ] ; then
      printf "\nNote: Installation is already version ${MEVER} - nothing to do.\n\n" | fold -s
      exit 0
    fi
  
    echo "Updating Jasspa MicroEmacs installation from v${MECRL} to v${MEVER}"
  
    echo ${MEVER} > ${INSTPATH}${INSTRPTH}/meinfo.upd
    skipped1=0
    while read ln; do
      if [ $skipped1 -eq 0 ] ; then
        skipped1=1    
      elif [ "${ln}" = "app" ] ; then
        install_app ".upd" $ln
      else
        install_package ".upd" $ln
      fi
    done <${INSTPATH}${INSTRPTH}/meinfo
  
    mv ${INSTPATH}${INSTRPTH}/meinfo.upd ${INSTPATH}${INSTRPTH}/meinfo  
    curl -fsSL -o microemacs-update $MEBASEURL/releases/latest/download/microemacs-install
    if [ $? -ne 0 ]; then
      printf "Error: Failed to download latest update script:\n    \"$MEBASEURL/releases/latest/download/microemacs-install\".\n\n" | fold -s
      exit 1
    fi
    chmod 755 microemacs-update
    mv microemacs-update ${INSTPATH}${INSTBPTH}/microemacs-update
    printf "Update to ${MEVER} complete.\n\n"
  
  else
  
    printf "\nInstallating Jasspa MicroEmacs ${INSTTYPE}v${MEVER} to \"${INSTPATH}\"\n" | fold -s
    mkdir -p "${INSTPATH}"
    if [ ! -d ${INSTPATH} ] ; then
      printf "\nError: Failed to create install path \"${INSTPATH}\", either create directory \"${INSTPATH}\" or rerun with sudo.\n\n" | fold -s
      return 1
    fi
  
    #install the core
    if [ "${INSTTYPE}" = "App " ] ; then
      install_app ""
    else
      echo ${MEVER} > ${INSTPATH}${INSTRPTH}/meinfo
      install_package "" binaries
    fi
    install_package "" macros
    install_package "" help_ehf
    curl -fsSL -o microemacs-update $MEBASEURL/releases/latest/download/microemacs-install
    if [ $? -ne 0 ]; then
      echo "Error: Failed to download latest update script \"$MEBASEURL/releases/latest/download/microemacs-install\"."
      exit 1
    fi
    chmod 755 microemacs-update
    mv microemacs-update ${INSTPATH}${INSTBPTH}/microemacs-update
  
    if [ "${INSTTYPE}" = "App " ] ; then
        echo "Installation complete."
        BINPATH="${INSTPATH}${INSTBPTH}/"
    else
      
      if [ -z "${BINPATH}" ] ; then
        BINPATH=${INSTPATH}
      else
        if [ ! -e ${BINPATH}/bin ] ; then
          mkdir -p "${BINPATH}/bin" >/dev/null 2>&1
        fi
        if [ ! -d ${BINPATH}/bin -o ! -w ${BINPATH}/bin ] ; then
          printf "\nWarning: Cannot create links in \"${BINPATH}\" to binaries in \"${INSTPATH}/bin\", either add ${INSTPATH}/bin to your PATH, or copy the scripts to somewhere in your PATH, or rerun with sudo.\n\n" | fold -s
          BINPATH=${INSTPATH}
        else
          rm -f ${BINPATH}/bin/mec
          rm -f ${BINPATH}/bin/mew
          rm -f ${BINPATH}/bin/tfs
          rm -f ${BINPATH}/bin/microemacs-update
          if [ $PLATFORM = "MSYS" ] ; then
            printf "#!/bin/sh\nPTH=%s\n%s/../share/jasspa/bin/mec %s\n" '$(dirname "$0")' '${PTH}' '$*' > ${BINPATH}/bin/mec
            printf "#!/bin/sh\nPTH=%s\n%s/../share/jasspa/bin/tfs %s\n" '$(dirname "$0")' '${PTH}' '$*' > ${BINPATH}/bin/tfs
            printf "#!/bin/sh\nPTH=%s\n%s/../share/jasspa/bin/microemacs-update %s\n" '$(dirname "$0")' '${PTH}' '$*' > ${BINPATH}/bin/microemacs-update
          else
            ln -s ${INSTPATH}/bin/mec ${BINPATH}/bin/mec
            ln -s ${INSTPATH}/bin/mew ${BINPATH}/bin/mew
            ln -s ${INSTPATH}/bin/tfs ${BINPATH}/bin/tfs
            ln -s ${INSTPATH}/bin/microemacs-update ${BINPATH}/bin/microemacs-update
          fi
        fi
      fi
      
      if [ $PLATFORM == "Linux" ] ; then
        while true; do
          read -p "Create Application launcher for mew ? (y/n) " rin
          case $rin in 
          y) ${INSTPATH}/bin/mec -p @crtappln -f ${INSTPATH}/bin/mew
             break;;
          n) break;;
          *) echo Invalid response...;;
          esac
        done
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
    if [ "${INSTTYPE}" != "App " ] ; then
      echo "To start using Jasspa MicroEmacs run:"
      echo ""
      echo "   ${BINPATH}mec"
      echo ""
      if [ $PLATFORM = "MSYS" ] ; then
        echo "in an MSYS2 console/terminal."
      else
        echo "in a console/terminal, or run:"
        echo ""
        echo "   ${BINPATH}mew"
        echo ""
        echo "for the GUI version, requires a working X (install XQuartz on macOS)."
      fi
      echo ""
    fi
  fi
       
elif [ "${INSTPKG}" = "openssl" ] ; then

  echo "Jasspa MicroEmacs - Installing OpenSSL libraries"
  install_package "" openssl
  echo "Package installation complete."

elif [ ${#INSTPKG} -eq 4 ] ; then

  echo "Jasspa MicroEmacs - Installing spelling ${INSTPKG}"
  install_package "" spelling_${INSTPKG}
  echo "Package installation complete."

else

  pkg=`echo "${INSTPKG}" | sed "s/-/_/g"`

  case $pkg in
    binaries | help_ehf | macros | spelling_*)
      echo "Jasspa MicroEmacs - Installing package $pkg"
      install_package "" $pkg
      echo "Package installation complete.";;
    *)
      echo "Error: Unknown install package \"${INSTPKG}\"."
      exit 1;;
  esac

fi
