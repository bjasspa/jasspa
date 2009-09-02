; Setup script for JASSPA MicroEmacs using Inno Setup

[Setup]
AppName=JASSPA MicroEmacs
AppVerName=JASSPA MicroEmacs 2009
AppVersion=2009.09.09
;
DefaultDirName={pf}\JASSPA\MicroEmacs
DefaultGroupName=JASSPA MicroEmacs
AppCopyright=Copyright © 1988-2009 JASSPA
MinVersion=4,4
Uninstallable=yes
OutputDir=.\output
ShowTasksTreeLines=yes
SolidCompression=yes
Compression=lzma/max
LicenseFile=JASSPA\MicroEmacs\COPYING
;
AppPublisher=JASSPA
AppPublisherURL=http://www.jasspa.com
ChangesAssociations=yes

[Languages]
Name: "en"; MessagesFile: "jasspa.isl"

; Custom message
[Messages]
SelectComponentsLabel2=Select the components you want to install; clear the components you do not want to install. Click Next when you are ready to continue.

[Types]
Name: "typical"; Description: "Typical installation";
Name: "full"; Description: "Full installation";
Name: "compact"; Description: "Minimal omitting spelling dictionaries";
Name: "custom"; Description: "Select the additional componets required"; Flags: iscustom

[Components]
Name: "base"; Description: "Base installation"; Types: typical full compact custom; Flags: fixed
Name: "help"; Description: "Embedded help information (Strongly recommended)"; Types: typical full
Name: "helpx"; Description: "Getting started guide and other help"; Types: typical full
Name: "enus"; Description: "American spelling dictionary"; Types: typical full
Name: "engb"; Description: "British spelling dictionary"; Types: full
Name: "fifi"; Description: "Finnish spelling dictionary"; Types: full
Name: "frfr"; Description: "French spelling dictionary"; Types: full
Name: "dede"; Description: "German spelling dictionary"; Types: full
Name: "itit"; Description: "Italian spelling dictionary"; Types: full
Name: "plpl"; Description: "Polish spelling dictionary"; Types: full
Name: "ptpt"; Description: "Portuguese spelling dictionary"; Types: full
Name: "ruye"; Description: "Russian YE spelling dictionary"; Types: full
Name: "ruyo"; Description: "Russian YO spelling dictionary"; Types: full
Name: "eses"; Description: "Spannish spelling dictionary"; Types: full
Name: "pixel"; Description: "MicroEmacs pixel files"; Types: full
Name: "contrib"; Description: "Useful contributed script files"; Types: full
Name: "utils"; Description: "Executables for find, grep, fgrep, egrep and diff"; Types: full; Flags: dontinheritcheck

[Tasks]
Name: "extmedit"; Description: "Add a MicroEmacs Edit option to right-mouse context menu (Recommended)"
Name: "ext"; Description: "Associate file extensions to MicroEmacs"
Name: "ext/me"; Description: "Make MicroEmacs the default editor for its own files (.eaf/.eff/.emf/.erf/.esf/.etf)"
Name: "ext/bup"; Description: "Make MicroEmacs the default editor for JASSPA backup files (.~/.#/.~0~)"
Name: "ext/jst"; Description: "Make MicroEmacs the default editor for JASSPA Structred text files (.jst)"
Name: "ext/c"; Description: "Make MicroEmacs the default editor for C/C++ files (.c/.cpp/.def/.h)"; Flags: unchecked
Name: "ext/y"; Description: "Make MicroEmacs the default editor for Lex/Yacc files (.l/.y)"; Flags: unchecked
Name: "ext/txt"; Description: "Make MicroEmacs the default editor for text files (.txt/.text)"; Flags: unchecked
Name: "ext/tex"; Description: "Make MicroEmacs the default editor for LaTeX files (.tex/.bib)"; Flags: unchecked
Name: "ext/man"; Description: "Make MicroEmacs the default editor for UNIX manual page files (.man)"; Flags: unchecked
Name: "ext/roff"; Description: "Make MicroEmacs the default editor for UNIX troff files (.0/.1/../.9)"; Flags: unchecked
Name: "ext/jdoc"; Description: "Make MicroEmacs the default for JASSPA documentation files (.tni/.sm/.nrs)"; Flags: unchecked
Name: "ext/mak"; Description: "Make MicroEmacs the default editor for Makefiles (.mak/.make)"; Flags: unchecked
Name: "ext/asm"; Description: "Make MicroEmacs the default editor for Assembler files (.asm/.s)"; Flags: unchecked
Name: "ext/pas"; Description: "Make MicroEmacs the default editor for Pascal files (.pas/.p)"; Flags: unchecked
Name: "ext/f77"; Description: "Make MicroEmacs the default editor for Fortran files (.f77/.f)"; Flags: unchecked
Name: "ext/tcl"; Description: "Make MicroEmacs the default editor for TCL files (.tcl)"; Flags: unchecked
Name: "ext/sh"; Description: "Make MicroEmacs the default editor for shell files (.bash/.csh/.ksh/.sh/.zsh)"; Flags: unchecked
Name: "ext/awk"; Description: "Make MicroEmacs the default editor for awk files (.awk/.gawk/.nawk)"; Flags: unchecked
Name: "ext/php"; Description: "Make MicroEmacs the default editor for PHP files (.php)"; Flags: unchecked
Name: "ext/asp"; Description: "Make MicroEmacs the default editor for ASP files (.asp)"; Flags: unchecked
Name: "ext/log"; Description: "Make MicroEmacs the default editor for logging files (.log)"; Flags: unchecked
Name: "ext/err"; Description: "Make MicroEmacs the default editor for error files (.err)"; Flags: unchecked
Name: "ext/out"; Description: "Make MicroEmacs the default editor for output files (.out)"; Flags: unchecked

[Dirs]
Name: "{app}\company"; Flags: uninsalwaysuninstall
Name: "{app}\contrib"; Flags: uninsalwaysuninstall
Name: "{app}\macros"; Flags: uninsalwaysuninstall
Name: "{app}\spelling"; Flags: uninsalwaysuninstall

[Files]
;
; Base files
;
Source: "JASSPA\MicroEmacs\me32.exe";  DestDir: "{app}"
Source: "JASSPA\MicroEmacs\meicons.exe";  DestDir: "{app}"
; Help Up to and including XP
Source: "JASSPA\MicroEmacs\build.txt";  DestDir: "{app}"
Source: "JASSPA\MicroEmacs\COPYING";  DestDir: "{app}"
Source: "JASSPA\MicroEmacs\cygwin.txt";  DestDir: "{app}"
Source: "JASSPA\MicroEmacs\faq.txt";  DestDir: "{app}"
Source: "JASSPA\MicroEmacs\infolist.txt";  DestDir: "{app}"
Source: "JASSPA\MicroEmacs\license.txt";  DestDir: "{app}"
Source: "JASSPA\MicroEmacs\patch.txt";  DestDir: "{app}"
Source: "JASSPA\MicroEmacs\readme.txt";  DestDir: "{app}"; Flags: isreadme
Source: "JASSPA\MicroEmacs\company\README.txt";  DestDir: "{app}\company\"
;
; Contributions
;
Source: "JASSPA\MicroEmacs\contrib\ME_4_all.reg";  DestDir: "{app}\contrib\"; Components: contrib
Source: "JASSPA\MicroEmacs\contrib\mesetup.reg";  DestDir: "{app}\contrib\"; Components: contrib
Source: "JASSPA\MicroEmacs\contrib\myipipe.emf";  DestDir: "{app}\contrib\"; Components: contrib
Source: "JASSPA\MicroEmacs\contrib\readme.txt";  DestDir: "{app}\contrib\"; Components: contrib
Source: "JASSPA\MicroEmacs\contrib\user.emf";  DestDir: "{app}\contrib\"; Components: contrib
;
; Pixel maps are extras
;
Source: "JASSPA\MicroEmacs\pixmaps\me.l.pm";  DestDir: "{app}\pixmaps"; Components: pixel
Source: "JASSPA\MicroEmacs\pixmaps\me.m.pm";  DestDir: "{app}\pixmaps"; Components: pixel
Source: "JASSPA\MicroEmacs\pixmaps\me.t.pm";  DestDir: "{app}\pixmaps"; Components: pixel
Source: "JASSPA\MicroEmacs\pixmaps\me.xpm";  DestDir: "{app}\pixmaps"; Components: pixel
Source: "JASSPA\MicroEmacs\pixmaps\me_128.png";  DestDir: "{app}\pixmaps"; Components: pixel
Source: "JASSPA\MicroEmacs\pixmaps\me_gnome_48.png";  DestDir: "{app}\pixmaps"; Components: pixel
Source: "JASSPA\MicroEmacs\pixmaps\me_l.png";  DestDir: "{app}\pixmaps"; Components: pixel
Source: "JASSPA\MicroEmacs\pixmaps\me_m.png";  DestDir: "{app}\pixmaps"; Components: pixel
Source: "JASSPA\MicroEmacs\pixmaps\me_s.png";  DestDir: "{app}\pixmaps"; Components: pixel
Source: "JASSPA\MicroEmacs\pixmaps\mini-me.xpm";  DestDir: "{app}\pixmaps"; Components: pixel
Source: "JASSPA\MicroEmacs\pixmaps\ne.xpm";  DestDir: "{app}\pixmaps"; Components: pixel
Source: "JASSPA\MicroEmacs\pixmaps\ne_l.png";  DestDir: "{app}\pixmaps"; Components: pixel
Source: "JASSPA\MicroEmacs\pixmaps\ne_m.png";  DestDir: "{app}\pixmaps"; Components: pixel
Source: "JASSPA\MicroEmacs\pixmaps\ne_s.png";  DestDir: "{app}\pixmaps"; Components: pixel
;
; Executables
;
Source: "JASSPA\MicroEmacs\find.exe";  DestDir: "{app}"; Components: utils
Source: "JASSPA\MicroEmacs\grep.exe";  DestDir: "{app}"; Components: utils
Source: "JASSPA\MicroEmacs\diff.exe";  DestDir: "{app}"; Components: utils
Source: "JASSPA\MicroEmacs\fgrep.exe";  DestDir: "{app}"; Components: utils
Source: "JASSPA\MicroEmacs\egrep.exe";  DestDir: "{app}"; Components: utils
;
; Base macros
;
Source: "JASSPA\MicroEmacs\macros\2dos.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\2mac.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\2unix.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\2win.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\abbrev.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\abbrlist.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\ada.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\aix.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\asmx86.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\asn1.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\asp.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\au3.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\au3.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\benchmrk.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\bibtex.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\bookmark.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\buffinit.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\c.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\c.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\calc.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\cfind.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\cfm.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\cfm.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\charset.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\charsutl.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\clearcs.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\cmacros.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\cobol.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\collapse.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\comment.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\cpp.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\cpp.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\crosswd.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\ctags.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\cvs.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\cygwin.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\darwin.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\dmf.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\doc.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\docmacro.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\docutl.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\dos.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\draw.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\ehftools.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\emf.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\emf.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\emftags.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\etfinsrt.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\euphor.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\euphor.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\f.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\f90.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\fattrib.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\favorite.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\fileopen.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\filetool.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\filetype.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\find.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\fold.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\format.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\freebsd.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\ftp.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\games.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\gdiff.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\gentags.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\h.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkada.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkapache.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkasmx86.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkasn1.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkasp.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkasp.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkau3.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkawk.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkbibtex.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkbinary.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkblist.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkbnf.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkc.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkc.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkcfm.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkcfms.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkcobol.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkcpp.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkcpp.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkcss.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkcygwin.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkdiff.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkdirlst.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkdirtre.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkdoc.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkdos.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkdot.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkehf.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkemf.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkemf.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkerf.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkeuphor.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkf90.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkfvwm.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkhtml.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkhtml.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkidl.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkimake.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkinfo.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkini.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkipipe.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkiss.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkjava.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkjs.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkjsp.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkjsp.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkjst.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkjst.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hklatex.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hklhtml.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hklisp.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hklists.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hklua.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkm4.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkmake.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkman.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkmeta.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkmhg.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkmod2.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hknroff.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hknroff.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkpascal.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkperl.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkphp.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkphp.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkphps.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkpls.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkpseudo.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkpsf.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkpython.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkr.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkreg.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkruby.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkrul.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hksamba.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkscheme.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hksgml.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkshell.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkslang.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkspec.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hksql.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hktcl.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hktexi.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hktxt.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkvb.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkverilg.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkvhdl.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkvrml.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkwiki.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkxml.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hkyacc.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hpp.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\hpux.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\html.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\html.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\htmlcore.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\htmltool.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\htmlutil.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\idl.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\imake.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\insdate.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\irix.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\itemlist.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\java.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\java.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\javatags.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\js.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\js.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\jsp.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\jst.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\jst.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\jst2html.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\jst2ltx.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\jst2rtf.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\jst2text.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\jstags.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\jstprint.erf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\jstutl.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\keyboard.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\killlist.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\language.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\langutl.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\latex.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\latex.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\linux.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\list.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\mahjongg.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\mail.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\mailmerg.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\majormod.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\makefile.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\matchit.edf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\matchit.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\me.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\mecua.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\meemacs.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\melogo.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\meme3_8.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\menedit.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\meserver.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\metris.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\mhg.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\misc.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\mod2.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\mouse.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\mouseosd.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\mouserec.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\msshift.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\narrow.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\newcomp.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\newuser.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\newuser.erf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\newuser.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\notes.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\notesutl.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\nroff.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\ntags.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\null";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\openbsd.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\openstep.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\organiza.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\organizd.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\organize.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\organizi.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\organizl.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\organizm.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\organizt.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\organizw.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\osd.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\osdcua.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\osdhelp.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\osdmisc.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\osdnedit.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\osf1.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\pagefile.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\pascal.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\pascal.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\password.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\patience.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\perl.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\perl.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\perltags.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\php.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\pls.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\pls.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\print.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\print.erf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\printall.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\printd.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\printd.erf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\printepc.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\printeps.erf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\printers.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\printf.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\printhpd.erf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\printhpl.erf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\printhtm.erf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\printstp.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\printwdw.erf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\psf.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\python.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\python.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\rect.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\ruby.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\rul.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\scheme.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\schemeb.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\schemebc.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\schemebh.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\schemech.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\schemed.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\schemegb.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\schemege.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\schemegg.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\schemego.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\schemej.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\schemel.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\schemelj.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\schemelm.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\schememd.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\schememw.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\schemepd.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\schemepl.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\schemes.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\schemesf.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\schemetb.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\schemetw.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\schemevi.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\schemosd.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\search.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\session.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\sessnstp.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\sgml.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\spec.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\spell.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\spellaut.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\spellutl.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\ssaver.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\sunos.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\tcl.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\tcl.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\tcltags.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\textags.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\toolbar.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\tooldef.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\toollist.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\tools.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\toolstd.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\toolstp.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\triangle.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\txt.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\unixterm.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\userstp.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\utils.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\vbtags.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\vhdl.eaf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\vhdl.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\vm.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\watch.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\win32.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\word.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\xfind.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\xml.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\xmlutil.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\yacc.etf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\zaurus.emf";  DestDir: "{app}\macros"
Source: "JASSPA\MicroEmacs\macros\zfile.emf";  DestDir: "{app}\macros"
;
; Help information
;
Source: "JASSPA\MicroEmacs\macros\me.ehf";  DestDir: "{app}\macros"; Components: help
Source: "JASSPA\MicroEmacs\me.chm";  DestDir: "{app}"; Components: helpx
Source: "JASSPA\MicroEmacs\jasspame.pdf";  DestDir: "{app}"; Components: helpx
;
; Spelling dictionary
;
Source: "JASSPA\MicroEmacs\spelling\COPYING"; DestDir: "{app}\spelling"; Components: frfr eses itit ptpt
Source: "JASSPA\MicroEmacs\spelling\README.txt"; DestDir: "{app}\spelling"
Source: "JASSPA\MicroEmacs\spelling\lsdmdede.edf"; DestDir: "{app}\spelling"; Components: dede
Source: "JASSPA\MicroEmacs\spelling\lsdmengb.edf"; DestDir: "{app}\spelling"; Components: engb
Source: "JASSPA\MicroEmacs\spelling\lsdmenus.edf"; DestDir: "{app}\spelling"; Components: enus
Source: "JASSPA\MicroEmacs\spelling\lsdmeses.edf"; DestDir: "{app}\spelling"; Components: eses
Source: "JASSPA\MicroEmacs\spelling\lsdmfifi.edf"; DestDir: "{app}\spelling"; Components: fifi
Source: "JASSPA\MicroEmacs\spelling\lsdmfrfr.edf"; DestDir: "{app}\spelling"; Components: frfr
Source: "JASSPA\MicroEmacs\spelling\lsdmitit.edf"; DestDir: "{app}\spelling"; Components: itit
Source: "JASSPA\MicroEmacs\spelling\lsdmplpl.edf"; DestDir: "{app}\spelling"; Components: plpl
Source: "JASSPA\MicroEmacs\spelling\lsdmptpt.edf"; DestDir: "{app}\spelling"; Components: ptpt
Source: "JASSPA\MicroEmacs\spelling\lsdmruye.edf"; DestDir: "{app}\spelling"; Components: ruye
Source: "JASSPA\MicroEmacs\spelling\lsdmruyo.edf"; DestDir: "{app}\spelling"; Components: ruyo
Source: "JASSPA\MicroEmacs\spelling\lsdxdede.edf"; DestDir: "{app}\spelling"; Components: dede
Source: "JASSPA\MicroEmacs\spelling\lsdxengb.edf"; DestDir: "{app}\spelling"; Components: engb
Source: "JASSPA\MicroEmacs\spelling\lsdxenus.edf"; DestDir: "{app}\spelling"; Components: enus
Source: "JASSPA\MicroEmacs\spelling\lsrdede.emf"; DestDir: "{app}\spelling"; Components: dede
Source: "JASSPA\MicroEmacs\spelling\lsrengb.emf"; DestDir: "{app}\spelling"; Components: engb
Source: "JASSPA\MicroEmacs\spelling\lsrenus.emf"; DestDir: "{app}\spelling"; Components: enus
Source: "JASSPA\MicroEmacs\spelling\lsreses.emf"; DestDir: "{app}\spelling"; Components: eses
Source: "JASSPA\MicroEmacs\spelling\lsrfifi.emf"; DestDir: "{app}\spelling"; Components: fifi
Source: "JASSPA\MicroEmacs\spelling\lsrfrfr.emf"; DestDir: "{app}\spelling"; Components: frfr
Source: "JASSPA\MicroEmacs\spelling\lsritit.emf"; DestDir: "{app}\spelling"; Components: itit
Source: "JASSPA\MicroEmacs\spelling\lsrplpl.emf"; DestDir: "{app}\spelling"; Components: plpl
Source: "JASSPA\MicroEmacs\spelling\lsrptpt.emf"; DestDir: "{app}\spelling"; Components: ptpt
Source: "JASSPA\MicroEmacs\spelling\lsrruye.emf"; DestDir: "{app}\spelling"; Components: ruye
Source: "JASSPA\MicroEmacs\spelling\lsrruyo.emf"; DestDir: "{app}\spelling"; Components: ruyo

[Icons]
Name: "{group}\MicroEmacs"; Filename: "{app}\me32.exe"
Name: "{group}\Getting Started"; Filename: "{app}\jasspame.pdf"; Components: helpx
Name: "{group}\Help"; Filename: "{app}\me.chm"; Components: helpx
Name: "{group}\Uninstall"; Filename: "{uninstallexe}"

[Registry]
; File specific icons in "meicons.exe"
; "me32.ico"      // 069 Original Icon
; "me32.ico"      // 000 Original Icon
; "c.ico"         // 001 C language
; "cpp.ico"       // 002 C++ language
; "h.ico"         // 003 H file
; "def.ico"       // 004 DEF file
; "emc.ico"       // 005 EMC file
; "empty.ico"     // 006 Blank Triangular frame
; "e_grey.ico"    // 007 'e' Grey
; "e_magent.ico"  // 008 'e' Magenta
; "e_blue.ico"    // 009 'e' Blue
; "e_black.ico"   // 010 'e' Black
; "e_cyan.ico"    // 011 'e' Cyan
; "e_green.ico"   // 012 'e' Green
; "e_red.ico"     // 013 'e' Red
; "e_yellow.ico"  // 014 'e' Yellow
; "ehf.ico"       // 015 EHF (Emacs Help file)
; "make.ico"      // 016 Makefile file
; "doc.ico"       // 017 doc file
; "txt.ico"       // 018 txt Text file
; "nroff0.ico"    // 019 Nroff extension 0
; "nroff1.ico"    // 020 Nroff extension 1
; "nroff2.ico"    // 021 Nroff extension 2
; "nroff3.ico"    // 022 Nroff extension 3
; "nroff4.ico"    // 023 Nroff extension 4
; "nroff5.ico"    // 024 Nroff extension 5
; "nroff6.ico"    // 025 Nroff extension 6
; "nroff7.ico"    // 026 Nroff extension 7
; "nroff8.ico"    // 027 Nroff extension 8
; "nroff9.ico"    // 028 Nroff extension 9
; "nroffso.ico"   // 029 Nroff extension .so
; "nrofftni.ico"  // 030 Nroff extension .tni
; "nroffnrs.ico"  // 031 Nroff extension .nrs
; "man.ico"       // 032 man UNIX manual file
; "erblue.ico"    // 033 'e' Rev Blue
; "erblue2.ico"   // 034 'e' Rev Dark Blue
; "erbrown.ico"   // 035 'e' Rev Brown
; "ercyan.ico"    // 036 'e' Rev Cyan
; "ergreen.ico"   // 037 'e' Rev Green
; "ergreen2.ico"  // 038 'e' Rev Dark Green
; "eraqua.ico"    // 039 'e' Rev Aqua Green
; "erblack.ico"   // 040 'e' Rev Black
; "ergrey.ico"    // 041 'e' Rev Frame colour
; "ermagent.ico"  // 042 'e' Rev Magenta
; "erred.ico"     // 043 'e' Rev Red
; "eryellow.ico"  // 044 'e' Rev Yellow
; "abr.ico"       // 045 ABR abreviation file
; "dic.ico"       // 046 DIC dictionary file
; "hash.ico"      // 047 Hash (backup) file
; "twiddle.ico"   // 048 Twiddle (backup) file
; "y.ico"         // 049 'y' (yacc) file
; "l.ico"         // 050 'l' (lex) file
; "p.ico"         // 051 'p' (pascal ??) file
; "etf.ico"       // 052 ETF template file
; "eaf.ico"       // 053 EAF abbreviation file
; "edf.ico"       // 054 EDF dictionary file
; "esf.ico"       // 055 ESF session file
; "emf.ico"       // 056 EMF macro file
; "awk.ico"       // 057 AWK file
; "i.ico"         // 058 i file
; "rc.ico"        // 059 Microsoft RC file
; "rul.ico"       // 060 Install Shield Rule file
; "log.ico"       // 061 Logging file
; "err.ico"       // 062 Error file
; "lbn.ico"       // 063 Nroff Hypertext library file
; "lib.ico"       // 064 Library file
; "htm.ico"       // 065 HTML file
; "htp.ico"       // 066 Hypertext pre-processed file
; "hts.ico"       // 067 Hypertext super file
; "sm.ico"        // 068 Superman file
; "pso.ico"       // 069 Post script ordering file
; "asm.ico"       // 070 Assembler File.
; "erf.ico"       // 071 erf file.
; "jst.ico"       // 072 jst file.
;
; Allow the "MicroEmacs Edit" on the mouse right context menu
;
Root: HKCR; Subkey: "*\shell"; Tasks: extmedit; Flags: uninsdeletekeyifempty
Root: HKCR; Subkey: "*\shell\MicroEmacs Edit"; Tasks: extmedit; Flags: uninsdeletekey
Root: HKCR; Subkey: "*\shell\MicroEmacs Edit\command"; Tasks: extmedit; ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
; MicroEmacs backup files.
;
Root: HKCR; Subkey: ".~"; Tasks: ext/bup;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".~0~"; Tasks: ext/bup;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".~1~"; Tasks: ext/bup;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".~2~"; Tasks: ext/bup;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".~3~"; Tasks: ext/bup;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".~4~"; Tasks: ext/bup;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".~5~"; Tasks: ext/bup;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".~6~"; Tasks: ext/bup;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".~7~"; Tasks: ext/bup;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".~8~"; Tasks: ext/bup;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".~9~"; Tasks: ext/bup;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: deletekey uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.backupfile"; ValueType: string; ValueName: ; ValueData: "JASSPA Backup File"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.backupfile\DefaultIcon"; ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,48"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.backupfile\shell"; ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.backupfile\shell\open"; ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.backupfile\shell\open\command"; ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.autosavefile"; Tasks: ext/bup;  ValueType: string; ValueName: ; ValueData: "JASSPA Autosave File"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.autosavefile\DefaultIcon"; Tasks: ext/bup;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,47"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.autosavefile\shell"; ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.autosavefile\shell\open"; ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.autosavefile\shell\open\command"; ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
; MicroEmacs files
;
Root: HKCR; Subkey: ".eaf"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "jasspa.eaf"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".eaf~"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".edf"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "jasspa.edf"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".edf~"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".ehf"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "jasspa.ehf"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".ehf~"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".eff"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "jasspa.eff"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".eff~"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".emf"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "jasspa.emf"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".emf~"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".enf"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "jasspa.enf"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".enf~"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".erf"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "jasspa.erf"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".erf~"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".esf"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "jasspa.esf"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".esf~"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".etf"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "jasspa.etf"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".etf~"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.eaf"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "JASSPA MicroEmacs Abbreviation File"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.eaf\DefaultIcon"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,53"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.eaf\shell"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.eaf\shell\open"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.eaf\shell\open\command"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.edf"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "JASSPA MicroEmacs Dictionary File"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.edf\DefaultIcon"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,54"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.edf\shell"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.edf\shell\open"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.edf\shell\open\command"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.ehf"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "JASSPA MicroEmacs Help File"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.ehf\DefaultIcon"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,15"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.ehf\shell"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.ehf\shell\open"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.ehf\shell\open\command"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.eff"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "JASSPA MicroEmacs Favourites File"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.eff\DefaultIcon"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,15"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.eff\shell"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.eff\shell\open"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.eff\shell\open\command"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.emf"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "JASSPA MicroEmacs Macro File"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.emf\DefaultIcon"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,56"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.emf\shell"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.emf\shell\open"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.emf\shell\open\command"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.enf"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "JASSPA MicroEmacs Notes File"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.enf\DefaultIcon"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,0"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.enf\shell"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.enf\shell\open"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.enf\shell\open\command"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.erf"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "JASSPA MicroEmacs Registry File"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.erf\DefaultIcon"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,71"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.erf\shell"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.erf\shell\open"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.erf\shell\open\command"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.esf"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "JASSPA MicroEmacs Session File"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.esf\DefaultIcon"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,55"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.esf\shell"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.esf\shell\open"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.esf\shell\open\command"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.etf"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "JASSPA MicroEmacs Template File"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.etf\DefaultIcon"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,52"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.etf\shell"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.etf\shell\open"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.etf\shell\open\command"; Tasks: ext/me;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
; JASSPA Structured Test Files
;
Root: HKCR; Subkey: ".jst"; Tasks: ext/jst;  ValueType: string; ValueName: ; ValueData: "jasspa.jstfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".jst~"; Tasks: ext/jst;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.jstfile"; Tasks: ext/jst;  ValueType: string; ValueName: ; ValueData: "JASSPA Structured Text File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.jstfile\DefaultIcon"; Tasks: ext/jst;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,72"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.jstfile\shell"; Tasks: ext/jst;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.jstfile\shell\open"; Tasks: ext/jst;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.jstfile\shell\open\command"; Tasks: ext/jst;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
; Bind C and C++ files.
;
Root: HKCR; Subkey: ".h"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "jasspa.hfile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".h~"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".hpp"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "jasspa.hppfile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".hpp~"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".hxx"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "jasspa.hxxfile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".hxx~"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".c"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "jasspa.cfile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".c~"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".cpp"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "jasspa.cppfile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".cpp~"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".cc"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "jasspa.cppfile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".cc~"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".cxx"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "jasspa.cxxfile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".cxx~"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".def"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "jasspa.deffile"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: ".def~"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: deletekey uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.cppfile"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "C++ Source File"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.cppfile\DefaultIcon"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,2"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.cppfile\shell"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.cppfile\shell\open"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.cppfile\shell\open\command"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.cxxfile"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "C++ Source File"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.cxxfile\DefaultIcon"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,2"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.cxxfile\shell"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.cxxfile\shell\open"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.cxxfile\shell\open\command"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.cfile"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "C Source File"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.cfile\DefaultIcon"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,1"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.cfile\shell"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.cfile\shell\open"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.cfile\shell\open\command"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.hfile"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "C/C++ Header File"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.hfile\DefaultIcon"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,3"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.hfile\shell"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.hfile\shell\open"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.hfile\shell\open\command"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.hppfile"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "C++ Header File"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.hppfile\DefaultIcon"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,3"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.hppfile\shell"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.hppfile\shell\open"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.hppfile\shell\open\command"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.hxxfile"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "C++ Header File"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.hxxfile\DefaultIcon"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,3"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.hxxfile\shell"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.hxxfile\shell\open"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.hxxfile\shell\open\command"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.deffile"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "C/C++ Definition File"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.deffile\DefaultIcon"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,4"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.deffile\shell"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.deffile\shell\open"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.deffile\shell\open\command"; Tasks: ext/c;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
; Bind Lex and Yacc
;
Root: HKCR; Subkey: ".y"; Tasks: ext/y;  ValueType: string; ValueName: ; ValueData: "jasspa.yfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".y~"; Tasks: ext/y;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".l"; Tasks: ext/y;  ValueType: string; ValueName: ; ValueData: "jasspa.lfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".l~"; Tasks: ext/y;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.yfile"; Tasks: ext/y;  ValueType: string; ValueName: ; ValueData: "YACC File"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.yfile\DefaultIcon"; Tasks: ext/y;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,49"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.yfile\shell"; Tasks: ext/y;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.yfile\shell\open"; Tasks: ext/y;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.yfile\shell\open\command"; Tasks: ext/y;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.lfile"; Tasks: ext/y;  ValueType: string; ValueName: ; ValueData: "LEX File"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.lfile\DefaultIcon"; Tasks: ext/y;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,50"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.lfile\shell"; Tasks: ext/y;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.lfile\shell\open"; Tasks: ext/y;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.lfile\shell\open\command"; Tasks: ext/y;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
; Text files
;
Root: HKCR; Subkey: ".txt"; Tasks: ext/txt;  ValueType: string; ValueName: ; ValueData: "jasspa.txtfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".txt~"; Tasks: ext/txt;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".text"; Tasks: ext/txt;  ValueType: string; ValueName: ; ValueData: "jasspa.textfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".text~"; Tasks: ext/txt;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.txtfile"; Tasks: ext/txt;  ValueType: string; ValueName: ; ValueData: "Text File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.txtfile\DefaultIcon"; Tasks: ext/txt;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,18"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.txtfile\shell"; Tasks: ext/txt;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.txtfile\shell\open"; Tasks: ext/txt;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.txtfile\shell\open\command"; Tasks: ext/txt;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.textfile"; Tasks: ext/txt;  ValueType: string; ValueName: ; ValueData: "Text File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.textfile\DefaultIcon"; Tasks: ext/txt;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,18"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.textfile\shell"; Tasks: ext/txt;  ValueType: string; ValueName: ; ValueData: ""; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.textfile\shell\open"; Tasks: ext/txt;  ValueType: string; ValueName: ; ValueData: ""; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.textfile\shell\open\command"; Tasks: ext/txt;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: deletekey uninsdeletevalue
;
; Log files
;
Root: HKCR; Subkey: ".log"; Tasks: ext/log;  ValueType: string; ValueName: ; ValueData: "jasspa.logfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".log~"; Tasks: ext/log;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.logfile"; Tasks: ext/log;  ValueType: string; ValueName: ; ValueData: "Text File"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.logfile\DefaultIcon"; Tasks: ext/log;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,61"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.logfile\shell"; Tasks: ext/log;  ValueType: string; ValueName: ; ValueData: ""; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.logfile\shell\open"; Tasks: ext/log;  ValueType: string; ValueName: ; ValueData: ""; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.logfile\shell\open\command"; Tasks: ext/log;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: deletekey uninsdeletevalue
;
; LaTeX files
;
Root: HKCR; Subkey: ".tex"; Tasks: ext/tex;  ValueType: string; ValueName: ; ValueData: "jasspa.texfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".tex~"; Tasks: ext/tex;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".bib"; Tasks: ext/tex;  ValueType: string; ValueName: ; ValueData: "jasspa.bibfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".bib~"; Tasks: ext/tex;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.texfile"; Tasks: ext/tex;  ValueType: string; ValueName: ; ValueData: "LaTeX File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.texfile\DefaultIcon"; Tasks: ext/tex;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,18"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.texfile\shell"; Tasks: ext/tex;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.texfile\shell\open"; Tasks: ext/tex;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.texfile\shell\open\command"; Tasks: ext/tex;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.bibfile"; Tasks: ext/tex;  ValueType: string; ValueName: ; ValueData: "LaTeX Bibliography File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.bibfile\DefaultIcon"; Tasks: ext/tex;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,18"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.bibfile\shell"; Tasks: ext/tex;  ValueType: string; ValueName: ; ValueData: ""; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.bibfile\shell\open"; Tasks: ext/tex;  ValueType: string; ValueName: ; ValueData: ""; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.bibfile\shell\open\command"; Tasks: ext/tex;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: deletekey uninsdeletevalue
;
; Manual Page files
;
Root: HKCR; Subkey: ".man"; Tasks: ext/man;  ValueType: string; ValueName: ; ValueData: "jasspa.manfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".man~"; Tasks: ext/man;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.manfile"; Tasks: ext/man;  ValueType: string; ValueName: ; ValueData: "UNIX Manpage File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.manfile\DefaultIcon"; Tasks: ext/man;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,32"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.manfile\shell"; Tasks: ext/man;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.manfile\shell\open"; Tasks: ext/man;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.manfile\shell\open\command"; Tasks: ext/man;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
; Nroff files
;
Root: HKCR; Subkey: ".0"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "jasspa.nroff0file"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".0~"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".1"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "jasspa.nroff1file"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".1~"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".2"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "jasspa.nroff2file"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".2~"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".3"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "jasspa.nroff3file"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".3~"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".4"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "jasspa.nroff4file"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".4~"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".5"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "jasspa.nroff5file"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".5~"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".6"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "jasspa.nroff6file"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".6~"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".7"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "jasspa.nroff7file"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".7~"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".8"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "jasspa.nroff8file"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".8~"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".9"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "jasspa.nroff9file"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".9~"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.nroff0file"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "Troff File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff0file\DefaultIcon"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,19"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff0file\shell"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff0file\shell\open"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff0file\shell\open\command"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.nroff1file"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "Troff File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff1file\DefaultIcon"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,20"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff1file\shell"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff1file\shell\open"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff1file\shell\open\command"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.nroff2file"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "Troff File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff2file\DefaultIcon"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,21"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff2file\shell"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff2file\shell\open"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff2file\shell\open\command"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.nroff3file"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "Troff File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff3file\DefaultIcon"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,22"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff3file\shell"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff3file\shell\open"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff3file\shell\open\command"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.nroff4file"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "Troff File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff4file\DefaultIcon"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,23"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff4file\shell"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff4file\shell\open"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff4file\shell\open\command"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.nroff5file"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "Troff File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff5file\DefaultIcon"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,24"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff5file\shell"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff5file\shell\open"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff5file\shell\open\command"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.nroff6file"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "Troff File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff6file\DefaultIcon"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,24"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff6file\shell"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff6file\shell\open"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff6file\shell\open\command"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.nroff7file"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "Troff File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff7file\DefaultIcon"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,26"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff7file\shell"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff7file\shell\open"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff7file\shell\open\command"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.nroff8file"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "Troff File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff8file\DefaultIcon"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,27"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff8file\shell"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff8file\shell\open"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff8file\shell\open\command"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.nroff9file"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "Troff File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff9file\DefaultIcon"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,28"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff9file\shell"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff9file\shell\open"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nroff9file\shell\open\command"; Tasks: ext/roff;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
; Makefiles
;
Root: HKCR; Subkey: ".mak"; Tasks: ext/mak;  ValueType: string; ValueName: ; ValueData: "jasspa.makefile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".mak~"; Tasks: ext/mak;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".make"; Tasks: ext/mak;  ValueType: string; ValueName: ; ValueData: "jasspa.makefile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".make~"; Tasks: ext/mak;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.makfile"; Tasks: ext/mak;  ValueType: string; ValueName: ; ValueData: "UNIX Manpage File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.makfile\DefaultIcon"; Tasks: ext/mak;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,16"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.makfile\shell"; Tasks: ext/mak;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.makfile\shell\open"; Tasks: ext/mak;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.makfile\shell\open\command"; Tasks: ext/mak;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
; JASSPA Documentation system
;
Root: HKCR; Subkey: ".tni"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: "jasspa.tnifile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".tni~"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".sm"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: "jasspa.smfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".sm~"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".nrs"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: "jasspa.nrsfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".nrs~"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.tnifile"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: "JASSPA Nroff Include File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.tnifile\DefaultIcon"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,30"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.tnifile\shell"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.tnifile\shell\open"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.tnifile\shell\open\command"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.nrsfile"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: "JASSPA Nroff Superfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nrsfile\DefaultIcon"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,31"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nrsfile\shell"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nrsfile\shell\open"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nrsfile\shell\open\command"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.smfile"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: "JASSPA Nroff Super Manual File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.smfile\DefaultIcon"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,68"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.smfile\shell"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.smfile\shell\open"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.smfile\shell\open\command"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
; JASSPA Documentation system
;
Root: HKCR; Subkey: ".tni"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: "jasspa.tnifile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".tni~"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".sm"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: "jasspa.smfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".sm~"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".nrs"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: "jasspa.nrsfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".nrs~"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.tnifile"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: "JASSPA Nroff Include File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.tnifile\DefaultIcon"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,30"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.tnifile\shell"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.tnifile\shell\open"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.tnifile\shell\open\command"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.nrsfile"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: "JASSPA Nroff Superfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nrsfile\DefaultIcon"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,31"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nrsfile\shell"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nrsfile\shell\open"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.nrsfile\shell\open\command"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.smfile"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: "JASSPA Nroff Super Manual File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.smfile\DefaultIcon"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,68"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.smfile\shell"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.smfile\shell\open"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.smfile\shell\open\command"; Tasks: ext/jdoc;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
; Assembler files
;
Root: HKCR; Subkey: ".asm"; Tasks: ext/asm;  ValueType: string; ValueName: ; ValueData: "jasspa.asmfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".asm~"; Tasks: ext/asm;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".s"; Tasks: ext/asm;  ValueType: string; ValueName: ; ValueData: "jasspa.asmfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".s~"; Tasks: ext/asm;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.asmfile"; Tasks: ext/asm;  ValueType: string; ValueName: ; ValueData: "Assembler Source File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.asmfile\DefaultIcon"; Tasks: ext/asm;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,70"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.asmfile\shell"; Tasks: ext/asm;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.asmfile\shell\open"; Tasks: ext/asm;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.asmfile\shell\open\command"; Tasks: ext/asm;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
; Pascal Files
;
Root: HKCR; Subkey: ".pas"; Tasks: ext/pas;  ValueType: string; ValueName: ; ValueData: "jasspa.pasfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".pas~"; Tasks: ext/pas;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".p"; Tasks: ext/pas;  ValueType: string; ValueName: ; ValueData: "jasspa.pasfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".p~"; Tasks: ext/pas;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.pasfile"; Tasks: ext/pas;  ValueType: string; ValueName: ; ValueData: "Pascal Source File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.pasfile\DefaultIcon"; Tasks: ext/pas;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,51"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.pasfile\shell"; Tasks: ext/pas;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.pasfile\shell\open"; Tasks: ext/pas;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.pasfile\shell\open\command"; Tasks: ext/pas;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
; Fortran Files
;
Root: HKCR; Subkey: ".f77"; Tasks: ext/f77;  ValueType: string; ValueName: ; ValueData: "jasspa.f77file"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".f77~"; Tasks: ext/f77;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".f"; Tasks: ext/f77;  ValueType: string; ValueName: ; ValueData: "jasspa.f77file"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".f~"; Tasks: ext/f77;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.f77file"; Tasks: ext/f77;  ValueType: string; ValueName: ; ValueData: "Pascal Source File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.f77file\DefaultIcon"; Tasks: ext/f77;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,0"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.f77file\shell"; Tasks: ext/f77;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.f77file\shell\open"; Tasks: ext/f77;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.f77file\shell\open\command"; Tasks: ext/f77;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
; TCL Files
;
Root: HKCR; Subkey: ".tcl"; Tasks: ext/tcl;  ValueType: string; ValueName: ; ValueData: "jasspa.tclfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".tcl~"; Tasks: ext/tcl;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.tclfile"; Tasks: ext/tcl;  ValueType: string; ValueName: ; ValueData: "TCL Source File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.tclfile\DefaultIcon"; Tasks: ext/tcl;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,7"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.tclfile\shell"; Tasks: ext/tcl;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.tclfile\shell\open"; Tasks: ext/tcl;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.tclfile\shell\open\command"; Tasks: ext/tcl;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
; Shell Files
;
Root: HKCR; Subkey: ".bash"; Tasks: ext/sh;  ValueType: string; ValueName: ; ValueData: "jasspa.shfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".bash~"; Tasks: ext/sh;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".csh"; Tasks: ext/sh;  ValueType: string; ValueName: ; ValueData: "jasspa.shfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".csh~"; Tasks: ext/sh;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".ksh"; Tasks: ext/sh;  ValueType: string; ValueName: ; ValueData: "jasspa.shfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".ksh~"; Tasks: ext/sh;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".sh"; Tasks: ext/sh;  ValueType: string; ValueName: ; ValueData: "jasspa.shfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".sh~"; Tasks: ext/sh;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".zsh"; Tasks: ext/sh;  ValueType: string; ValueName: ; ValueData: "jasspa.shfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".zsh~"; Tasks: ext/sh;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.shfile"; Tasks: ext/sh;  ValueType: string; ValueName: ; ValueData: "Shell Script File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.shfile\DefaultIcon"; Tasks: ext/sh;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,11"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.shfile\shell"; Tasks: ext/sh;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.shfile\shell\open"; Tasks: ext/sh;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.shfile\shell\open\command"; Tasks: ext/sh;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
; Awk Files
;
Root: HKCR; Subkey: ".awk"; Tasks: ext/awk;  ValueType: string; ValueName: ; ValueData: "jasspa.awkfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".awk~"; Tasks: ext/awk;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".nawk"; Tasks: ext/awk;  ValueType: string; ValueName: ; ValueData: "jasspa.awkfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".nawk~"; Tasks: ext/awk;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".gawk"; Tasks: ext/awk;  ValueType: string; ValueName: ; ValueData: "jasspa.awkfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".gawk~"; Tasks: ext/awk;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.awkfile"; Tasks: ext/awk;  ValueType: string; ValueName: ; ValueData: "Shell Script File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.awkfile\DefaultIcon"; Tasks: ext/awk;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,57"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.awkfile\shell"; Tasks: ext/awk;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.awkfile\shell\open"; Tasks: ext/awk;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.awkfile\shell\open\command"; Tasks: ext/awk;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
; PHP Files
;
Root: HKCR; Subkey: ".php"; Tasks: ext/php;  ValueType: string; ValueName: ; ValueData: "jasspa.phpfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".php~"; Tasks: ext/php;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.phpfile"; Tasks: ext/php;  ValueType: string; ValueName: ; ValueData: "PHP File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.phpfile\DefaultIcon"; Tasks: ext/php;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,9"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.phpfile\shell"; Tasks: ext/php;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.phpfile\shell\open"; Tasks: ext/php;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.phpfile\shell\open\command"; Tasks: ext/php;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
; ASP Files
;
Root: HKCR; Subkey: ".asp"; Tasks: ext/asp;  ValueType: string; ValueName: ; ValueData: "jasspa.aspfile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".asp~"; Tasks: ext/asp;  ValueType: string; ValueName: ; ValueData: "jasspa.backupfile"; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.aspfile"; Tasks: ext/asp;  ValueType: string; ValueName: ; ValueData: "ASP File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.aspfile\DefaultIcon"; Tasks: ext/asp;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,12"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.aspfile\shell"; Tasks: ext/asp;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.aspfile\shell\open"; Tasks: ext/asp;  ValueType: string; ValueName: ; ValueData: ""; Flags: uninsdeletevalue
Root: HKCR; Subkey: "jasspa.aspfile\shell\open\command"; Tasks: ext/asp;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: uninsdeletevalue
;
; Error files
;
Root: HKCR; Subkey: ".err"; Tasks: ext/err;  ValueType: string; ValueName: ; ValueData: "jasspa.errfile"; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.errfile"; Tasks: ext/err;  ValueType: string; ValueName: ; ValueData: "Error File"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.errfile\DefaultIcon"; Tasks: ext/err;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,62"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.errfile\shell"; Tasks: ext/err;  ValueType: string; ValueName: ; ValueData: ""; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.errfile\shell\open"; Tasks: ext/err;  ValueType: string; ValueName: ; ValueData: ""; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.errfile\shell\open\command"; Tasks: ext/err;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: deletekey uninsdeletevalue
;
; Out files
;
Root: HKCR; Subkey: ".out"; Tasks: ext/out;  ValueType: string; ValueName: ; ValueData: "jasspa.outfile"; Flags: uninsdeletevalue
;
Root: HKCR; Subkey: "jasspa.outfile"; Tasks: ext/out;  ValueType: string; ValueName: ; ValueData: "Output File"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.outfile\DefaultIcon"; Tasks: ext/out;  ValueType: string; ValueName: ; ValueData: "{app}\meicons.exe,61"; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.outfile\shell"; Tasks: ext/out;  ValueType: string; ValueName: ; ValueData: ""; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.outfile\shell\open"; Tasks: ext/out;  ValueType: string; ValueName: ; ValueData: ""; Flags: deletekey uninsdeletevalue
Root: HKCR; Subkey: "jasspa.outfile\shell\open\command"; Tasks: ext/out;  ValueType: string; ValueName: ; ValueData: """{app}\me32.exe"" -c -o ""%1"""; Flags: deletekey uninsdeletevalue
