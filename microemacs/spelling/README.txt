> -!- document -!-
> Copyright (c) 2004-2024 JASSPA (www.jasspa.com).
>
> This is part of JASSPA's MicroEmacs, see the LICENSE file for licensing and
> copying information.
>

Place any spelling  dictionaries  in this directory. The spelling
dictionaries  can be downloaded from  http://www.jasspa.com.  The
spelling dictionaries supported at the time of writing are:

ls_dede - German spell rules and dictionary.
ls_elgr - Greek spell rules and dictionary.
ls_engb - British English spell rules and dictionary.
ls_enus - American English spell rules and dictionary.
ls_eses - Spanish spell rules and dictionary.
ls_fifi - Finnish spell rules and dictionary.
ls_frfr - French spell rules and dictionary.
ls_itit - Italian spell rules and dictionary.
ls_plpl - Polish spell rules and dictionary.
ls_ptpt - Portuguese spell rules and dictionary.
ls_ruru - Russian spell rules and dictionary.

The files  contained  within a  spelling  archive  use the naming
convention:

    lsr<Language><Country>.emf  Spelling rules.
    lsd<Language><Country>.emf  Base dictionary.
    README_<Language><Country>.txt  Dictionary readme.
    LICENSE_<Language><Country>.txt  Dictionary license information.


Usage
-----
Spelling is setup using user-setup (main menu Tools -> User Setup),
on the Start-Up tab the Language setting can be used to configure
the initial dictionary to load and the Spelling section configures
whether MicroEmacs' Auto-Spelling feature is enabled.
 
When enabled, the automatic spell mode performs background spell
checking and highlights erroneous words. Spelling errors are
hilighted in most color schemes. To correct then move the mouse over
the word and right select, the Auto spell entry should display in a
pop-up, select the replacement word to correct.

You can spell the whole buffer using:

    esc-x spell-buffer
    esc-x auto-spell-buffer            

Multiple  dictionaries may be supported on the same platform. The
active dictionary may be changed from the dialog of spell buffer.

Installation
------------
    MicroEmacs can automatically download and install languages when 
    required as long as your computer has internet access.
    
    If the microemacs-install script was used to install MicroEmacs,
    the microemacs-update script can be used to install a dictionary
    by running the script with a single argument of the language to
    install, i.e.:
    
        microemacs-update enus
    
    Alternatively, download the spelling dictionary and decompress
    into this directory.
        
    For a zip archive:-
    
        cd /path_to/jasspa/spelling
        unzip /path_to_archive/ls_enus.zip
    
    For tar+gzip archive (gtar on some platforms):-
    
        cd /path_to/jasspa/spelling
        tar zxvf /path_to_archive/ls_enus.tar.gz
