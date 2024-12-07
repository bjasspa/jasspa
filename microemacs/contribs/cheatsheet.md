## <img src="/home/dgroth/workspace/jasspa/microemacs/graphics/me_m.png" /> MicroEmacs Cheatsheet - 2024-12-07 14:33

Homepage: [https://github.com/bjasspa/jasspa](https://github.com/bjasspa/jasspa)  
Help pages: [https://bjasspa.github.io/jasspa/](https://bjasspa.github.io/jasspa/)

Small, low footprint  text editor with highly  sophisticated  editing  facilities for
editing source code of many  programming and markup  languages. Same interface
in terminal and in graphical mode on all major platforms.


__Installation (MacOS / Linux):__

`/bin/sh -c "$(curl -fsSL https://github.com/bjasspa/jasspa/releases/latest/download/microemacs-install)"`

__Installation (Windows):__

Download and Install: [Jasspa_MicroEmacs_20241101_installer_windows.msi](https://github.com/bjasspa/jasspa/releases/download/me_20241101/Jasspa_MicroEmacs_20241101_installer_windows.msi)

__User Setup:__

After  starting  `mec` the terminal or `mew` the GUI version you will be asked
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
| mec/mew FILENAME  | open the given file  | mec/mew -c         | continue default session     |
| mec/mew -h        | display help         | mec/mew <br >-cSESSION  | continue named SESSION       |
| mec/mew -V        | show version         | mec/mew -u USERNAME| start ME with other username |

</div>

__MicroEmacs Editor most important menu commands (Esc =):__

<div class="compact">

| File               | Edit                     | Search                 |         View        |
|:-------------------|:-------------------------|:-----------------------|:--------------------|
| Open: `C-x C-f`    | Undo: `C-x u`            | Incr Search: `C-s`      | Buffer Info: `C-x =` |
| Favorites          | Set Mark: `C-space`      | OSD Search             | Occur: `S-f6`        |
| Close: `C-x k`     | Copy Region: `Esc w`     | Find Next: `C-x h`      | Buffer: `C-x b`      |
| Save: `C-x C-s`    | Cut/Kill Region: `C-w`   | Replace: `Esc r`        | Whitespaces: `F11`   |
| Save As: `C-x C-w` | Paste/Yank Region: `C-y` | Set Bookmark: `C-x C-a` |                     |
| File Browser: `F10`| Cut Rectangle: `Esc C-w` | Goto Bookmark: `C-x a`  |                     |
| Load Session:      | Paste Retangle `Esc C-y` | Goto Line: `Esc g`      |                     |
| Exit; `C-x C-c`    | Insert File: `C-x C-i`   |                        |                     | 
</div>

__Hint:__ To  repeat  a  command N times press  `Esc N` and then  enter  the  command,  for
example, do the following:

- `Esc 15 C-x u` - undo the last 15 editing steps
- `Esc 10 Down` - go 10 lines down
- `Esc 10 Esc x redo  ENTER` - redo the last 10  editing  steps

__Text selection:__  `Ctrl-Space` to start then cursor keys, `Esc w` to copy and `C-y` to yank/paste.

<div style="page-break-after: always"> </div>

## &nbsp;

__Further menu points with most important commands:__

<div class="compact">

| Format              | Tools              | Advanced           | Window            | Help                   |
|:-----------------------|:-------------------|:----------------|:------------------|:-----------------------|
| Incr Indent         | Spell Word: `esc $`| Rec Mac:  `C-x (`  | Split WV:`C-x 2`  | General H: `Esc ?`     |
| Decr Indent         | Spell Buffer: `f7` | End Mac:   `C-x )` | Split WH: `C-x 3` | List Bnd:  `C-h b`     |
| Fill Para: `Esc q`  | Diff               | Exec Mac: `C-x e`  | One W:    `C-x 1` | List Buffs: `C-x C-b`  |
| Tabs to Spaces      | Graphical Diff     | Exec Cmd: `Esc x`  | Delete W: `C-x 0` | Descr Key: `C-h k`     |
|                     | User Setup         | Notes:       `f8`  | Prev W: `C-x p`   | Help  Cmd: `C-h C-c`   |
|                     | Buffer Setup       | File Tools         | Next W:  `C-x o`  | About ME               |
|                     | Scheme Editor      | Ipipe: `Esc \`     |                   |                        |
</div>

__Other important commands / shortcuts:__

| Important                     | Navigation                 | Deletion                          |
|-------------------------------|:---------------------------|:----------------------------------|
| 655360 osd (__menu__): `Esc =`| beginning-of-line: `C-a`   | forward-delete-char: `C-d`        |
| execute-named-command: `Esc x`| end-of-line: `C-e`         | backward-delete-char: `backspace` |
| abort-command: `C-g`          | forward-char: `C-f`        | kill-line (rest): `C-k`           |                              
| list-commands: `C-h c`        | forward-word: `Esc f`      | delete-buffer: `C-x k`            |      
| help-command: `C-h C-c`       | backward-char: `C-b`       | kill-region: `C-w`                |    
| describe-bindings: `C-h b`    | backward-word: `Esc b`     | delete-window (current): `C-x 0`  |
| suspend-emacs (mec): `C-c z`  | occur (list): `S-f6`       | delete-other-windows `C-x 1`      |
| help: `Esc x help`            | item-list (outline)        | 


__MicroEmacs Files:__

<div class="noth">

| &nbsp;                       | &nbsp;                       | &nsbp;                 |
|:-----------------------------|------------------------------|:-----------------------|
| Abbreviation Files `*.eaf`   | Help File `me.ehf`           | Session Files `*.esf`  |
| Dictionary Files `*.edf`     | Macro Files `*.emf`          | Template Files `*.etf` |
| Favorite File `username.eff` | Registry File `username.erf` |                        |
</div>

__USERNAME.emf__

The user can  configure  specific key bindings,  abbreviation  files, and file
extension  mappings within its `USERNAME.emf`  file, which is usually based on
the  username  on the  current  machine,  but could be switched as well giving
options  like  `MEUSERNAME=test`  on the command  line. Here an example for an
user called `kiosk-user` which adds the file extension .Tmd to be edited using
Markdown mode, declares a global  abbreviation  file  kiosk-user.eaf  and then
binds the key combination `C-x t` to the insert-template command.

```
; file ~/.config/jasspa/kiosk-user.emf
global-abbrev-file "kiosk-user"         ; file is kiosk-user.eaf
add-file-hook ".tmd .Tmd" fhook-r-md    ; tmd files are Markdown files
add-file-hook ".rnw .snw" fhook-latex   ; snw and rnw files are LaTeX files
add-file-hook ".re2c" fhook-c           ; re2c files are C files
global-bind-key insert-template "C-x t" ; new keybinding for insert-template
```

And here is an example for an abbreviation file (kiosk-user.eaf), after adding the line:

```
MM "Max Musterman, University of Potsdam, Germany"
```

Hint: You can reload the abbreviation file within the current editor session
after saving it by using the following keys: 

`Esc 0 Esc x global-abbreviation-fileENTERkiosk-userENTER`


__Macros and Macro Recording:__ 

MicroEmacs  can be  extended  using  self  written  or  recorded  macros.  For
beginners  it might be easier to record key  sequences  and replay them later.
The steps to do this are as follows:

Press  the keys  `C-x (` and then do your  editing  steps,  you then  stop the
recording  with `C-x )`. Thereafter you can re-execute the last recorded macro
with `C-x e`. You can as well save the executed macro with a name then with the `save-kbd-macro` command.

<div style="page-break-after: always"> </div>

## &nbsp;

## Snippets

Snippets are text  fragments  which can be inserted by typing an  abbreviation
into the text after pressing the `Esc Esc` key combination. There are two type
of abbreviations, global abbreviations which are available in all documents or
scripts and buffer specific abbreviations which are the same for all documents
of the same type, for instance for all Python files. These  abbreviations  are
defined in files having the extension `.eaf`.

To define a __global abbreviation__  file you have to add the  following  line to
your USERNAME.emf setup file (USERNAME should be replaced with your ME username).

```
global-abbrev-file "USERNAME"
```

and  then  add a file  `USERNAME.eaf`  to your ME  configuration  folder  with
content like this:

```
DG "Detlef Groth, University of Potsdam, Germany
UP "Universty of Potsdam
br "best regards,\rDetlef Groth"
```

MicroEmacs comes for many programming and markup languages already with buffer
specific abbreviation files.
To see what abbreviations are 

## Templates

Templates  are files which are usually used to insert  larger  chunks of text,
for instance to start a new application. User defined  templates have the filew
extension "*.etf" and should be placed in the folder  `~/.config/jasspa/HOOK/`
where HOOK is the filehook for which the should be used like  "python", "r" or
"md" (Markdown).




<div style="page-break-after: always"> </div>

## &nbsp;

### Installation of True Type Fonts for X11

For the GUI version additional TrueType Font installations on X systems (MacOS
- [XQuartz](, and Linux - X11) improve  visual  display and ISO  encodings  usage.
Here an example how you can index  existing  True Type fonts which are already
on your system  installed by the package  manager.  Recommended  fonts are for
example,  liberation-mono and dejavu-sans-mono as they come with a lot of font
encodings are quite visually appealing.
 
```
### check installable mono fonts 
### Fedora
### sudo dnf search mono-fonts | less
### sudo dnf install gnu-free-mono-fonts cascadia-mono-fonts dejavu-sans-mono-fonts
### sudo dnf install liberation-mono-fonts adobe-source-code-pro-fonts
### sudo apt install fonts-firacode font-cascadia-code fonts-ubuntu-console
### sudo apt install fonts-anonymous-pro
[[ ! -d ~/.local/share/fonts/ttf ]] && mkdir -p ~/.local/share/fonts/ttf
find  /usr/ -iregex \
  ".*\\(Mono\\|Mono-?Bold\\|Mono-?Regular\\|Code-Regular\\|Code-Bold\\).ttf" 2>/dev/null \
  | xargs ln -sf -t ~/.local/share/fonts/ttf
mkfontscale ~/.local/share/fonts/ttf/
mkfontdir ~/.local/share/fonts/ttf
xset +fp ~/.local/share/fonts/ttf/
xset fp rehash
alias mfontscale="LC_ALL=C xfontsel -pattern '*-r-*-m-*' -print -scaled"
mfontscale
```

You can check the  installed  fonts then with the tool  `xfontsel`.  Default  for Western
Europe usually should be ISO-8859-15  which covers for instance German umlauts
and the Euro symbol. The alias  _mfontscale_  gives easy access to  monospaced
fonts.

To install other fonts like Ubuntu Mono you can do the following:

```
cd ~/.local/share/fonts/ttf
BURL="https://github.com/braver/programmingfonts/raw/gh-pages/fonts/resources/ubuntu/"
wget "${BURL}/ubuntu-bold.ttf -O ubuntu-mono-bold.ttf
wget "${BURL}/ubuntu.ttf    -O ubuntu-mono-medium.ttf    
wget "{BURL}/license.txt -O ubuntu-license.txt
mkfontscale .
mkfontdir .
xset fp rehash

```

