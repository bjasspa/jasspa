


Installation(1)                   microemacs                    Installation(1)




INSTALLATION 
     This page describes introductory  notes for the  installation and setup of
     MicroEmacs '02. 

  Quick Install 
     The quickest way  to install MicroEmacs  without reading the  rest of this
     document is to:- 

       *  Create a new directory i.e. me or microemacs. 
       *  Unpack the macros archive into this directory. 
       *  Unpack any spelling dictionaries into this directory. 
       *  Unpack the executable into this directory. 
       *  Run me from this directory. 

     On starting, use the mouse and configure the user from the menu bar:- 

       Help->User Setup 

     This allows the user  and screen settings to  be altered. On becoming more
     accustomed to the editor then a fuller installation may be performed. 

     Getting Help 
          See Contact Information for full  contact information. A mail archive
          exists at:- 

            http://groups.yahoo.com/group/jasspa/ 

          If you wish to  participate in the list  then you must first register
          by sending an empty mail message body to:- 

            jasspa-subscribe@yahoogroups.com 

          You  will  then  be  able  to  mail  any  questions  into  the group.
          Registration is  required  in  order to  prevent  spam  mailings from
          entering into the lists. 

  Distribution 
     MicroEmacs is distributed in the following files:- 

     Complete Installations 
          The Microsoft '95/'98/NT platforms may be installed using the Install
          Shield  installation  utility  and  do  not  require  the  components
          specified in later sections. 
          jasspame.exe - '95/'98/NT Self Extracting Install Shield Installation

     Executable Source Code 
          The source code release for MicroEmacs '02 contains makefiles (*.mak)
          for all supported  platforms. Microsoft  '95/'98/NT makefiles contain
          options at the top of the  makefile to enable/disable console and URL
          support. 
          mesrc.zip - Source code for all platforms 
          mesrc.tar.gz - Source code 

     Executable Images 
          medos.zip - DOS Executable 
          mewin32.zip - Windows 32' (95/98/NT) Executable 
          mewin32s.zip - Windows win32s (Win3.1/3.11) Executable 
          meirix6.gz - Silicon Graphics Irix 6 Executable 
          meaix43.gz - IBM's AIX 4.3 Executable 
          mehpux10.gz - Hewlett Packard HP-UX 10 Executable 
          mehpux11.gz - Hewlett Packard HP-UX 11 Executable 
          mesunos55.gz - Sun OS 5.5 Executable 
          mesunos56.gz - Sun OS 5.6 Executable 
          mesolx86.gz - Sun Solaris 2.6 Intel Platform Executable 
          melinux20.gz - Linux 2.0.0 Executable 
          mefreebsd.gz - Free BSD Executable 

     Help File Images (all platforms) 
          mewinhlp.zip - Windows Help file 
          mehtm.zip  - HTML Help files for 8.3 file systems (.htm) 
          mehtml.tar.gz - HTML Help files (.html) 

     Macro File Images (all platforms) 
          memacros.zip - Macro files 
          memacros.tar.gz - Macro files 

     Spelling Dictionaries (all platforms) 
          One of the following base  dictionaries is required for spelling. The
          extended dictionaries require the base dictionary and are recommended
          for  a  more   comprehensive  spelling  list.   Other  languages  are
          supported. 

          lsdmenus.zip - American rules and base dictionary. 
          lsdxenus.zip - American extended dictionary. 
          lsdmengb.zip - British rules and base dictionary. 
          lsdxengb.zip - British extended dictionary. 
          lsdmfifi.zip - Finnish rules and dictionary. 
          lsdmfrfr.zip - French rules and dictionary. 
          lsdmdede.zip - German rules and base dictionary. 
          lsdxdede.zip - German extended dictionary. 
          lsdmitit.zip - Italian rules and dictionary 
          lsdmplpl.zip - Polish rules and dictionary. 
          lsdmptpt.zip - Portuguese rules and dictionary. 
          lsdmeses.zip - Spanish rules and dictionary. 

          lsdmenus.tar.gz - American rules and base dictionary. 
          lsdxenus.gz - American extended dictionary. 
          lsdmengb.tar.gz - British rules and base dictionary. 
          lsdxengb.gz - British extended dictionary. 
          lsdmfifi.tar.gz - Finnish rules and dictionary. 
          lsdmfrfr.tar.gz - French rules and dictionary. 
          lsdmdede.tar.gz - German rules and base dictionary. 
          lsdxdede.gz - German extended dictionary. 
          lsdmitit.tar.gz - Italian rules and dictionary 
          lsdmplpl.tar.gz - Polish rules and dictionary. 
          lsdmptpt.tar.gz - Portuguese rules and dictionary. 
          lsdmeses.tar.gz - Spanish rules and dictionary. 

          NOTE: The binary versions of the executables held on the site include
          the platform name as part  of the executable name  i.e. me for DOS is
          called medos.exe. On installing the binaries onto the target machine,
          you should  rename  the  executable  to  me  or  me.exe,  whatever is
          appropriate. The ONLY exception to this rule is the Microsoft Windows
          executable where  mewin32.exe  should  be  renamed  to  me32.exe. Our
          reason for this naming is to  allow the executables to be unpacked in
          the same directory and not be confused with each other. 

  Quick Start Guild For All Platforms 
     Simply create a directory, down-load the files required (see list for each
     platform below) and extract  into this directory. From  a shell or command
     prompt, change to  the directory,  making it the  current one  (i.e. cd to
     it), and run the  executable. MicroEmacs '02 should  open with the on-line
     help page visible. 

     On Windows based systems this can also be achieved by creating a short-cut
     and setting the Working Directory in Properties to this path. 

     To enable MicroEmacs  to be  run from  any directory,  simply include this
     directory  in  you  PATH  environment  variable.  Alternatively,  copy the
     executable to  somewhere in  your  PATH and  set the  environment variable
     MEPATH to point to this directory. 

     MicroEmacs  '02  will  function  normally  in  this  environment,  but  in
     multi-user  environments  and  for  up-dating  purposes,  it  is  strongly
     recommended that a proper installation is used, see below. 

  Installation 
     DOS 

          Executable: 
               Compiled with DJGPP V1.0 

          Distribution components required: 
               medos.zip 
               memacros.zip 
               <spelling>.zip 

               mewinhlp.zip if you are using windows 3.1/3.11 

          Recommended installed components: 
               4dos - Command shell (giving stderr redirection). 
               grep - Version of grep (djgpp recommended) 
               make - Version of make (djgpp recommended) 
               diff - Version of diff (djgpp recommended) 

          Installation: 
               Create the directory c:\me (or other location) 

               Unzip the MicroEmacs components into c:\me 

               Edit "c:\autoexec.bat" and add the following lines:- 

                 SET MENAME=<name> 
                 SET PATH=%PATH%;c:\me 
                 SET MEPATH="c:\me" 

               Reboot the system. 

                    MicroEmacs may be run from the command line using 

                 me 

          Graphics Cards: 
               MicroEmacs may be configured to  the text modes of your graphics
               card. Refer to you graphics card  DOS text modes to identify the
               text modes supported by  your monitor. The  text mode number may
               be entered  into  the  user  monitor  configuration,  defined in
               Help->User Setup. 

          Running From Windows (3.x) 
               The DOS version of MicroEmacs may  be executed from a .pif file.
               Use  the  pif  editor  to  create  a  new  .pif  file  to launch
               MicroEmacs. The size  of the  DOS window may  be configured from
               the command  line,  set  the  terminal  size  using  one  of the
               following command lines:- 

                 me -c -v$TERM=E80x50        - 80 x 50 window 
                 me -c -v$TERM=E80x25        - 80 x 25 window. 

               We usually add the -c option so that MicroEmacs is executed with
               history information. This may be omitted if required. 

     Windows 3.1/3.11 

          Executable: 
               Compiled with Microsoft Developer 2.0 

          Helper DLL: 
               Under Win32s a  helper DLL  methnk16.dll is  required to perform
               the pipe-shell-command(2) in  a synchronous  manner. This should
               be installed into  the C:\WINDOWS\SHELL  directory. This (rather
               inelegantly) gets  around  the  problems of  spawning  a process
               under win32s due to a number  of Microsoft bugs in the operating
               system. Note:  that  on a  spawn  operation a  MS-DOS  window is
               visible, this is due to the  nature of the command shell on this
               platform which  has  a  tendency  to prompt  the  user  at every
               opportunity, hence a certain amount of interaction (which is out
               of our control) is necessary. 

               The helper DLL is compiled with a 16-bit Windows compiler - MSVC
               1.5. 

          Distribution components required: 
               mewin32s.zip 
               memacros.zip 
               mewinhlp.zip 
               <spelling>.zip 

          Recommended installed components: 
               4dos - command shell (giving stderr redirection) 
               grep - Version of grep (GNU port of grep recommended) 
               diff - Version of diff (GNU port of grep recommended) 
               make - use nmake or GNU port of make. 

          win32s 
               win32s is a  requirement on this  platform, typically taken from
               pw1118.exe which freely available on the Internet. 

          Installation: 
               This version  of Windows  does not  have a  install directory as
               '95/'98 and it is expected that the MS-DOS version will coexist.
               No  Install  Shield  installation  is  provided.  Install  in  a
               directory structure  similar to  MS-DOS. Install  the helper DLL
               methnk16.dll  in   the  C:\WINDOWS\SHELL   directory.  Create  a
               me32.ini(8) file  in the  C:\WINDOWS  directory to  identify the
               location of the MicroEmacs '02 components, this much the same as
               the '95/'98  file,  change  the  directory  paths  to  suite the
               install base. 

          Support Status: 
               The win32s release has not been used with vengeance, although no
               specific problems have been reported with this release. 

     Windows '95/'98/NT 

          Executable: 
               Compiled with Microsoft Developer 5.0 

          Install Shield 
               An Install  Shield  version  of  MicroEmacs  is  available which
               includes all of the distribution components. 

          Distribution components required: 
               mewin32.zip 
               memacros.zip 
               <spelling>.zip 
               mewinhlp.zip (optional) 

          Recommended installed components: 
               4dos or 4nt - command shell 
               grep - Version of grep (GNU port of grep recommended) 
               diff - Version of diff (GNU port of grep recommended) 
               make - use nmake or GNU port of make. 

          Installation: 
               Create  the  directory  "C:\Program Files\Jasspa\MicroEmacs" (or
               other location) 

               Unzip    the    MicroEmacs     components    into    "C:\Program
               Files\Jasspa\MicroEmacs" 

               Create the  file  "c:\windows\me32.ini"  and  add  the following
               lines:- 

                 [Defaults] 
                 mepath=C:\Program Files\Jasspa\MicroEmacs 
                 userPath=C:\Program Files\Jasspa\MicroEmacs 
                 fontfile=dosapp.fon 

               Create a short cut to MicroEmacs for the Desktop 

               Right click on the desk top 

                 => New 
                 => Short 
                 => Command Line: "c:\Program Files\Jasspa\MicroEmacs\me.exe -c"
                 => Short Cut Name: "MicroEmacs" 

               MicroEmacs may be executed from the shortcut. 

          Open Actions 
               Microsoft Windows 95/98/NT provide  short cut actions, assigning
               an open action to  a file. The short  cuts may be installed from
               the Install  Shieled  installation,  but  may  alternativelly be
               explictly defined by editing the registry file with regedit(1). 

               A file open  action in  the registry is  bound to  the file file
               extension, to bind a file extension  .foo to the editor then the
               following registry entries should be defined:- 

                 [HKEY_CLASSES_ROOT\.foo] 
                 "MicroEmacs_foo" 
                 [HKEY_CLASSES_ROOT\MicroEmacs_foo\DefaultIcon] 
                 "C:\Program File\JASSPA\MicroEmacs\meicons,23" 
                 [HKEY_CLASSES_ROOT\MicroEmacs_foo\Shell\open] 
                 "&Open" 
                 [HKEY_CLASSES_ROOT\MicroEmacs_foo\Shell\open\command] 
                 "C:\Program File\JASSPA\MicroEmacs\me32.exe -o "%1"" 

               In  the  previous  exaple  the  DefaultIcon  entry  is  the icon
               assigned to the file. This may be an icon taken from meicons.exe
               (in this case  icon number 23),  or may be  some other icon. The
               open  action  in  the   example  uses  the   -o  option  of  the
               client-server, which loads the  file into the current MicroEmacs
               '02 session, alternatively the  -c option may  be used to retain
               the previous  context, or  no option  if a  new session  with no
               other files loaded is started. 

               A generic open  for ALL files  may be defined  using a wildcard,
               this may  be  used  to  place a  MicroEmacs  edit  entry  in the
               right-click popup menu, as follows:- 

                 [HKEY_CLASSES_ROOT\*\shell] 
                 [HKEY_CLASSES_ROOT\*\shell\MicroEmacs] 
                 "&MicroEmacs" 
                 [HKEY_CLASSES_ROOT\*\shell\MicroEmacs\command] 
                 "C:\Program File\JASSPA\MicroEmacs\me32.exe -o "%1"" 

     UNIX 

          Executable: 
               Compiled with native compilers. 

          Distribution Components Required: 
               me<unix>.gz 
               memacros.tar.gz 
               <spelling>.gz 
               html.tar.gz (optional) 

          Installation: 
               It is  recommended  that  all files  are  placed  in /usr/local,
               although they may be installed locally. 

               Unpack the executable and placed in "/usr/local/bin" 

               Create the  new  directory  "/usr/local/microemacs",  unpack and
               install the memacros.tar.gz into this directory. 

               For csh(1) users execute  a "rehash" command  and then me(1) can
               be executed from the command line. 

               By  default  a  X-Windows  terminal  is  displayed,  ensure that
               $DISPLAY  and  $TERM  are  correctly  configured.  To  execute a
               terminal emulation then execute  me with the  -n option i.e. "me
               -n". Note that  this is  not required if  you are  using a vt100
               emulation. 

  Organizing a local user profile 
     MicroEmacs uses local  user configuration profiles  to store user specific
     information.  The  user  information  may  be  stored  in  the  MicroEmacs
     directory, or more typically in a users private directory. The environment
     variable $MENAME is typically used to determine the identity of the user. 

     The location  of  the  user  profile will  depend  upon  your installation
     configuration. 

     Single Machine 
          For a  single  user  machine  it  is  typically  easiest  to  use the
          installed MicroEmacs directory where  user specific files are placed.
          This method, although  not recommended,  is simple as  all files that
          are executed are in the same location. The $MEPATH is not changed. 

     UNIX The UNIX environment is  fairly easy and operates  as most other UNIX
          applications. The user should create  a MicroEmacs directory in their
          home  directory  for  their  own  local  configuration.  Assigning  a
          suitable name such  as "microemacs", or  if the file  is to be hidden
          ".microemacs". 

          The $MEPATH environment  variable of  the user should  be modified to
          include  the  users   MicroEmacs  path  BEFORE   the  default  macros
          MicroEmacs path i.e. 

          Ksh/Zsh: 
               export MEPATH=$HOME/microemacs:/usr/local/bin 

          Csh/Bash: 
               setenv MEPATH $HOME/microemacs:/usr/local/bin 

          Where $HOME is defined as "/usr/<name>" (typically by default). 

     DOS/Windows 
          DOS and Windows are  a little more tricky  as numerous directories at
          the root level are more than  a little annoying. It is suggested that
          the user directory  is created  as a sub-directory  of the MicroEmacs
          directory. i.e. 

               "c:\me\<user>" for DOS 

          or   "c:\Program Files\Jasspa\MicroEmacs\<user>" for Windows 

          The $MEPATH  environment variable  (see  me32.ini(8) for  Windows) is
          modified  to  include  the   user  component  before  the  MicroEmacs
          component where $MEPATH is defined i.e. 

            SET MEPATH=c:\me\<user>;c:\me 

          where <user> is the user name (or $MENAME). 

  Alternative Directory Configurations 
     Numerous other configurations exist to  organize the macro directories, to
     take the  directory  organization  to  the extreme  then  it  is sometimes
     easiest to  keep all  of  the macro  components separate.  An installation
     layout which encompasses different macro directories for:- 

       *  User profiles - 1 per user. 
       *  Shared company profiles - 1 per organization. 
       *  MicroEmacs macros which are updated from time to time. 

     The configuration on different systems may be defined as follows:- 

     UNIX The shared files are placed in /usr/local 

            /usr 
               \ 
              local 
                 \ 
              microemacs   - Spelling + standard macros 
                   \ 
                 company   - Company specific files 

          The user profile is stored in the users directory 

            /usr 
              \ 
            <name> 
                \ 
             microemacs    - User specific files 

          The user should configure the $MEPATH as: 

            MEPATH=$(HOME)/microemacs:/usr/local/microemacs/company:/usr/local/microemacs

     DOS/WINDOWS 
          For DOS and MS-Windows  environments, bearing in  mind the problem of
          the root directory, then it is easier  to use the "me" directory as a
          place holder for  a number of  sub-directories, using a configuration
          such as:- 

                        c: 
                        | 
                        me      - Place holder directory 
                      / | \ 
                    /   |   \ 
               <name> macros company 

          The user should configure the $MEPATH as:- 

            SET MEPATH=c:\me\<name>;c:\me\company;c:\me\macros 

  User Profile Files 
     Files contained in the user profiles typically include:- 

     <name>.emf - The users start up profile. 
     <name>.edf - The users spelling dictionary. 
     <name>.erf - The users registry configuration file. 

     These files are established  from the menu  "Help->User Setup". The "Setup
     Path" item  defines  the  location  of the  files,  but  must  be MANUALLY
     included in the $MEPATH environment. 

  Company Profiles 
     Company profiles  include standard  files and  extensions to  the standard
     files which may be  related to a company,  this is typically <company>.emf
     where <company> is the name of the company. 

     The directory may also  include template files  etf(8) files which defines
     the standard header  template used  in the  files. Files  in the "company"
     directory would over-ride the standard template files. 

     The company  directory  should be  added  to  the $MEPATH  after  the user
     profile and before the MicroEmacs standard macro directory. 

SEE ALSO 
     $MENAME(5), $MEPATH(5), Company Profiles, File Hooks, File Language
     Templates, User Profiles. 

COMPANY PROFILES 
     This section describes how  a company profile  should be incorporated into
     MicroEmacs  '02.  A  company  profile  defines  a  set  of  extensions  to
     MicroEmacs which  encapsulate settings  which are  used on  a company wide
     basis. This  type  of configuration  is  typically used  with  a networked
     (shared) installation. The company profile would typically include:- 

       *  Name of the company. 
       *  Standard header files including company copyright statements. 
       *  Standard file layouts 
       *  Company defined language extensions. 

  Location Of The Company Information 
     It is suggested that  all of the company  extensions applied to MicroEmacs
     '02 are  performed  in a  separate  directory location  which  shadows the
     MicroEmacs standard macro file directory.  This enables the original files
     to be sourced if a  user does not want to  include the company files. This
     method  also  allows  MicroEmacs  to  be  updated  in  the  future, whilst
     retaining the  company files.  For  our example,  we  shall use  a company
     called JASSPA,  you  should replace  references  to jasspa  with  your own
     company name. The steps involved are laid out as follows:- 

     Create a new company directory 
          You may  skip  this step  if  you are  going  to modify  the standard
          installation. 

          Create a new directory to hold the company information. i.e. 

               /usr/local/microemacs/jasspa - UNIX 
               c:\Program Files\JASSPA\MicroEmacs\jasspa - Microsoft 

          Modify the $MEPATH(5)  of the (of  all users) to  include the company
          directory on the search path i.e. 

          UNIX Users edit their local $MEPATH or a base $MEPATH is added to the
               system .login or .profile scripts. 

                    MEPATH=/usr/local/microemacs 
                    MEPATH=/usr/local/microemacs/jasspa:$MEPATH 

          Microsoft Windows Platforms 
               Edit the me32.ini  file and  modify the mepath  entry to reflect
               the location of the company directory:- 

                    mepath=C:\Prog....\Mic...\macros\jasspa;C:\Prog...\Mic...\macros

          DOS Platforms 
               Edit the  autoexec.bat  file and  modify  MEPATH to  include the
               company directory location. 

                    SET MEPATH=c:\me\jasspa;c:\me 

  Content Of The Company Information 

     Company macro file 
          The company  file  is  typically  called by  the  company  name (i.e.
          jasspa.emf) create a new company file. The file includes your company
          name and hook functions for any new file types that have been defined
          for the company, an example company  file for Jasspa might be defined
          as:- 

            ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
            ; 
            ;  Author        : Jasspa 
            ;  Created       : Thu Jul 24 09:44:49 1997 
            ;  Last Modified : <190698.2225> 
            ; 
            ;  Description     Extensions for Jasspa 
            ; 
            ;  Notes 
            ; 
            ;  History 
            ; 
            ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
            ; Define the name of the company. 
            set-variable %company-name "Jasspa" 
            ; Add Jasspa specific file hooks 
            ; Make-up foo file hook 
            add-file-hook ".foo"    fhook-foo 
            1 add-file-hook "-!-[ \t]*foobar.*-!-" fhook-foo ; -!- foobar -!- 
            ; Override the make with localised build command 
            set-variable %compile-com "build" 

          The file contains  company specific  file hooks  and the  name of the
          company. 

     Other Company Files 
          Files defined on  behalf of the  company are included  in the company
          directory. These would include:- 

            *  Template header files etf(8). 
            *  Hook file  definitions (hkXXX.emf)  for company  specific files,
               see add-file-hook(2). 
            *  Extensions to  the  standard  hook  definitions  (myXXX.emf) for
               company specific language extensions to the standard hook files.
               See File Hooks and File Language Templates. 

SEE ALSO 
     $MENAME(5), $MEPATH(5), File Hooks, File Language Templates, Installation,
     user-setup(3), User Profiles. 

USER PROFILES 
     This section  describes how  a  user profile  should be  incorporated into
     MicroEmacs '02. A user  profile defines a set  of extensions to MicroEmacs
     which encapsulates settings which are used by an individual user. 

     The user profile allows:- 

       *  Saving of the last session (history), allowing the next invocation of
          MicroEmacs '02 to restore your previous session. 
       *  Personalized spelling dictionaries. 
       *  Redefinition of MicroEmacs '02, allowing the editor to be tailored to
          an  individual's  requirements.  Including  the  re-binding  of keys,
          modification of the screen colors. Definition of personal macros etc.

  Identification 
     In order to identify a user  MicroEmacs '02 uses information in the system
     to determine the name of  the user, and in  turn the configuration to use.
     On all  systems the  value  of the  environment variable  $MENAME(5) takes
     priority over any other means of  user identification. If this variable is
     not defined  then  the  host  system  typically  provides  a  mechanism to
     determine the current user. DOS and Windows systems present problems where
     a login prompt is not supplied. 

     Each of the supported platforms are now described. 

     UNIX The environment variable $LOGNAME  is defined. This  is the user name
          used by the system. 

     DOS  MS-DOS typically  has no  concept  of the  user  name. The  user name
          should be  defined  in the  autoexec.bat  file,  choose a  name  of 8
          characters or less, i.e.  to fix the  user name to  fred then add the
          following line:- 

            SET MENAME=fred 

          Remember to re-boot the  system before the  new command takes effect.
          (see the next step, there is another change to autoexec.bat). 

     Microsoft Windows 
          Microsoft windows environments may, or may not, have logging enabled.
          If you have to  log into your system  then a login identification has
          been supplied  and  will  be recognized  by  MicroEmacs,  setting the
          environment variable $MENAME(5) to this value. 

          If login is not enabled then  the me32.ini(8) file may be modified to
          provide a  default login  name. To  add  the user  fred then  add the
          following lines to the ini file:- 

            [guest] 
            MENAME=fred 

          If login  is  subsequently enabled  on  the system  then  these lines
          should be removed.  These lines  force the user  identification to be
          fred. 

          The above  technique may  be used  within the  windows environment to
          modify your login  name. Assuming  that the  system administrator has
          assigned fred a user  login name of fwhite,  and fred requires all of
          his configuration files to  be the same name  as his UNIX login which
          is fred. Then fred may force his  user name to fred from the me32.ini
          file as follows:- 

            [fwhite] 
            MENAME=fred 

          Once fred has  entered MicroEmacs  he will  adopt his  new login name
          which will be used to identify his  own files etc. The action of this
          statement is  to  force the  environment  variable $MENAME  to  a new
          value. Any other environment variables may be forced in this way i.e.
          $HOSTNAME is a  good candidate here  as the me32.ini  is local to the
          machine. 

     Shared Platforms 
          Platforms may share the  same set of  configuration files. Consider a
          system which  may  boot  under  MS-DOS, Windows  '98,  NT  and Linux.
          Provided that the macro  files are located on  a file system that may
          be mounted by all  of the other operating  systems and the $MEPATH is
          set appropriately, then a single set of MicroEmacs macro files may be
          shared across all platforms. 

  Personal MicroEmacs Directory 
     The private user profile is stored  in a separate directory. The directory
     that MicroEmacs uses must be created  by the user, create the directory in
     your local file system. In addition, the MicroEmacs search path $MEPATH(5)
     should be modified to include your new MicroEmacs personal directory. 

     UNIX Create  in  your  local  directory,  typically  called  microemacs or
          .microemacs (if it is to be hidden). 

          Add/modify  the  $MEPATH(5)  environment  variable  to  include  your
          personal directory in your .login,  .chsrc or .profile file, the file
          and exact syntax  will depend upon  your shell. For  a Korn shell the
          following line would be added to the .profile file:- 

            export MEPATH=$HOME/.microemacs:/usr/local/microemacs 

          Where $HOME is assumed  to be the users  login home directory, or use
          the directory location of your new directory. 

     DOS  For MS-DOS environments, there is  typically no user directory, it is
          suggested that  the  user  directory  is  created  in  the MicroEmacs
          directory, use the $MENAME defined in the previous step i.e. 

            mkdir c:\me\fred 

          Change  the  $MEPATH(5)  in  the  autoexec.bat  to  include  the  new
          directory i.e. 

            SET MEPATH=c:\me\fred;c:\me 

     Windows 
          Windows environments,  the  me32.ini(8)  userPath  entry  defines the
          location of the  user profile directories,  within the Install Shield
          installation, the me32.ini is typically defined as:- 

            userPath=C:\Program Files\JASSPA\MicroEmacs 

          Create your MicroEmacs personal directory in this folder, the name of
          the folder should be  your login name or  $MENAME, depending upon how
          your name is identified. 

  Creating Your Profile 
     Once you have created a new directory to store your user profile, create a
     default profile  for  yourself  from  MicroEmacs  using  the user-setup(3)
     dialog:- 

       Help => User Setup 

     Fill in the entries  in the dialog,  and ensure that  Save is depressed on
     exit to write the files. 

     The dictionaries often  present difficulties  the first time,  a prompt to
     save the dictionary requires  the full pathname and  the name of the file,
     the pathname  is  the  path  to  your  personal  folder,  the  filename is
     typically your username.edf. Once the file  is created you will not have a
     problem in the future. 

  The User Profile 
     Files created in the user directory include:- 

       *  Setup  registry  and  previous   session  history  username.erf,  see
          erf(8)). This  stores the  user-setup settings  and also  the context
          from your previous edit session. 
       *  Users start-up file username.emf, see  emf(8) the user may make local
          changes to  MicroEmacs in  this file,  this may  include changing key
          bindings, defining new  hook functions etc.  You should over-ride the
          standard MicroEmacs  settings  from  your start-up  file  rather than
          modifying the standard MicroEmacs files. 
       *  Personal spelling  dictionary  username.edf,  see  edf(8).  This file
          contains your  personal spelling  modifications,  any words  that are
          added to the spelling dictionary are added to this file. 

     In addition to the above,  if new file hooks  are defined then they should
     be added to this directory (if they are not global to the company). 

EXAMPLE 
     The following are examples of some individuals start-up files:- 

       ; Jon's special settings 
       ; 
       ; Last Modified <190698.2226> 
       ; 
       ; Macro to delete the whitespace, or if an a word all of the 
       ; word until the next word is reached. 
       define-macro super-delete 
           set-variable #l0 0 
           !while &not &sin @wc " \t\n" 
               forward-char 
               set-variable #l0 &add #l0 1 
           !done 
           !repeat 
               !force forward-char 
               !if $status 
                   set-variable #l0 &add #l0 1 
               !endif 
           !until &or &seq @wc "" &not &sin @wc " \t\n" 
           #l0 backward-delete-char 
           !return 
       !emacro 
       ; Make a previous-buffer command. 
       define-macro previous-buffer 
           &neg @# next-buffer 
       !emacro 
       ; spotless; Perform a clean and remove any multi-blank lines. 
       define-macro spotless 
           -1 clean 
       !emacro 
       ; comment-adjust; Used for comments in electric-c mode (and the other 
       ; electic modes. Moves to the comment fill position, saves having to mess
       ; around with comments at the end of the line. 
       0 define-macro comment-adjust 
           ; delete all spaces up until the next character 
           !while &sin @wc " \t" 
               forward-delete-char 
           !done 
           ; Fill the line to the current $c-margin. We use this as 
           ; this is the only variable that tells us where the margin 
           ; should be. 
           !if &gre $window-acol 0 
               backward-char 
               !if &sin @wc " \t" 
       	    forward-delete-char 
                   !jump -4 
               !else 
                   forward-char 
               !endif 
           !endif 
           ; Now fill to the $c-margin 
           &sub $c-margin $window-acol insert-string " " 
       !emacro 
       ; Macro to force buffer to compile buffer for C-x ' 
       define-macro compile-error-buffer 
           !force delete-buffer *compile* 
           change-buffer-name "*compile*" 
       !emacro 
       ; 
       ; Set up the bindings. 
       ; 
       global-bind-key super-delete            "C-delete" 
       global-bind-key beginning-of-line       "home" 
       global-bind-key end-of-line             "end" 
       global-bind-key undo                    "f4" 
       !if &seq %emulate "ERROR" 
           global-bind-key comment-adjust      "esc tab" 
           global-bind-key comment-adjust      "C-insert" 
           ; Like a korn shell please. 
           ml-bind-key tab "esc esc" 
       !endif 
       ; 
       ; Setup for windows and UNIX. 
       ; 
       ; Define my hilighting colour for Windows and UNIX. 
       !if &equ &band $system 0x001 0 
           !if &not &seq $platform "win32" 
               ; Small bold font is better for me. 
               change-font "-*-clean-medium-r-*-*-*-130-*-*-*-*-*-*" 
               ; Small non-bold font. 
               ; change-font "-misc-fixed-medium-r-normal--13-*-*-*-c-70-iso8859-1"
               ; Change the size of the screen 
               change-frame-width 82 
               change-frame-depth 50 
           !endif 
       !endif 
       ; Change the default diff command-line for GNU diff utility all platforms
       set-variable %diff-com "diff --context --minimal --ignore-space-change --report-identical-files --recursive"
       set-variable %gdiff-com "diff --context --ignore-space-change -w" 
       ; Setup for cygnus 
       !if &seq $platform "win32" 
           set-variable %cygnus-bin-path "c:/cygwin/bin" 
           set-variable %cygnus-hilight 1 
           set-variable %cygnus-prompt "$" 
       !endif 
       ; Set up the ftp flags. The letters have the following meaning: 
       ; c   - Create a console (*ftp-console* for ftp, *http-console* for http)
       ; s   - Show the console 
       ; p   - Show download progress ('#' every 2Kb downloaded) 
       set-variable %ftp-flags "csp" 
       ; Info files 
       ;To hilight the .info and also the dir file 
       add-file-hook ".info dir"                                   fhook-info   ; Info-files
       ;To hilight all info files without the extension .info 
       ;but starting with the text "This is info file.. 
       -2 add-file-hook "This is Info file"                        fhook-info 

       ; Finished 
       ml-write "Configured to Jon's requirements" 

SEE ALSO 
     $MEPATH(5), $MENAME(5), user-setup(3), Company Profiles, File Hooks, File
     Language Templates, Installation. 

