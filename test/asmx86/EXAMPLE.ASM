comment $

   +----------------------------------------------------------------------+
   |                                                                      |
   |   AsmWiz  (C) Copyright 1996 Charon Software, All Rights Reserved    |
   |                                                                      |
   |                          AsmWiz Demo Program                         |
   |                                                                      |
   |                        assembled with MASM 6.0                       |
   |                                                                      |
   +----------------------------------------------------------------------+

   The code here has been designed to be easily understandable and is not as
   efficient as the coding used in the actual library.
$


; the use of a dummy stack segment here eliminates the meaningless error
; message from LINK about there being no stack defined.

Sseg          segment byte stack 'prog'     ; dummy stack segment
Sseg          ends

Cseg          segment byte public 'prog'

              assume cs:Cseg, ds:Cseg, ss:Sseg

              org            100h



extrn  BKO_GETKEY:near, MD_DONE:near, MD_DELAY:near, MD_INIT:near
extrn  MV_INIT:near, MV_LOCATE:near, MV_MODE:near, MV_GETMODE:near
extrn  MV_POPUP:near, MV_STROUT:near, MV_HIDECURSOR:near
extrn  MV_SHOWCURSOR:near, MV_STROUT:near, MV_INSLINE:near, MV_DELLINE:near
extrn  MV_INSCHR:near, MV_DELCHR:near, MV_COLOR:near, MV_CLS:near
extrn  MV_FIXCOLOR:near, MV_FRAME:near, MI_GETSCREEN:near, MI_PARSE:near
extrn  S0_LENGTH:near, S0_UPCASES:near



MAIN          proc           far       ; AsmWiz example program
              call           Initialize     ; initialize screen and colors
              call           ShowWelcome    ; display welcome message
              call           SlideWelcome   ; slide welcome message left
              call           ShowWindow     ; display pop-up window w/ text
              call           ShowCopyright  ; display copyright message
              call           WaitOnKey      ; display "press any" and wait
              call           Terminate      ; restore original screen mode, etc
              mov            ax,4C00h       ;
              int            21h            ; exit program
MAIN          endp                     ; AsmWiz example program



Initialize    proc           near      ; initialize the screen
              call           MV_GETMODE     ; get current screen mode
              mov            OldMode,al     ; save it
              call           MV_INIT        ; initialize display routines
              mov            al,3           ; mode 3: 80x25 color text
              call           MV_MODE        ; set display mode
              call           MV_HIDECURSOR  ; turn off the cursor
              mov            si,0080h       ; pointer to the command line
              lea            di,FileBuf     ; pointer to filename buffer
              lea            bx,OptBuf      ; pointer to option buffer
              mov            al,"/"         ; use normal DOS switch character
              call           MI_PARSE       ; parse the command line
              or             ah,ah          ; is there an option?
              jz             CheckCRT       ;   no, go check the CRT type
              mov            si,bx          ; pointer to first option
              mov            di,bx          ;
              call           S0_UPCASES     ; convert it to uppercase
              cmp byte ptr   [bx],"B"       ; is it /B for monochrome mode?
              jne            CheckCRT       ;   no, check CRT type
              mov            al,1           ; set to mono
              jmp            SetCRT         ;   go set type
CheckCRT:     call           MI_GETSCREEN   ; see what the screen type is
SetCRT:       call           MV_FIXCOLOR    ; set the color handler to suit
              mov            al,1Fh         ; bright white on blue
              call           MV_COLOR       ; set text color
              call           MV_CLS         ; clear screen to new color
              call           MD_INIT        ; initialize hi-res timer system
              ret                           ;
Initialize    endp                     ; initialize the screen



ShowWelcome   proc           near      ; display welcome message
              mov            dx,0125h       ; row 1, column 37
              call           MV_LOCATE      ; set cursor location
              lea            dx,WelcomeMsg  ; ptr to "Welcome"
              call           MV_STROUT      ; display it
              mov            cx,8           ;
Welcome1:     call           MV_INSLINE     ; scroll it down
              push           cx             ;
              mov            cx,4           ;
              call           MD_DELAY       ; delay 4/100ths of a second
              pop            cx             ;
              loop           Welcome1       ; ...nine times
              mov            dx,0B25h       ; row 11, column 37
              call           MV_LOCATE      ; set cursor location
              lea            dx,ToTheMsg    ; ptr to "to the"
              call           MV_STROUT      ; display it
              mov            dx,0D01h       ; row 13, column 1
              call           MV_LOCATE      ; set cursor location
              lea            dx,AsmMsg      ; ptr to "Asm" <cr>
              call           MV_STROUT      ; display it
              mov            cx,37          ;
Welcome2:     call           MV_INSCHR      ; scroll it right
              push           cx             ;
              mov            cx,2           ;
              call           MD_DELAY       ; delay 2/100ths of a second
              pop            cx             ;
              loop           Welcome2       ; ...37 times
              mov            dx,0D4Dh       ; row 13, column 73
              call           MV_LOCATE      ;
              lea            dx,WizMsg      ; ptr to "Wiz"
              call           MV_STROUT      ; display it
              mov            dx,0D29h       ; row 13, column 41
              call           MV_LOCATE      ; set cursor location
              mov            cx,36          ;
Welcome3:     call           MV_DELCHR      ; scroll it left
              push           cx             ;
              mov            cx,2           ;
              call           MD_DELAY       ; delay 2/100ths of a second
              pop            cx             ;
              loop           Welcome3       ; ...36 times
              mov            dx,1925h       ; row 25, column 37
              call           MV_LOCATE      ; set cursor location
              lea            dx,LibraryMsg  ; ptr to "Library"
              call           MV_STROUT      ; display it
              mov            dx,0E01h       ; row 14, column 1
              call           MV_LOCATE      ; set cursor location
              mov            cx,10          ;
Welcome4:     call           MV_DELLINE     ; scroll it up
              push           cx             ;
              mov            cx,4           ;
              call           MD_DELAY       ; delay 4/100ths of a second
              pop            cx             ;
              loop           Welcome4       ; ...10 times
              mov            cx,200         ; delay 200/100ths of a second
              call           MD_DELAY       ;
              ret                           ;
ShowWelcome   endp                     ; display welcome message



SlideWelcome  proc           near      ; slide welcome message left
              mov            dx,0901h       ; row 9, column 1
              mov            ax,4           ;
ScrollAll:    mov            cx,20          ; columns to scroll
LineLeft:     call           MV_LOCATE      ; set cursor position
              call           MV_DELCHR      ; scroll it left
              loop           LineLeft       ;   go for all columns
              add            dh,2           ; move to next line
              dec            ax             ; done yet?
              jnz            ScrollAll      ;   no, go for next row
              mov            al,15h         ; magenta on white
              call           MV_COLOR       ; set text color
              mov            cx,070Eh       ; upper left row, col
              mov            dx,111Ah       ; lower right row, col
              mov            si,-9          ; solid block frame
              mov            ax,4           ; frame loop count
WildFrame:    call           MV_FRAME       ; display frame
              sub            cx,0101h       ; move upper left corner out one
              add            dx,0101h       ; move lower right corner out one
              inc            si             ; set to next lighter frame type
              dec            ax             ; are we still drawing frames?
              jnz            WildFrame      ;   yep, go draw that funky thang
              ret                           ;
SlideWelcome  endp                     ; slide welcome message left



ShowCopyright proc           near      ; display copyright message
              mov            dx,010Fh       ; row 1, column 15
              call           MV_LOCATE      ; set cursor position
              mov            al,1Bh         ; cyan on blue
              call           MV_COLOR       ; set text color
              lea            dx,CopyrMsg    ; copyright text
              call           MV_STROUT      ; display it
              ret                           ;
ShowCopyright endp                     ; display copyright message



ShowWindow    proc           near      ; display pop-up window w/ text
              lea            dx,ScrWindow   ;
              call           MV_POPUP       ; pop up intro window
              lea            si,WindowText  ; pointer to window text
              mov            dx,word ptr ScrWindow
              xchg           dl,dh          ;
              add            dx,0102h       ; starting text position
ShowText:     call           MV_LOCATE      ; set cursor position
              call           S0_LENGTH      ; determine length of text
              jcxz           ShoWindowXit   ;   if zero, we're done-- go exit
              mov            al,[si]        ; get text color
              call           MV_COLOR       ; set it
              inc            si             ; skip over color to actual text
              xchg           dx,si          ;
              call           MV_STROUT      ; display the text
              xchg           dx,si          ;
              add            si,cx          ; move to next text line
              inc            dh             ; move to next screen line
              jmp            ShowText       ;   go for all text
ShoWindowXit: ret                           ;
ShowWindow    endp                     ; display pop-up window w/ text



WaitOnKey     proc           near      ; display "press any key", wait for it
              mov            dx,191Bh       ; row 25, column 26
              call           MV_LOCATE      ; set cursor location
              mov            al,74h         ; red on white
              call           MV_COLOR       ;
              lea            dx,PressAnyMsg ; ptr to "Press any key..."
              call           MV_STROUT      ; display it
              mov            al,07h         ; normal video
              call           MV_COLOR       ;
              mov            al,1           ; wait for key
              call           BKO_GETKEY     ; get a key
              ret                           ;
WaitOnKey     endp                     ; display "press any key", wait for it



Terminate     proc           near      ; restore original screen mode, etc
              call           MD_DONE        ; shut down hi-res timer system
              mov            al,OldMode     ; get old screen mode
              call           MV_MODE        ; set video mode
              call           MV_SHOWCURSOR  ; restore the cursor
              ret                           ;
Terminate     endp                     ; restore original screen mode, etc



WelcomeMsg    db "Welcome",0
ToTheMsg      db "to the",0
AsmMsg        db "Asm",13,0
WizMsg        db "Wiz",0
LibraryMsg    db "Library",0
CopyrMsg      db "AsmWiz  Copyright 1990-1994  Thomas G. Hanlin III",0
PressAnyMsg   db "Press any key to continue",0

ScrWindow     db 3,40,23,79        ; window coordinates
              db 1,4Eh             ; frame type and color
              dw offset ScrTitle   ; pointer to title
ScrTitle      db "The Assembly Wizard's Library",0   ; window title

WindowText    db 4Ah,"Let's face it, asm programming can  ",0
              db 4Ah,"be slow.  By the time you finish all",0
              db 4Ah,"the low-level stuff, you forget what",0
              db 4Ah,"you wanted to do in the first place!",0
              db 4Ah,"AsmWiz takes care of all the little ",0
              db 4Ah,"tedious details for you, so you can ",0
              db 4Ah,"concentrate on the real work.       ",0
              db 4Ah,0
              db 70h,"Extensive text and graphics support ",0
              db 30h,"Base conversions and 32-bit math    ",0
              db 70h,"Delay and countdown services        ",0
              db 30h,"Random numbers, checksums, CRCs     ",0
              db 70h,"Parse input or the command line     ",0
              db 30h,"Scan DOS environment variables      ",0
              db 70h,"Intercept Break and Control-C       ",0
              db 30h,"String functions, sound effects     ",0
              db 70h,"Mouse and keyboard management       ",0
              db 30h,"File handling with buffers, sharing ",0
              db 70h,"International time and date support ",0,0

FileBuf   db 80 dup(?)
OptBuf    db 80 dup(?)

OldMode   db 3

Cseg          ends
              end            MAIN

