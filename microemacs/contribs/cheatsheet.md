## <img src="https://raw.githubusercontent.com/bjasspa/jasspa/main/microemacs/graphics/me_m.png" /> MicroEmacs Cheatsheet - 2025-10-14 06:49

Homepage: [https://github.com/bjasspa/jasspa](https://github.com/bjasspa/jasspa)
Help pages: [https://bjasspa.github.io/jasspa/](https://bjasspa.github.io/jasspa/)

Small, low footprint  text editor with highly  sophisticated  editing  facilities for
editing source code of many  programming and markup  languages. Same interface
in terminal and in graphical mode on all major platforms.


__Installation (MacOS / Linux):__

`/bin/sh -c "$(curl -fsSL https://github.com/bjasspa/jasspa/releases/latest/download/microemacs-install)"`

__Installation (Windows / Intel):__

Download and Install: [Jasspa_MicroEmacs_20250901_installer_windows_intel.msi](https://github.com/bjasspa/jasspa/releases/download/me_20250901/Jasspa_MicroEmacs_20250901_installer_windows_intel.msi)

Or via Powershell: 

`Invoke-RestMethod -Uri https://github.com/bjasspa/jasspa/releases/latest/download/microemacs-install.ps1 | Invoke-Expression`

__User Setup:__

After  starting  `mec` (terminal) or `mew` (GUI version) you will be asked
for your name and the basic user  settings.  Thereafter  run the  `user-setup`
command  either via __`Esc x user-setup  ENTER`__ or via "Menu (__`Esc =`__) - Tools -
User Setup" where you should setup your __keyboard layout__ at least, may be select the more
traditional  __Rebind  Home Keys__ and __MS Shift  Region__  options and then
optionally  as well a __spell  checking  language__  in the first  setup  tab.
Beginners  as well might  un-select  the  __Edit__  options  beside the
__Setup File__ name so that the users configuration file is not autoloaded after pressing OK.

__MicroEmacs command line arguments (mec terminal or mew graphical application):__

<div class="noth">

|                   |                      |                    |                                      |
|:------------------|----------------------|--------------------|------------------------------|
| mec/mew FILENAME  | open given file      | mec/mew -c         | continue default session     |
| mec/mew -h        | display help         | mec/mew <br/>-cSESSION  | continue named SESSION       |
| mec/mew -V        | show version         | mec/mew -u USERNAME| start with other username |

</div>

__MicroEmacs  Editor most  important  menu commands (use 'Esc =' to access the
menu without a mouse ):__

<div class="compact">

| File               | Edit                     | Search                  |         View         |
|:-------------------|:-------------------------|:------------------------|:---------------------|
| Open: `C-x C-f`    | Undo: `C-x u`            | Incr Search: `C-s`      | Buffer Info: `C-x =` |
| Favorites          | Set Mark: `C-space`      | Search Dialog           | Occur: `S-f6`        |
| Close: `C-x k`     | Copy Region: `Esc w`     | Find Next: `C-x h`      | Buffer: `C-x b`      |
| Save: `C-x C-s`    | Cut/Kill Region: `C-w`   | Replace: `Esc r`        | Toolbar: `S-f11`     |
| Save As: `C-x C-w` | Paste/Yank Region: `C-y` | Set Bookmark: `C-x C-a` | Whitespaces: `f11`   |
| File Browser: `F10`| Cut Rectangle: `Esc C-w` | Goto Bookmark: `C-x a`  |                      |
| Load Session       | Paste Rectangle `Esc C-y`| Goto Line: `Esc g`      |                      |
| Exit: `C-x C-c`    | Insert File: `C-x C-i`   |                         |                      |

</div>

__Hint:__  To  repeat  a  command N times press  `Esc N` and press the command
shortcut or  enter  the  command name into the message line,  for
example, you can do the following:

- `Esc 10 Down` - go 10 lines down by pressing  Esc, then  writing 10 into the
   message line and then pressing the Down key
- `Esc 15 C-x u` - undo the last 15 editing steps
- `Esc 10 Esc x redo  ENTER` - redo the last 10  editing  steps
- `C-a Esc x 10 C-k` - delete the next ten lines

__Text selection:__  `Ctrl-Space` to start then cursor keys (or `C-p` or `C-n`) , `Esc w` to copy and `C-y` to yank/paste.

<div style="page-break-after: always"> </div>

## &nbsp;

__Further menu points with most important commands:__

<div class="compact">

| Format              | Tools              | Advanced           | Window            | Help                   |
|:--------------------|:-------------------|:-------------------|:------------------|:-----------------------|
| Incr Indent         | Spell Word: `esc $`| Rec Mac:  `C-x (`  | Split WV:`C-x 2`  | General H: `Esc ?`     |
| Decr Indent         | Spell Buffer: `f7` | End Mac:   `C-x )` | Split WH: `C-x 3` | List Bnd:  `C-h b`     |
| Fill Para: `Esc q`  | Diff               | Exec Mac: `C-x e`  | One W:    `C-x 1` | List Buffs: `C-x C-b`  |
| Tabs to Spaces      | Graphical Diff     | Exec Cmd: `Esc x`  | Delete W: `C-x 0` | Descr Key: `C-h k`     |
| Clean Buffer        | User Setup         | Notes:       `f8`  | Prev W: `C-x p`   | Help  Cmd: `C-h C-c`   |
|                     | Buffer Setup       | File Tools         | Next W:  `C-x o`  | About ME               |
|                     | Scheme Editor      | Ipipe: `Esc \`     |                   |                        |
</div>

__Other important commands / shortcuts:__

| Important                     | Navigation                 | Deletion                          |
|-------------------------------|:---------------------------|:----------------------------------|
| 655360 osd (__menu__): `Esc =`| beginning-of-line: `C-a`   | forward-delete-char: `C-d`        |
| execute-named-command:        | end-of-line: `C-e`         | backward-delete-char: `backspace` |
| `Esc x`                       | forward-char: `C-f`        | forward-kill-word: `Esc-d`        |
| abort-command: `C-g`          | forward-word: `Esc f`      | backward-kill-word: `Esc backsp`  |
| list-commands: `C-h c`        | backward-char: `C-b`       | kill-line (rest): `C-k`           |
| help-command:  `C-h C-c`       |  backward-word:  `Esc b`  | kill-region:`C-w`                 |
| describe-bindings: `C-h b`    | occur (list): `S-f6`       | delete-buffer: `C-x k`            |
| suspend-emacs (mec): `C-c z`  | item-list (code outline)   | delete-window (current): `C-x 0`  |
| help: `Esc x help`            | abbrev-list / toolbar      | delete-other-windows `C-x 1`      |  

__MicroEmacs Files:__

<div class="noth">

| &nbsp;                        | &nbsp;                    | &nsbp;                        |
|:------------------------------|:--------------------------|:------------------------------|
| Abbreviation Files: `*.eaf`   | Help File: `me.ehf`       | Registry File: `USERNAME.erf` |
| Dictionary Files: `*.edf`     | Macro Files: `*.emf`      | Session Files: `*.esf`        |
| Favorite File: `USERNAME.eff` | User File: `USERNAME.emf` | Template Files: `*.etf`       |

</div>

__USERNAME.emf__

The  user can  configure  specific  key  bindings,  abbreviation  files,  file
extension  mappings and short self written  macros  within its  `USERNAME.emf`
file, which is usually based on the username on the current machine, but could
be switched as well giving options like  `MEUSERNAME=test` on the command line
or using the `-u  USERNAME`  command  line option. Here an example for an user
called  `kiosk-user`  which adds the file  extension  `.Tmd` to be edited  using
Markdown mode, declares a global  abbreviation  file  kiosk-user.eaf  and then
binds the key combination `C-x t` to the `insert-template` macro command.

```emf
; file ~/.config/jasspa/kiosk-user.emf
global-abbrev-file "kiosk-user"         ; file is kiosk-user.eaf
add-file-hook ".tmd .Tmd" fhook-md      ; tmd files are Markdown files
add-file-hook ".rnw .snw" fhook-latex   ; snw and rnw files are LaTeX files
add-file-hook ".re2c" fhook-c           ; re2c files are C files
global-bind-key insert-template "C-x t" ; new key-binding for insert-template
```

<div style="page-break-after: always"> </div>

## &nbsp;

## Snippets / Abbreviation Files

Snippets are text  fragments  which can be inserted by typing an  abbreviation
into the text after pressing usually the `Esc Esc` key combination. There are two type
of abbreviations, global abbreviations which are available in all documents or
scripts and buffer specific abbreviations which are the same for all documents
of the same type, for instance for all Python files. These  abbreviations  are
defined in files having the extension `.eaf`.

To define a __global abbreviation__  file you have to add the  following  line to
your USERNAME.emf setup file (USERNAME should be replaced with your ME username).

```emf
global-abbrev-file "USERNAME"
```

and  then  add a file  `USERNAME.eaf`  to your ME  configuration  folder  with
content like this:

```emf
DG "Detlef Groth, University of Potsdam, Germany
UP "University of Potsdam
br "best regards,\rDetlef Groth"
JML "[Jasspa MicroEmacs Text Editor](https://github.com/bjasspa/jasspa)"
```

Hint: You can reload the abbreviation file within the current editor session
after saving it by using the following keys:

`Esc 0 Esc x global-abbreviation-file<ENTER>`

MicroEmacs comes for many programming and markup languages already with buffer
specific  abbreviation  files. To see what abbreviations are available you can
use for  instance  the  `abbrev-list`  command,  which  allows  you in the GUI
version graphically select the abbreviations with the mouse. To configure your
own abbreviation file for a specific  programming language you should create a
file  `myHOOK.emf`,  where `HOOK` is your file type like 'python', 'r' or 'md'
(Markdown)      in     your      config      folder     so     for     example
`~/.config/jasspa/mypython.eaf`   and  in  this  you  then   place   your  own
abbreviations.  Hint: You can as well copy the default  `python.eaf` file from
the macros directory to your personal config folder and modify that file.

Here an example  for a Python  abbreviation  file which is  declared in a file
_mypython.emf_ and implemented in a file _python.eaf_ (\p sets a mark \P jumps to that mark):

```emf
; file ~/.config/USERNAME/mypython.emf
buffer-abbreviation-file "python"
; file ~/.config/USERNAME/python.eaf
sb "#!/usr/bin/env python3"
enc "# -*- coding: UTF-8 -*-\r" 
if "if \p:\r    \r\P"
elif "elif \p:\r    \r\P"
else "else:\r    \p\r\P"
def "def \p ():\r    '''def docu'''\r"
```

## Templates

Templates  are files which are usually used to insert  larger  chunks of text,
for  example,  to start a new  application  using a template  for the  current
programming language you are coding. User defined  templates have the file
extension  ".etf"  and user  defined  ones  should  be  placed  either in the  folder
_~/config/jasspa/default_  for generic  templates  like License  files or user
addresses which are not defined as abbreviations, 
or in file type specific folders named _~/.config/jasspa/HOOK/_
where HOOK is the file hook for which they  should be used. For  example  like
_~/.config/jasspa/python_, _.../r_ (R file templates) or _.../md_ (Markdown file
templates).

The following  place holders which are used for  replacement  of text in these
templates or for  inserting the cursor are defined:  _\$CURSOR\$_,  _\$FILE_NAME\$_,
_\$USER\$_, _\$YEAR\$_ - for more see `C-h C-c insert-template`.

## Macro Recording

MicroEmacs  can be  extended  using  self  written  or  recorded  macros.  For
beginners  it might be easier to record key  sequences  and replay them later.
The steps to do this are as follows:

- press  the keys  `C-x (`
- do your  editing  steps using only your keyboard
- stop recording with `C-x )`
- execute recorded macro with `C-x e`
- execute a macro N times by `Esc N C-x e`
- save macro using the `save-kbd-macro` command

## Macro Programming

<table width="100%">
<tr>
<td width='40%'>

__Variables:__

- local (within macros): `#l0 #l1 ... #l9` 
- namespace: `.local .macroname.local`
- global: `#g0 #g1 ... #g9` - %global

__Control-Flow:__

- `!if cond` - `!elif cond` - `!else`
- `!while cond` - `!done` 
- `!repeat` - `!until cond`
- `!continue` - `!break`

__Data structures:__

- scalar - `set-variable #l0 1`
- list   - `set-variable #l0 "|item1|item2|item3|"`

__Operators:__

- logical: `&and`, `&not`, `&or`
- math: `&abs`, `&equ`, 

__Functions:__

__Macros:__

- Definition: `define-macro` - `!emacro`
- Arguments: @1 @2 
- Numeric arguments: @? (bool) and @# (number)

</td>
<td>
Example:

```emf
set-variable %global "Max Musterman"
define-macro hwllo-world-test
    set-variable #l0 "World"
    1000 ml-write &spr "Hello %s for %s!" #l0 %global
    set-variable #l1 0
    !while &gre #l1 10
        set-variable #l1 &add #l1 1
        1000 ml-write &spr "#l0 is now '%i'!" #l1
    !done
    1000 ml-write "Done!"
!emacro
```
</td>
</tr>
</table>
<div style="page-break-after: always"> </div>


## &nbsp;

### Installation of TrueType Fonts for X11 for mew/mesw

For the GUI version (mew/mesw) additional TrueType Font installations on X systems (MacOS with
[XQuartz](https://www.xquartz.org/)  or Linux  with [X11](https://www.x.org) or  XWayland)  improve
the visual  display and Windows code pages or the ISO-8859  encodings  usage.
Here an example how you can index  existing  TrueType fonts which are already
on your system  installed by the package  manager.  Recommended  fonts are for
example,  liberation-mono and dejavu-sans-mono as they come with a lot of font
encodings and they are quite visually appealing.

```bash
### check installable mono fonts
### Fedora
### sudo dnf search mono-fonts | less
### sudo dnf install gnu-free-mono-fonts cascadia-mono-fonts dejavu-sans-mono-fonts
### sudo dnf install liberation-mono-fonts adobe-source-code-pro-fonts
### sudo apt install fonts-firacode font-cascadia-code fonts-ubuntu-console
### sudo apt install fonts-anonymous-pro
if [ ! -d ~/.local/share/fonts ]; then
    mkdir -p ~/.local/share/fonts
fi
find  /usr/ -iregex \
  ".*\\(Mono\\|Mono-?Bold\\|Mono-?Regular\\|Code-Regular\\|Code-Bold\\).ttf" 2>/dev/null \
  | xargs ln -sf -t ~/.local/share/fonts
mkfontscale ~/.local/share/fonts/
mkfontdir ~/.local/share/fonts
xset +fp ~/.local/share/fonts
xset fp rehash
alias mfontsel="LC_ALL=C xfontsel -pattern '*-r-*-m-*' -scaled"
```

You can check the installed  fonts then with the tool `xfontsel`.  Default for
Western  Europe usually should be ISO-8859-15 or  Windows-CP1252  which covers
for instance  German  umlauts and the Euro symbol or the standards  ISO-8859-5
(Cyrillic),  ISO-8859-7 (Greek), ISO-8859-9 (Turkish), ISO-8859-10 (Nordic) or
ISO-8859-13  (Baltic). The alias defined above  _mfontscale_ gives easy access
to monospaced fonts. See as well the
[Wikipedias entry for ISO-encodings](https://de.wikipedia.org/wiki/ISO_8859).

Hint: To convert between different encodings using the command: `change-buffer-charset`

To install other fonts like Ubuntu Mono you can do the following:

```bash
cd ~/.local/share/fonts
BURL="https://github.com/braver/programmingfonts/raw/gh-pages/fonts/resources/ubuntu/"
wget ${BURL}/ubuntu-bold.ttf -O ubuntu-mono-bold.ttf
wget ${BURL}/ubuntu.ttf    -O ubuntu-mono-medium.ttf
wget ${BURL}/license.txt -O ubuntu-license.txt
mkfontscale .
mkfontdir .
xset fp rehash
```

To check the fontpath you can as well add this to your .bashrc configuration:

```bash
### end of .bashrc
alias mfontsel="xfontsel -pattern '*-r-*-m-*' -scaled"
if [ "$DISPLAY" != "" ]; then
   if [ "`xset q | grep .local/share/fonts`" == "" ]; then
     xset +fp ~/.local/share/fonts
     set fp rehash
   fi
fi
```

For   more   information  and for  adding   other   fonts:
[https://github.com/mittelmark/x11-ttf-fonts](https://github.com/mittelmark/x11-ttf-fonts)
