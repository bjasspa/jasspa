{****************************************************************************
 *                                                                          *
 * VGA Driver routines                                                      *
 *                                                                          *
 ****************************************************************************
 *
 * Author:          Jon Green
 *
 * Creation Date:   20/07/94
 *
 * File:
 *
 * Description:
 *
 * Access the DOS BIOS to change the colour palette of the VGA driver.
 * This set of routines access the VGA registers exchanging the colour
 * palette values. The information to access the registers was obtained
 * from the following reference:-
 *
 * DOS Programmer's Reference 3rd Edition
 * QUE Coorporation 1992
 * ISBN: 0-880022-790-7
 *
 * Note that the reference only describes the calling convention to the BIOS.
 * After playing with the calls. It appears that there are 64 colour registers
 * [0..63] which contain 6 bit RGB values. A maximum of 16 of these triples
 * are accessed through the palette registers giving the standard
 * 16 colours that we are all familier with.
 *
 ***************************************************************************}

unit    vga;

interface

uses    dos;

const
    VGA_COLOUR_GROUP = 15;          { 16 items in a colour group }

type
    VGATexel = record
        red : word;
        green : word;
        blue : word;
    end;

    VGAColour = array [0..VGA_COLOUR_GROUP] Of VGATexel;
    VGAPalette = array [0..VGA_COLOUR_GROUP] Of word;

    VGActl = record
        offset : integer;           { Start offset of VGA palette }
        colourReg    : VGAColour;
        extColourReg : VGAColour;
        paletteReg   : VGAPalette;
    end;

    VGActlp = ^ VGActl;

procedure   VGAColourGet (var colour : VGAColour; offset : integer);
procedure   VGAColourSave (var ctl : VGActl; colour : VGAColour; offset : integer);
procedure   VGAColourRestore (var ctl : VGActl);
procedure   VGAColourResume (var ctl : VGActl);
procedure   VGAColourSwap (var ctl : VGActl);

implementation

procedure SaveColourRegisters (var store : VGAColour; offset : VGAPalette);

var
    i : integer;
    regs : Registers;

begin

    for i:= 0 to VGA_COLOUR_GROUP do
    begin
        { Save the colour Registers }
        regs.AH := $10;
        regs.AL := $15;
        regs.BX := offset[i];
        Intr ($10, regs);
        store[i].red := regs.DH;
        store[i].blue := regs.CL;
        store[i].green := regs.CH;
    end;
end;

procedure RestoreColourRegisters (store : VGAColour; offset : VGAPalette);

var
    i : integer;
    regs : registers;

begin
    for i:= 0 to VGA_COLOUR_GROUP do
    begin
        { Restore the colour Registers }
        regs.AH := $10;
        regs.AL := $10;
        regs.BX := offset [i];
        regs.DH := store[i].red;
        regs.CH := store[i].green;
        regs.CL := store[i].blue;
        Intr ($10, regs);
    end;
end;


procedure SavePaletteRegisters (var store : VGAPalette);

var
    i : integer;
    regs : Registers;

begin

    for i:= 0 to VGA_COLOUR_GROUP do
    begin
        { Save the palette Registers }
        regs.AH := $10;
        regs.AL := $07;
        regs.BL := i;
        Intr ($10, regs);
        store[i] := regs.BH;
    end;
end;

procedure RestorePaletteRegisters (var store : VGAPalette);

var
    i : integer;
    regs : Registers;

begin

    for i:= 0 To VGA_COLOUR_GROUP Do
    begin
        { Restore the palette Registers }
        regs.AH := $10;
        regs.AL := $00;
        regs.BL := i;
        regs.BH := store[i];
        Intr ($10, regs);
    end;
end;

procedure VGAColourGet (var colour : VGAColour; offset : integer);
var
    palt : VGAPalette;
begin
    SavePaletteRegisters (palt);
    SaveColourRegisters (colour, palt);
end;


procedure VGAColourSave (var ctl : VGActl; colour : VGAColour; offset : integer);

var
    i : integer;
    palt : VGAPalette;

begin

    SavePaletteRegisters (ctl.paletteReg);
    SaveColourRegisters (ctl.extColourReg,ctl.paletteReg);
    RestoreColourRegisters (colour,ctl.paletteReg);
end;


procedure VGAColourRestore (var ctl : VGActl);
begin

    RestoreColourRegisters (ctl.extColourReg, ctl.paletteReg);

end;

procedure VGAColourResume (var ctl : VGActl);

var
    i : integer;
    palt : VGAPalette;

begin

    for i := 0 to VGA_COLOUR_GROUP do
        palt [i] := ctl.offset+i;
    RestoreColourRegisters (ctl.colourReg,ctl.paletteReg);
    RestorePaletteRegisters (palt);

end;

procedure VGAColourSwap (var ctl : VGActl);
var
    i : integer;
    palt : VGAPalette;

begin

    for i := 0 to VGA_COLOUR_GROUP do
        palt [i] := ctl.offset+i;

    RestorePaletteRegisters (palt);

end;

begin
end.

