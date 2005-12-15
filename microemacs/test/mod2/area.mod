MODULE Areas;

(* Coronado Enterprises - 1987 *)

FROM Terminal2 IMPORT WriteString, WriteChar, WriteLn, ReadChar,
                      WriteReal, ReadReal;

VAR InChar, CapInChar : CHAR;

(* ************************************************ Area Of Square *)
PROCEDURE AreaOfSquare;
VAR Length, Area : REAL;
BEGIN
   WriteString("Square    Enter length of a side : ");
   ReadReal(Length);
   Area := Length * Length;
   WriteLn;
   WriteString("The area is ");
   WriteReal(Area,15);
   WriteLn;
END AreaOfSquare;

(* ********************************************* Area Of Rectangle *)
PROCEDURE AreaOfRectangle;
VAR Width, Height, Area : REAL;
BEGIN
   WriteString("Rectangle    Enter Width : ");
   ReadReal(Width);
   WriteLn;
   WriteString("Enter Height ");
   ReadReal(Height);
   Area := Width * Height;
   WriteString("      The area is ");
   WriteReal(Area,15);
   WriteLn;
END AreaOfRectangle;

(* ********************************************** Area Of Triangle *)
PROCEDURE AreaOfTriangle;
VAR Base, Height, Area : REAL;
BEGIN
   WriteString("Triangle    Enter base : ");
   ReadReal(Base);
   WriteLn;
   WriteString("Enter height : ");
   ReadReal(Height);
   Area := 0.5 * Base * Height;
   WriteString("      The area is ");
   WriteReal(Area,15);
   WriteLn;
END AreaOfTriangle;

(* ************************************************ Area Of Circle *)
PROCEDURE AreaOfCircle;
VAR Radius, Area : REAL;
BEGIN
   WriteString("Circle      Enter Radius : ");
   ReadReal(Radius);
   WriteLn;
   Area := 3.141592 * Radius * Radius;
   WriteString("The area is ");
   WriteReal(Area,15);
   WriteLn;
END AreaOfCircle;

(* ************************************************** Main Program *)
BEGIN
   REPEAT
      WriteLn;
      WriteString("Input the first letter of the selection");
      WriteString("and press Enter.");
      WriteLn;
      WriteString("Select shape: Square Rectangle Triangle ");
      WriteString("Circle Quit");
      WriteLn;
      WriteString("Requested shape is ");
      ReadChar(InChar);
      CapInChar := CAP(InChar);           (* Get capital of letter *)
      CASE CapInChar OF
        'S' : AreaOfSquare;    |
        'R' : AreaOfRectangle; |
        'T' : AreaOfTriangle;  |
        'C' : AreaOfCircle;    |
        'Q' : WriteString("Quit program");
              WriteLn;
      ELSE
         WriteChar(InChar);
         WriteString(" Invalid Character ");
         WriteLn;
      END;
   UNTIL CapInChar = 'Q';
END Areas.

