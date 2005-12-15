IMPLEMENTATION MODULE Terminal2;

(* Implementation of Terminal2 for Stony Brook
   Frank Schoonjans - January 2003                  *)

IMPORT Terminal;
IMPORT WholeStr,STextIO,IOChan,StdChans,IOConsts,SWholeIO;
IMPORT RealStr,Strings,ConvTypes;

PROCEDURE ReadChar(VAR v : CHAR);
BEGIN
    REPEAT
       v:=Terminal.ReadChar()
    UNTIL v#00C;
    WriteChar(v);
    REPEAT UNTIL Terminal.ReadChar()=CHR(13);
    WriteLn;
END ReadChar;

PROCEDURE ReadString(VAR v: ARRAY OF CHAR);
VAR i  : CARDINAL;
    ch : CHAR;
BEGIN
    i:=0;
    LOOP
      REPEAT
         ch:=Terminal.ReadChar()
      UNTIL ch#00C;
      IF (ch=EOL) OR (ch=CHR(13)) OR (ch=CHR(10)) THEN ch:=EOL; EXIT END;
      IF ch>=" " THEN
         WriteChar(ch);
         v[i]:=ch; INC(i);
         IF i>HIGH(v) THEN EXIT END;
      END;
    END;
    IF i<=HIGH(v) THEN v[i]:=00C; END;
    WriteLn;
END ReadString;

PROCEDURE ReadCard(VAR v: CARDINAL);
VAR str : ARRAY [0..80] OF CHAR;
    res : RealStr.ConvResults;
BEGIN
    ReadString(str);
    WholeStr.StrToCard(str,v,res);
    Done := res=ConvTypes.strAllRight;
    IF ~Done THEN v:=0 END;
END ReadCard;

PROCEDURE ReadInt(VAR v: INTEGER);
VAR str : ARRAY [0..80] OF CHAR;
    res : RealStr.ConvResults;
BEGIN
    ReadString(str);
    WholeStr.StrToInt(str,v,res);
    Done := res=ConvTypes.strAllRight;
    IF ~Done THEN v:=0 END;
END ReadInt;

PROCEDURE ReadReal(VAR r: REAL);
VAR str : ARRAY [0..80] OF CHAR;
    res : RealStr.ConvResults;
BEGIN
    ReadString(str);
    RealStr.StrToReal(str,r,res);
    Done := res=ConvTypes.strAllRight;
    IF ~Done THEN r:=0.0 END;
END ReadReal;

PROCEDURE WriteChar(v: CHAR);
BEGIN
    Terminal.Write(v);
END WriteChar;

PROCEDURE WriteLn;
BEGIN
    Terminal.WriteLn;
END WriteLn;

PROCEDURE WriteString(v: ARRAY OF CHAR);
BEGIN
    Terminal.WriteString(v);
END WriteString;

PROCEDURE WriteInt(v: INTEGER;  w: CARDINAL);
VAR str : ARRAY [0..80] OF CHAR;
BEGIN
    WholeStr.IntToStr(v,str);
    WHILE Strings.Length(str)<w DO
      Strings.Insert(" ",0,str);
    END;
    WriteString(str);
END WriteInt;

PROCEDURE WriteCardDigits(v: CARDINAL; w: CARDINAL; oh : CARDINAL);
CONST MAXCHARS=80;
CONST OHstr = "0123456789ABCDEF";
VAR str : ARRAY [0..MAXCHARS] OF CHAR;
    i,j : CARDINAL;
BEGIN
    Done:=TRUE;
    i:=0;
    REPEAT
      str[i]:=OHstr[v MOD oh];
      v:=v DIV oh;
      INC(i);
    UNTIL (v=0) OR (i>MAXCHARS);
    IF (v#0) AND (i>MAXCHARS) THEN
      Done:=FALSE;
      RETURN
    END;
    IF (w>0) AND (i<w) THEN
      FOR j:=1 TO w-i DO
        WriteChar(" ");
      END;
    END;
    FOR j:=i-1 TO 0 BY -1 DO
      WriteChar(str[j])
    END;
END WriteCardDigits;

PROCEDURE WriteCard(v: CARDINAL; w: CARDINAL);
BEGIN
    WriteCardDigits(v,w,10);
END WriteCard;

PROCEDURE WriteOct(v: CARDINAL; w: CARDINAL);
BEGIN
    WriteCardDigits(v,w,8);
END WriteOct;

PROCEDURE WriteHex(v: CARDINAL; w: CARDINAL);
BEGIN
    WriteCardDigits(v,w,16);
END WriteHex;

PROCEDURE WriteReal(r: REAL; w: CARDINAL);
VAR str : ARRAY [0..15] OF CHAR;
    len : CARDINAL;
BEGIN
    RealStr.RealToStr(r,str);
    len:=Strings.Length(str);
    WHILE (len>0) AND (str[len-1]="0") DO
      DEC(len)
    END;
    str[len-1]:=00C;
    WHILE len<w DO
      Strings.Concat(" ",str,str);
      INC(len);
    END;
    WriteString(str);
    Done:=TRUE;
END WriteReal;

PROCEDURE Wait;
BEGIN
    REPEAT UNTIL Terminal.ReadChar()#00C;
END Wait;

BEGIN
    Terminal.Reset;
    Done:=TRUE;
FINALLY
    Wait;
END Terminal2.
