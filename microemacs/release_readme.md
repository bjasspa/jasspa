## INTRODUCTION

MicroEmacs is an Emacs like text editor for developers, its great strength is its speed, fast key-bindings and powerful macro language. If you are looking for a menu driven editor this is probably not for you!

Jasspa MicroEmacs also has a couple of other base limitations, firstly, no real Unicode support - so you you are constantly editing multi-language files this is probably not the best tool for you. MicroEmacs also does not support long line wrapping in its rendering, one line of a file will always be represented as one line on the screen regardless of how long the line is - this is *Micro*Emacs, if these issues are blockers to you, try Emacs.

For move information visit our [Github repository](https://github.com/bjasspa/jasspa/).

## VERY QUICK START GUIDE

For your platform download the single-file binary zip file:

| Platform      | Console/Terminal | GUI (X11 on Linux/Mac) |
|:-------------:|:----------------:|:----------------------:|
| Linux intel   | [mesc](https://github.com/bjasspa/jasspa/releases/download/me_<VERSION>/Jasspa_MicroEmacs_<VERSION>_abin_linux_intel_mesc.zip)   | [mesw](https://github.com/bjasspa/jasspa/releases/download/me_<VERSION>/Jasspa_MicroEmacs_<VERSION>_abin_linux_intel_mesw.zip)   |
| Linux aarch   | [mesc](https://github.com/bjasspa/jasspa/releases/download/me_<VERSION>/Jasspa_MicroEmacs_<VERSION>_abin_linux_aarch_mesc.zip)   | [mesw](https://github.com/bjasspa/jasspa/releases/download/me_<VERSION>/Jasspa_MicroEmacs_<VERSION>_abin_linux_aarch_mesw.zip)   |
| MacOS apple   | [mesc](https://github.com/bjasspa/jasspa/releases/download/me_<VERSION>/Jasspa_MicroEmacs_<VERSION>_abin_macos_apple_mesc.zip)   | [mesw](https://github.com/bjasspa/jasspa/releases/download/me_<VERSION>/Jasspa_MicroEmacs_<VERSION>_abin_macos_apple_mesw.zip)   |
| MacOS intel   | [mesc](https://github.com/bjasspa/jasspa/releases/download/me_<VERSION>/Jasspa_MicroEmacs_<VERSION>_abin_macos_intel_mesc.zip)   | [mesw](https://github.com/bjasspa/jasspa/releases/download/me_<VERSION>/Jasspa_MicroEmacs_<VERSION>_abin_macos_intel_mesw.zip)   |
| Windows intel | [mesc](https://github.com/bjasspa/jasspa/releases/download/me_<VERSION>/Jasspa_MicroEmacs_<VERSION>_abin_windows_intel_mesc.zip) | [mesw](https://github.com/bjasspa/jasspa/releases/download/me_<VERSION>/Jasspa_MicroEmacs_<VERSION>_abin_windows_intel_mesw.zip) |
| Windows arm   | [mesc](https://github.com/bjasspa/jasspa/releases/download/me_<VERSION>/Jasspa_MicroEmacs_<VERSION>_abin_windows_arm_mesc.zip)   | [mesw](https://github.com/bjasspa/jasspa/releases/download/me_<VERSION>/Jasspa_MicroEmacs_<VERSION>_abin_windows_arm_mesw.zip)   |

Extract the executable from the zip and run.

The console/terminal version should work on all computers of the given platform, whereas the GUI version may not work on all computers (particularly macOS where XQuartz is required) but will give the best user experience.

This is a truly portable version, it will not alter/add to your environment or registry and will only create/edit files you explicitly ask it to. So this version is ideal for trying MicroEmacs without affecting your computer.

For a more complete experience execute command **init-session**, this creates a small user configuration directory in your user area, allowing MicroEmacs to save setup changes and download supporting files such as the comprehensive help docs and spelling dictionaries for numerous languages.


## INSTALLERS

- **Windows:** Download and run [Jasspa_MicroEmacs_<VERSION>_installer_windows.msi](https://github.com/bjasspa/jasspa/releases/download/me_<VERSION>/Jasspa_MicroEmacs_<VERSION>_installer_windows_intel.msi) installer (intel only), which contains the binaries, macros and help file, to create a fully working environment.

- **UNIX:** The preferred install method is using the [microemacs-install](https://github.com/bjasspa/jasspa/releases/download/me_<VERSION>/microemacs-install) script, run the following script in a terminal:

      /bin/sh -c "$(curl -fsSL https://github.com/bjasspa/jasspa/releases/latest/download/microemacs-install)"
    
  Or download the script first and run locally, note that this will always install the latest release. If the script encounters issues during the installation processes, typically insufficient permissions, please follow the instructions given.
  
  Once successfully installed a `microemacs-update` script can be used to update the installation to the latest version or to install spelling languages, e.g. run:
  
      microemacs-update enus
   
- **UNIX - Homebrew:** For users familiar with brew we also provide a brew installer. In a terminal run:

      brew tap bjasspa/jasspa
      brew install microemacs
    
  This will install the binaries, macros and help. To install spelling languages run:
  
      brew install microemacs-spelling-<LANG>
    
  Use `brew search microemacs` for a full list of available packages.

On all platforms, spelling dictionaries can also be downloaded and installed by MicroEmacs as and when required.
 

## SLOWER QUICK START GUIDE

To build a more complete, native environment do the following:

1. Create a main MicroEmacs directory to hold all required files, this is typically ~/MicroEmacs on Unix or c:\\Users\\\<user-name>\\MicroEmacs on Windows, but the path can be anywhere. In following steps this path will be referred to as ~/MicroEmacs

2. Download the following zip package files:

    a. **Required:** Binaries for your platform (Jasspa_MicroEmacs_<VERSION>_bin_\<PLATFORM>\_\<ARCHITECTURE>\_binaries.zip)
     
    b. **Required:** Macro files ([Jasspa_MicroEmacs_<VERSION>_macros.zip](https://github.com/bjasspa/jasspa/releases/download/me_<VERSION>/Jasspa_MicroEmacs_<VERSION>_macros.zip))

    c. Help file ([Jasspa_MicroEmacs_<VERSION>_help_ehf.zip](https://github.com/bjasspa/jasspa/releases/download/me_<VERSION>/Jasspa_MicroEmacs_<VERSION>_help_ehf.zip))

    d. Spelling dictionaries for any language you require (Jasspa_MicroEmacs_<VERSION>_spelling_\<LANGUAGE>.zip)
     
    e. OpenSSL dynamic libraries (Jasspa_MicroEmacs_<VERSION>_bin_\<PLATFORM>\_\<ARCHITECTURE>\_openssl.zip) for https/ftps support (country permitting) 

3. Extract downloaded zip files into the ~/MicroEmacs directory you created in (1), this should have created:

    `~/MicroEmacs/bin` - containing platform directories for each of the binary packages you downloaded and extracted.

    `~/MicroEmacs/macros` - containing many macro files (*.emf), the me.ehf help file and other supporting files.

    `~/MicroEmacs/spelling` - containing the dictionaries of the languages you downloaded.

4. Set the following environment variable:

     `MEINSTALLPATH=~/MicroEmacs`

   Setting an environment variable is platform & shell dependent.

5. Run:

     `~/MicroEmacs/bin/<platform>/mec`

   The console version should run on all platforms, if the message '[Failed to load file me]' appears at the bottom of the window then you have either not set the environment variable correctly or it has not taken effect in your current shell/command prompt.

6. Try running:

     `~/MicroEmacs/bin/<platform>/mew`

   This should always work on Windows but may not on Unix systems, particularly macOS, where an X server (XQuartz) is required to support the window interface. Fixing issues causing mew to not run is beyond the scope of this document.

7. Optionally copy the binaries (mec & mew) into a directory on your path (or add ~/MicroEmacs/bin/\<platform> to your path. This simplifies running MicroEmacs in a terminal.

8. Run mew, if working, mec otherwise and follow the setup steps to create a basic user configuration area, this will create the directory:

     `~/MicroEmacs/<user-name>`

   All your customisations should go in here. Run the command user-setup (in the Tools menu) to start customising the editor.

9. Enjoy fast, free editing!
