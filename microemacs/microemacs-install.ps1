#Requires -Version 5.1

param(
  [string]$package,
  [string]$notused1,
  [string]$notused2,
  [string]$prefix,
  [switch]$help,
  [switch]$h
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

if($help -or $h) {
  $sn = $MyInvocation.MyCommand.Name
  Write-Host "Usage: $sn [-h | --help] [ --prefix=<path> ] [ <package> ]"
  Write-Host "Where:"
  Write-Host "  <path>     Is the install location, defaulting to %ProgramFiles%"
  Write-Host "             or %APPDATA% depending on permissions."
  Write-Host "  <package>  The package to install, e.g. engb, enus, dede etc for spelling"
  Write-Host "             languages and openssl for https support."
  Write-Host ""
  exit 1
}

$mebaseurl = "https://github.com/bjasspa/jasspa"
$instpath = ""
$binpath = ""
   
$arch = $env:PROCESSOR_ARCHITECTURE
if ($arch -eq 'ARM64') {
  $meplatful = 'windows100-arm64'
  $meplatpkg = 'windows_arm'
  $mevcruntm = 'arm64'
} elseif ($arch -eq 'AMD64') {
  $meplatful = 'windows100-intel32'
  $meplatpkg = 'windows_intel'
  $mevcruntm = 'x86'
} else {
  Write-Host "Error: Architecture '$arch' is not currently supported - please request suport."
  exit 1
}
$isupdt = $MyInvocation.MyCommand.Name.Contains("microemacs-update") 

function install_path_check([string]$bpath,[string]$pInd) {
  $errmsg=""
  $rc=0
  if($isupdt) {
    if(-not (Test-Path "${bpath}\meinfo")) {
      $errmsg = "${pInd}Error: Installation found at `"$bpath`" was not created by the microemacs-install script, please re-install first."
      $rc = 1
    }
    else {
      Try {
        [io.file]::OpenWrite("${bpath}\meinfo").close()
      }
      Catch {
        $errmsg = "${pInd}Error: Cannot write to install path `"$bpath`", either change ownership/permissions or rerun with sudo."
        $rc = 1
      }
    }
  }
  elseif(Test-Path "$bpath") {
    if(-not (Test-Path -Path "$bpath")) {
      $errmsg = "${pInd}Error: Install path `"$bpath`" already exists but isn't a directory."
      $rc = 1
    }
    else {
      Try { 
        [io.file]::OpenWrite("$bpath\jasspa.tmp").close()
        Remove-Item -Path "$bpath\jasspa.tmp"
      }
      Catch {
        $errmsg = "${pInd}Error: Cannot write to install path `"$bpath`", either change ownership/permissions or rerun as Administrator."
        $rc = 1
      }
      if($rc -eq 0) {
        if(Test-Path "$bpath\meinfo") {
          $errmsg = "${pInd}Warning: MicroEmacs already installed to `"$bpath`", consider using microemacs-update instead."
          $rc = 2
        }
        elseif((Test-Path "$bpath\macros") -or (Test-Path "$bpath\bin")) {
          $errmsg = "${pInd}Warning: Install path `"instpath`" was not created by this installer, continuing will overwrite existing installation."
          $rc = 2
        }
      }
    }
  }
  else {
    $ppath = Split-Path -parent $bpath
    if(Test-Path "$ppath") {
      if(-not (Test-Path -Path "$ppath")) {
        $errmsg = "${pInd}Error: Install path `"$ppath`" already exists but isn't a directory."
        $rc = 1
      }
      else {
        Try {
          [io.file]::OpenWrite("$ppath\jasspa.tmp").close()
          Remove-Item -Path "$ppath\jasspa.tmp"
        }
        Catch {
          $errmsg = "${pInd}Error: Cannot write to path `"$ppath`", either create directory `"$bpath`" or rerun as Administrator."
          $rc = 1
        }
      }
    }
    else {
      #TODO should also pass in upgrade as no dir should be created here
      Try {
        New-Item -Path $ppath -ItemType Directory -Force > $null
        if(Test-Path -Path $ppath) {
          Remove-Item $ppath -Force
        }
        else {
          $errmsg = "${pInd}Error: Cannot create install base path `"$ppath`", either create directory `"$bpath`" or rerun as Administrator."
          $rc = 1
        }
      }
      Catch {
        $errmsg = "${pInd}Error: Cannot create install base path `"$ppath`", either create directory `"$bpath`" or rerun as Administrator."
        $rc = 1
      }
    }
  }
  return @{
    rc = $rc
    errmsg = $errmsg
    instpath = $bpath
  }
}
function install_package([string]$iext,[string]$pkg,[string]$ipth) {
  if($pkg -eq "binaries") {
    $ip = "bin_${meplatpkg}_$pkg"
  }
  elseif($pkg -eq "openssl") {
    $ip = "bin_${meplatpkg}_$pkg"
  }
  else {
    $ip=$pkg
  }
  Write-Host "Installing: MicroEmacs_${mever}_${ip}..."
  $pfn = "Jasspa_MicroEmacs_${mever}_${ip}.zip"
  $tfn = Join-Path $env:TEMP $pfn
  try {
    Invoke-WebRequest -Uri "${meurl}/${pfn}" -OutFile $tfn
  }
  catch {
    Write-Host "Error: Failed to download install package `"${meurl}/${pfn}`":`n    $($_.Exception.Message)"
    exit 1
  }
  try {
    Expand-Archive -Path $tfn -DestinationPath $ipth -Force
  }
  catch {
    Write-Host "Error: Failed to extract install package `"$tfn`":`n    $($_.Exception.Message)"
    exit 1
  }
  Remove-Item $tfn -Force
  
  if($ip -ne $pkg) {
    Move-Item -Path "${ipth}\bin\${meplatful}\*" -Destination "${ipth}\bin" -Force
    Remove-Item "${ipth}\bin\${meplatful}" -Force
  }
  if(-not (Select-String -Path "$ipth\meinfo${iext}" -Pattern "$pkg" -Quiet)) {
    Add-Content -Path "$ipth\meinfo${iext}" -Value "$pkg"
  }
}

try {
  $resp = Invoke-WebRequest -Method Get -Uri "$mebaseurl/releases/latest/" -UseBasicParsing -MaximumRedirection 0 -ErrorAction SilentlyContinue
  $melrl = $resp.Headers['Location']
  if(-not $melrl) {
    Write-Host "Error: Failed to obtain version number of latest release (no location header)."
    exit 1
  }
  if($melrl -match 'me_(\d{8})$') {
    $mever = $Matches[1]
  }
  else {
    Write-Host "Error: Failed to obtain version number of latest release ($melrl)."
    exit 1
  }
} catch {
  Write-Host "Error: Failed to obtain version number of latest release:`n    $($_.Exception.Message)"
  exit 1
}  
$meurl = "$mebaseurl/releases/download/me_$mever"

# Now work out where to install/upgrade
$scopeMch = $false
if($prefix -ne "") {
  # Install path given by user - remove trailing '/jasspa/' '/' & check permissions etc
  $instpath = $prefix.Replace("/","\")
  if($instpath -match "\\$") {
    $instpath = $instpath.Substring(0,$instpath.Length - 1)
  }
  if(($instpath -match "\\jasspa$") -or ($instpath -match "\\microemacs$") -or ($instpath -match "\\jasspa microemacs$") -or ($instpath -match "\\me$")) {
  }
  else {
      $instpath = "$(instpath)\jasspa"
  }
  $rr = install_path_check $instpath ""
  if($rr.rc -eq 1) {
    Write-Host $rr.errmsg
    exit 1
  }
  $instpath = $rr.instpath
  if($isupdt) {
    Write-Host "Jasspa MicroEmacs - Updating `"${instpath}`" to version ${mever}"
  }
  else {
    Write-Host "Jasspa MicroEmacs - Installing version ${mever} to `"${instpath}`""
    if($rr.rc -ne 0) {
      Write-Host $rr.errmsg
    }
    $ii = $true
    while ($ii) {
      $rr = Read-Host "`nDo you want to continue? (y/n) "
      switch ($rr.ToLower()) {
        'y' { $ii = $false }
        'n' { exit 1 }
        default { Write-Host "Please answer y or n." }
      }
    }
    if(-not ($instpath.StartsWith($env:USERPROFILE))) {
      $scopeMch = $true
    }
  }
}
elseif($isupdt) {
  # No path given, this is an update so get the path from the location of the script
  $instpath = $PSScriptRoot
  # script should be in a \jasspa\bin directory, remove \bin first
  if($instpath -match "\\$") {
    $instpath = $instpath.Substring(0,$instpath.Length - 1)
  }
  if($instpath -match "\\bin$") {
    $instpath = $instpath.Substring(0,$instpath.Length - 4)
  }
  $rr = install_path_check $instpath ""
  if($rr.rc -eq 0) {
    $instpath = $rr.instpath
  }
  else {
    # script not run from within the release, check the 2 default places before giving up
    $rr = ${env:ProgramFiles(x86)}
    $rr = install_path_check "${rr}\Jasspa MicroEmacs" ""
    if($rr.rc -eq 0) {
      $instpath = $rr.instpath
    }
    else {
      $rr = install_path_check "${rr}\jasspa" ""
      if($rr.rc -eq 0) {
        $instpath = $rr.instpath
      }
      else {
        $rr = $env:APPDATA
        $rr = install_path_check "${rr}\jasspa" ""
        if($rr.rc -eq 0) {
          $instpath = $rr.instpath
        }
        else {
          Write-Host "`nError: Failed to locate the Jasspa MicroEmacs installation directory, run with --prefix to set the location.`n"
          exit 1
        }
      }
    }
  }
  Write-Host "Jasspa MicroEmacs - Updating `"${instpath}`" to version ${mever}"
}
else {
  # No path given, this is an install, check ProgramFiles for all and AppData for user
  if($meplatful -match "32$") {
    $rr = ${env:ProgramFiles(x86)}
  }
  else {
    $rr = ${env:ProgramFiles}
  }
  $rr = install_path_check "${rr}\Jasspa MicroEmacs" "    "
  $aicerr = $rr.rc
  $aermsg = $rr.errmsg
  $ainpth = $rr.instpath
  $rr = $env:APPDATA
  $rr = install_path_check "${rr}\jasspa" "    "
  $uicerr = $rr.rc
  $uermsg = $rr.errmsg
  $uinpth = $rr.instpath
  
  if($aicerr -eq 1) {
    Write-Host "Cannot install to `"${ainpth}`" for all users:`n`n${aermsg}`n"
    
    if($uicerr -eq 1) {
      Write-Host "Cannot install to `"${uinpth}`" for current user:`n`n${uermsg}`n"
      Write-Host "Please resolve issues for one of the above or use --prefix option to set install location`n"
      exit 1
    }
    
    $instpath=${uinpth}
    Write-Host "Install Jasspa MicroEmacs version ${mever} to `"${instpath}`":"
    if($uicerr -ne 0) {
      Write-Host "`n${uermsg}`n"
    }
    $ii = $true
    while ($ii) {
      $rr = Read-Host "`nDo you want to continue? (y/n) "
      switch ($rr.ToLower()) {
        'y' { $ii = $false }
        'n' { exit 1 }
        default { Write-Host "Please answer y or n." }
      }
    }
  }
  elseif($uicerr -eq 1) {
    Write-Host "Cannot install to `"${uinpth}`" for current user:`n`n${uermsg}`n"
    $instpath = ${ainpth}
    $scopeMch = $true
    Write-Host "Install Jasspa MicroEmacs version ${MEVER} to `"${instpath}`":"
    if($aicerr -ne 0) {
      Write-Host "`n${aermsg}"
    }
    $ii = $true
    while ($ii) {
      $rr = Read-Host "Do you want to continue? (y/n) "
      switch ($rr.ToLower()) {
        'y' { $ii = $false }
        'n' { exit 1 }
        default { Write-Host "Please answer y or n." }
      }
    }
  }
  else {
    Write-Host "Jasspa MicroEmacs v${mever} can be installed for all users or for just the current user:`n"
    if($aicerr -ne 0) {
      Write-Host "Select (a) to install to `"${ainpth}`" for all users, however note:`n`n${aermsg}`n"
    }
    else {
      Write-Host "Select (a) to install to `"${ainpth}`" for all users.`n"
    }
    if($uicerr -ne 0) {
      Write-Host "Select (u) to install to `"${uinpth}`" for current user, however note:`n`n${uermsg}`n"
    }
    else {
      Write-Host "Select (u) to install to `"${uinpth}`" for current user.`n"
    }
    $ii = $true
    while ($ii) {
      $rr = Read-Host "`nHow do you want to continue or (q) to quit? (a/u/q) "
      switch ($rr.ToLower()) {
        'a' { 
          $instpath = ${ainpth}
          $scopeMch = $true
          $ii = $false
        }
        'u' { 
          $instpath = ${uinpth}
          $ii = $false
        }
        'q' { exit 1 }
        default { Write-Host "Please answer a, u or q." }
      }
    }
  }
} 


if($package -eq "") {
  if($isupdt) {
    $onVer=$true
    foreach($ln in Get-Content "${instpath}\meinfo") {
      if($onVer) {
        if($ln -eq $mever) {
          Write-Host "`nNote: Installation is already version ${mever} - nothing to do.`n"
          exit 0
        }
        Write-Host "Updating Jasspa MicroEmacs installation from v${ln} to v${mever}"
        Set-Content -Path "${instpath}\meinfo.upd" -Value "${mever}"
        $onVer=$false
      }
      else {
        install_package ".upd" $ln "${instpath}"
      }
    }
    Move-Item -Path "${instpath}\meinfo.upd" -Destination "${instpath}\meinfo" -Force
    try {
      Invoke-WebRequest -Uri "$mebaseurl/releases/latest/download/microemacs-install.ps1" -OutFile "${instpath}\bin\microemacs-update.ps1"
    }
    catch {
      Write-Host "Error: Failed to download latest update script `"$mebaseurl/releases/latest/download/microemacs-install.ps1`":`n    $($_.Exception.Message)"
      exit 1
    }
    Write-Host "Update to ${mever} complete and successful.`n"
  }
  else {
    Write-Host "`nInstallating Jasspa MicroEmacs v${mever} to `"${instpath}`""
    if(-not (Test-Path -Path "${instpath}")) {
      New-Item -Path "${instpath}" -ItemType Directory -Force > $null
      if(-not (Test-Path -Path "${instpath}")) {
        Write-Host "`nError: Failed to create install path `"${instpath}`", either create directory `"${instpath}`" or rerun as Administrator.`n"
        exit 1
      }
    }
    Set-Content -Path "${instpath}\meinfo" -Value "${mever}"
    
    #install the core
    install_package "" "binaries" "${instpath}"
    install_package "" "macros" "${instpath}"
    install_package "" "help_ehf" "${instpath}"
    try {
      Invoke-WebRequest -Uri "$mebaseurl/releases/latest/download/microemacs-install.ps1" -OutFile "${instpath}\bin\microemacs-update.ps1"
    }
    catch {
      Write-Host "Error: Failed to download latest update script `"$mebaseurl/releases/latest/download/microemacs-install.ps1`":`n    $($_.Exception.Message)"
      exit 1
    }
    $wScrO = New-Object -ComObject("WScript.Shell")
    $ii = 0
    while ($ii -eq 0) {
      $rr = Read-Host "Create a shortcut for MicroEmacs on the desktop? (y/n) "
      switch ($rr.ToLower()) {
        'y' { $ii = 2 }
        'n' { $ii = 1 }
        default { Write-Host "Please answer y or n." }
      }
    }
    if($ii -eq 2) {
      $ii = [System.IO.Path]::Combine([Environment]::GetFolderPath("Desktop"),"Jasspa MicroEmacs.lnk")
      $sc = $wScrO.CreateShortcut($ii)
      $sc.TargetPath = "${instpath}\bin\mew.exe"
      $sc.WorkingDirectory = "${instpath}"
      $sc.IconLocation = "${instpath}\bin\mew.exe"
      $sc.Save()
    }
    $ii = 0
    while ($ii -eq 0) {
      $rr = Read-Host "Create a shortcut for MicroEmacs in the Start menu? (y/n) "
      switch ($rr.ToLower()) {
        'y' { $ii = 2 }
        'n' { $ii = 1 }
        default { Write-Host "Please answer y or n." }
      }
    }
    if($ii -eq 2) {
      if($scopeMch) {
        $ii = "$env:ProgramData\Microsoft\Windows\Start Menu\Programs\Jasspa MicroEmacs.lnk"
      }
      else {
        $ii = "$env:APPDATA\Microsoft\Windows\Start Menu\Programs\Jasspa MicroEmacs.lnk"
      }
      $sc = $wScrO.CreateShortcut($ii)
      $sc.TargetPath = "${instpath}\bin\mew.exe"
      $sc.WorkingDirectory = "${instpath}"
      $sc.IconLocation = "${instpath}\bin\mew.exe"
      $sc.Save()
    }
    <# For some reason the New-ItemProperty in the following simply hangs with no info - shame!
    if($scopeMch) {
      $ii = 0
      while ($ii -eq 0) {
        $rr = Read-Host "Create an Edit with MicroEmacs menu item in Explorer? (y/n) "
        switch ($rr.ToLower()) {
          'y' { $ii = 2 }
          'n' { $ii = 1 }
          default { Write-Host "Please answer y or n." }
        }
      }
      if($ii -eq 2) {
        try{
          New-Item -Path "Registry::HKEY_CLASSES_ROOT\*\shell\Edit with MicroEmacs" -Force > $null
          New-ItemProperty -Path "Registry::HKEY_CLASSES_ROOT\*\shell\Edit with MicroEmacs" -Name "command" -Value "`"${instpath}\bin\mew.exe`" -c -o `"%1`"" -PropertyType String -Force
        }
        catch {
          Write-Host "Warning: Failed to create Explorer menu item:`n    $($_.Exception.Message)"
        }
      }
    }
    #>
    $pl = $env:Path -split ";"
    if($pl -contains "${instpath}\bin") {
      Write-Host "Installation complete and programmes are in your PATH."
      $binpath = ""
    }
    else {
      Write-Host "Installation complete, but the programmes are not in your PATH."
      $ii = 0
      while ($ii -eq 0) {
        $rr = Read-Host "Add MicroEmacs to your PATH? (y/n) "
        switch ($rr.ToLower()) {
          'y' { $ii = 2 }
          'n' { $ii = 1 }
          default { Write-Host "Please answer y or n." }
        }
      }
      if($ii -eq 2) {
        if($scopeMch) {
          try {
            [Environment]::SetEnvironmentVariable("Path",$env:Path + ";${instpath}\bin",[EnvironmentVariableTarget]::Machine)
          }
          catch {
            $scopeMch = $false
          }
        }
        if(-not $scopeMch) {
          [Environment]::SetEnvironmentVariable("Path",$env:Path + ";${instpath}\bin",[EnvironmentVariableTarget]::User)
        }
        $env:Path += ";${instpath}\bin"
        $binpath = ""
      }
      else {
        $binpath = "${instpath}\bin\"
      }
    }
    Write-Host "To install spelling support for a language run:"
    Write-Host ""
    Write-Host "   ${binpath}microemacs-update <lang-id>"
    Write-Host ""
    Write-Host "where <lang-id> is the 4 character language code, such as `"enus`" or `"dede`" etc."
    Write-Host "To add https support OpenSSL libraries are required, if not already available run:"
    Write-Host ""
    Write-Host "   ${binpath}microemacs-update openssl"
    Write-Host ""
    Write-Host "To start using Jasspa MicroEmacs run:"
    Write-Host ""
    Write-Host "   ${binpath}mec"
    Write-Host ""
    Write-Host "in a console/terminal, or run:"
    Write-Host ""
    Write-Host "   ${binpath}mew"
    Write-Host ""
    $vcv = (Get-ItemProperty -Path "HKLM:\SOFTWARE\Classes\Installer\Dependencies\VC,redist.${mevcruntm},*" -Name Version).Version
    if($vcv -eq $null) {
        Write-Host "NOTE: You will need to install Microsoft Visual C++ runtime libraries for ${mevcruntm} before you can run Microemacs, see:`n`n    https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170`n`n"
    }
    elseif([int] $vcv.Substring(0,3) -lt 14) {
        Write-Host "NOTE: You may need to install the latest version of Microsoft Visual C++ runtime libraries for ${mevcruntm} before you can run Microemacs, see:`n`n    https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170`n`n"
    }
  }
}       
elseif($package -eq "openssl") {
  Write-Host "Jasspa MicroEmacs - Installing OpenSSL libraries"
  install_package "" "openssl" "${instpath}"
}
elseif($package.Length -eq 4) {
  Write-Host "Jasspa MicroEmacs - Installing spelling $package"
  install_package "" "spelling_$package" "${instpath}"
}
else {
  $pkg = $package.Replace("-","_")
  if(($pkg -eq "binaries") -or ($pkg -eq "help_ehf") -or ($pkg -eq "macros") -or ($pkg -match "spelling_.*")) {
    Write-Host "Jasspa MicroEmacs - Installing package $pkg"
    install_package "" $pkg "${instpath}"
  }
  else {
    Write-Host "Error: Unknown install package `"$package`"."
    exit 1
  }
}
