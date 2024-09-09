INTRODUCTION
------------

MicroEmacs is an Emacs like text editor for developers, its great strength is its speed, fast key-bindings and powerful
macro language. If you are looking for a menu driven editor this is probably not for you!

Jasspa MicroEmacs also has a couple of other base limitations, firstly, no real Unicode support - so you you are constantly
editing multi-language files this is probably not the best tool for you. MicroEmacs also does not support long line
wrapping in its rendering, one line of a file will always be represented as one line on the screen regardless of how
long the line is - this is *Micro*Emacs, if these issues are blockers to you, try Emacs.

For move information visit: https://github.com/bjasspa/jasspa/

VERY QUICK START GUIDE
----------------------

For your platform download the single-file binary zip file:

  - Jasspa_MicroEmacs_<VERSION>_abin_<PLATFORM>_mecs.zip for console/terminal app, this should work on all computers of
    given platform.

  - Jasspa_MicroEmacs_<VERSION>_abin_<PLATFORM>_mews.zip  for window  based app, this should give best user  experience
    but may not work on all computers (particularly macOS where XQuartz is required).

Extract the executable for the zip and run.

This is a truly portable version, it will not alter/add to your environment or registry and will only create/edit
files you explicitly ask it to. So this version is ideal for trying MicroEmacs without affecting your computer.

For a more complete experience execute command  init-session, this creates a small user configuration directory in
your user area, allowing MicroEmacs to save setup changes and download supporting files such as the comprehensive help
docs and spelling dictionaries for numerous languages.


INSTALLERS
----------

- Homebrew installers for Linux and macOS, details to follow.

- Jasspa_MicroEmacs_<VERSION>_installer_windows.msi contains the binaries for Windows, the macros and help file, 
  simply download and run to create a fully working environment.

Spelling dictionary can be downloaded and installed by MicroEmacs as and when required.
 

SLOWER QUICK START GUIDE
------------------------

To build a more complete, native environment do the following:

1. Create a main MicroEmacs directory to hold all required files, this is typically ~/MicroEmacs on Unix or
   c:\Users\<user-name>\MicroEmacs on Windows, but the path can be anywhere. In following steps this path will be
   referred to as ~/MicroEmacs

2. Download the following zip package files:

    a. Required: Binaries for your platform (Jasspa_MicroEmacs_<VERSION>_bin_<PLATFORM>_binaries.zip)
     
    b. Required: Macro files (Jasspa_MicroEmacs_<VERSION>_macros.zip)

    c. Help file (Jasspa_MicroEmacs_<VERSION>_help_ehf.zip)

    d. Spelling dictionaries for any language you require (Jasspa_MicroEmacs_<VERSION>_spelling_<LANGUAGE>.zip)
     
    e. OpenSSL dynamic libraries (Jasspa_MicroEmacs_<VERSION>_bin_<PLATFORM>_openssl.zip) for https/ftps support (country
       permitting) 

3. Extract downloaded zip files into the ~/MicroEmacs directory you created in (1), this should have created:

    ~/MicroEmacs/bin - containing platform directories for each of the binary packages you downloaded and
     extracted.

    ~/MicroEmacs/macros - containing many macro files (*.emf), the me.ehf help file and other supporting
     files.

    ~/MicroEmacs/spelling - containing the dictionaries of the languages you downloaded.

4. Set the following environment variable:

     MEINSTALLPATH=~/MicroEmacs

   Setting an environment variable is platform & shell dependent.

5. Run:

     ~/MicroEmacs/bin/<platform>/mec

   The console version should run on all platforms, if the message '[Failed to load file me]' appears at the bottom of
   the window then you have either not set the environment variable correctly or it has not taken effect in your
   current shell/command prompt.

6. Try running:

     ~/MicroEmacs/bin/<platform>/mew

   This should always work on Windows but may not on Unix systems,  particularly macOS, where an X server (XQuartz) is
   required  to  support  the  window  interface.  Fixing  issues  causing  mew to not run is beyond the scope of this
   document.

7. Optionally copy the binaries (mec & mew) into a directory on your path (or add ~/MicroEmacs/bin/<platform> to your
   path. This simplifies running MicroEmacs in a terminal.

8. Run mew, if working, mec otherwise and follow the setup steps to create a basic user configuration area, this will
   create the directory:

     ~/MicroEmacs/<user-name>

   All your customisations should go in here. Run the command user-setup (in the Tools menu) to start customising the
   editor.

9. Enjoy fast, free editing!
