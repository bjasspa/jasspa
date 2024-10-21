
## <img src="microemacs/graphics/me_m.png" /> &nbsp;Jasspa MicroEmacs Text Editor

MicroEmacs  text editor is an Emacs like text editor  suitable for 
experienced programmers and as well beginners.

In contrast to other editors it offers a set of unique features:

- small size 1-2MB on your harddisk
- single file binaries - easy to install 
- low memory footprint - 1-2 Mb in principle
- terminal and GUI version working the same way with full menu support
- hypertext help system within your editor
- alternative keybindings

As other editors it has obviously:

- session management (pro: parallel sessions for every project - 
  due to low memory footprint are no problem)
- extensible by a macro language (as well a macro recorder)
- syntax highlighting for many languages - even nested highlighting (R, Python etc in Markdown documents )
- spell checking for many languages
- and much more ...

Cons (because it is a __Micro__-Emacs):

- no Unicode  support - but all the ISO and  extended  Windows  encodings  are
  usable even on UTF-8 terminals
- no  soft  (visual)  wrap - what  you  see is what  you  get (you can let ME
  autowrap for you the text during writing)

<img src="microemacs/graphics/ME24.png" width="390px"/>&nbsp;&nbsp;<img src="microemacs/graphics/ME24-terminal-greek.png" width="390px"/>

_Left:_ GUI  version  - theme "Basic  Black on Cream",  _Right:_  Greek Text in the
Terminal  version  within a Tmux  session with an open menu entry - theme:  "Default  White on Black".
The menu entry can be opened on a terminal version usually with "Esc =".
There  are  more  than  25  themes  (schemes)  to  choose  from  ... The  main
appplication  frame is split here into two  windows, one is for editing a file, the
other here has the hyperhelp documentation open. You can as well edit the same
file within several separate  windows, for  instance writing  something on top,
and then in the  other  windows  more down in the same file ... but  obviously
editing one file in a single window is as well possible.

<img src="microemacs/graphics/ME24-pydoc-solarized-light.png" width="390px"/>&nbsp;&nbsp;<img src="microemacs/graphics/ME24-r-doc-ayu-dark.png" width="390px"/>

_Left:_ GUI version - theme "Solarized Light", displaying on top a help page defined using Markdown (bottom window) for the [pydoc](contribs/hkpydoc.emf) macro,  
_Right:_ GUI version - theme "Ayu Dark", displaying the hypertext enabled R documentation browser defined with the r-doc command.

## Installation

You can compile the code yourself, or you pick one or two of the precompiled
ones from the Release page.

MicroEmacs single file executables comes in two flavors:

- mecs - terminal version
- mews - GUI version (require X11 on Linux and MacOS)

Download one of the following zip archives with the binaries  inside and place
the executables into a folder belonging to your PATH

| Platform      | Terminal        | GUI (X11 on Linux/Mac) |
|:-------------:|:---------------:|:----------------------:|
| Linux         | [mecs](https://github.com/bjasspa/jasspa/releases/download/me_20241001/Jasspa_MicroEmacs_20241001_abin_linux_mecs.zip)       | [mews](https://github.com/bjasspa/jasspa/releases/download/me_20241001/Jasspa_MicroEmacs_20241001_abin_linux_mews.zip) |
| MacOS apple   | [mecs](https://github.com/bjasspa/jasspa/releases/download/me_20241001/Jasspa_MicroEmacs_20241001_abin_macos_apple_mecs.zip) | [mews](https://github.com/bjasspa/jasspa/releases/download/me_20241001/Jasspa_MicroEmacs_20241001_abin_macos_apple_mews.zip) |
| MacOS intel   | [mecs](https://github.com/bjasspa/jasspa/releases/download/me_20241001/Jasspa_MicroEmacs_20241001_abin_macos_intel_mecs.zip) | [mews](https://github.com/bjasspa/jasspa/releases/download/me_20241001/Jasspa_MicroEmacs_20241001_abin_macos_intel_mews.zip)
| Windows       | [mecs](https://github.com/bjasspa/jasspa/releases/download/me_20241001/Jasspa_MicroEmacs_20241001_abin_windows_mecs.zip)     | [mews](https://github.com/bjasspa/jasspa/releases/download/me_20241001/Jasspa_MicroEmacs_20241001_abin_windows_mews.zip)

To test the  integrity  of the  downloads  you can use the
[sha256 hash keys](https://github.com/bjasspa/jasspa/releases/download/me_20241001/Jasspa_MicroEmacs_20241001_abin.sha256).

Then test the executable in your terminal:

```
mecs -V
```

This should printout the version and exit MicroEmacs.

## User Setup

After  starting  your  first  real  session  you are  proposed  to do call the
init-session function if you like to save your settings on the current system.
You can do this by calling the  `init-session`  function. Press `Esc` and then
the `x` key, you see that you are in the command  line at the bottom can write
some text, write `init-session` press ENTER.

Then you should select your keyboard layout. Either you use the menubar on top
to find that  functionality "Esc =" should work in the terminal and in the GUI
application,  the  latter  can use as well  mouse or "Alt-t"  key and then `u`
combination, or you enter the command `Esc x user-setup`. In the Menu you have
to access the "(T)ools - (U)ser Setup" entry.

In User setup select in the "Start-Up" tab your keyboard layout and in GUI mode in
the  "Platform"  tab your font. X11 users  here will  greatly  benefit if they
installed  the  xfontsel  tool as it allows to  visually  select the font on a
MacOS or Linux system.

If you are  ready  use  "File-Save  All" to save  your  settings.  The next ME
session should start with these saved settings.


## Tools to improve your editing on Linux and MacOS

- X11 (macOS, Linux):
    - xfontsel - to interactively select fonts from within your ME session
    - mkfontscale - if you like to add your own TTF fonts as available fonts

MicroEmacs versions  before  20241001  might require the tools luit and abduco for better
support of international character encodings on UTF-8 enabled terminals.

## License

GPL - see the file [COPYING.txt](microemacs/COPYING.txt)

## Authors

- Dave Conroy         1985-1986
- Daniel M. Lawrence  1986-1988  
- John Green          1990-2010
- Steven Phillips     1990-2024
- Detlef Groth        2021-2024

