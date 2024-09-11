#!/usr/bin/env bash


function tpl2pkg {
   if [ -z $2 ]; then
      printf "tpl2pkg VERSION TEMPLATE\n"
   else
      version=$(echo $1 | sed -E 's/.+MicroEmacs_([0-9]+).+/\1/') #'
      outfile=$(echo $2 | sed -E 's/.tpl$//')
      printf "$version $outfile\n"
      dld="https://github.com/bjasspa/jasspa/releases/download/me_${version}"
      if [ ! -f "Jasspa_MicroEmacs_${version}_bin_windows_binaries.zip" ]; then
         wget "${dld}/Jasspa_MicroEmacs_${version}_bin_windows_binaries.zip"
      fi
      if [ ! -f "Jasspa_MicroEmacs_${version}_macros.zip" ]; then
         wget "${dld}/Jasspa_MicroEmacs_${version}_macros.zip"
      fi
      if [ ! -f "Jasspa_MicroEmacs_${version}_help_ehf.zip" ]; then
         wget "${dld}/Jasspa_MicroEmacs_${version}_help_ehf.zip"
      fi
      if [ -d "microemacs-package" ]; then 
          rm -rf microemacs-package
      fi
      mkdir microemacs-package
      unzip -o Jasspa_MicroEmacs_${version}_bin_windows_binaries.zip -d microemacs-package
      unzip -o Jasspa_MicroEmacs_${version}_macros.zip -d microemacs-package
      unzip -o Jasspa_MicroEmacs_${version}_help_ehf.zip -d microemacs-package
      mv microemacs-package/bin/*/me?.exe microemacs-package/
      dig=$(sha256sum microemacs-package/mec.exe | cut -c 1-64)
      sed -E "s/digest-mec.exe/${dig}/" $2 > microemacs-package/PKGBUILD
      dig=$(sha256sum microemacs-package/mew.exe | cut -c 1-64)      
      sed -i -E "s/digest-mew.exe/${dig}/" microemacs-package/PKGBUILD
      dig=$(sha256sum microemacs-package/readme.txt | cut -c 1-64)      
      sed -i -E "s/digest-readme.txt/${dig}/" microemacs-package/PKGBUILD
      dig=$(sha256sum microemacs-package/COPYING.txt | cut -c 1-64)      
      sed -i -E "s/digest-COPYING.txt/${dig}/" microemacs-package/PKGBUILD
   fi  
}

tpl2pkg $1 $2
