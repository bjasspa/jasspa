
## MicroEmacs (2024) TextEditor

MicroEmacs  text editor is an Emacs like text editor  suitable for experienced
programmers and as well beginners.

In contrast to other editors it offers a set of unique features:

- small size 1-2MB on your harddisk
- single file binaries - easy to install 
- low memory footprint - 1-2 Mb in principle
- terminal and GUI version with full menu support

As other editors it has:

- session management
- extensible by macro language
- syntax hilighting - even nested highlighting (R, Python etc in Markdown documents )
- spell checking
- much more ...

Cons:

- no Unicode support - but all the ISO and extended Windows encodings are usable
- no soft (visual) wrap - what you see is what you get

## Installation

You can compile the code yourself, or you pick one or two of the precompiled
ones from the Release page.

MicroEmacs single file executables comes in two flavors:

- mecs - terminal version
- mews - GUI version (require X11 on Linux and MacOS)

Download one of the following zip archives with the binaries  inside and place
the executables into a folder belonging to your PATH

| Platform      | mecs (terminal) | GUI (X11 on Linux/Mac) |
|:-------------:|:---------------:|:----------------------:|
| Linux         | [x](releases/download/me_20240901/Jasspa_MicroEmacs_20240901_abin_linux_mecs.zip)       | [+](releases/download/me_20240901/Jasspa_MicroEmacs_20240901_abin_linux_mews.zip) |
| MacOS apple   | [x](releases/download/me_20240901/Jasspa_MicroEmacs_20240901_abin_macos_apple_mecs.zip) | [x](releases/download/me_20240901/Jasspa_MicroEmacs_20240901_abin_macos_apple_mews.zip) |
| MacOS intel   | [x](releases/download/me_20240901/Jasspa_MicroEmacs_20240901_abin_macos_intel_mecs.zip) | [x](releases/download/me_20240901/Jasspa_MicroEmacs_20240901_abin_macos_intel_mews.zip)
| Windows       | [x](releases/download/me_20240901/Jasspa_MicroEmacs_20240901_abin_windows_mecs.zip)     | [x](releases/download/me_20240901/Jasspa_MicroEmacs_20240901_abin_windows_mews.zip)

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
to access the "__T__ools-__U__ser Setup" entry.

In User setup select in the "Start-Up" your keyboard layout and in GUI mode in
the  "Platform"  tab your font. X11 users  here will  greatly  benefit if they
installed  the  xfontsel  tool as it allows to  visually  select the font on a
MacOS or Linux system.

If you are  ready  use  "File-Save  All" to save  your  settings.  The next ME
session should start with these saved settings.

## License

GPL - see the file [COPYING](COPYING)




