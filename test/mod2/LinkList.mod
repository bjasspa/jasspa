MODULE LinkList;

FROM Terminal2  IMPORT WriteString, WriteChar, WriteLn;
FROM Storage    IMPORT ALLOCATE, DEALLOCATE;
FROM SYSTEM     IMPORT TSIZE;

TYPE  NextPointer = POINTER TO FullName;
      FullName    = RECORD
                      FirstName : ARRAY[0..12] OF CHAR;
                      Initial   : CHAR;
                      LastName  : ARRAY[0..15] OF CHAR;
                      Next      : NextPointer;
                    END;

VAR   StartOfList : NextPointer;
      PlaceInList : NextPointer;
      TempPlace   : NextPointer;
      Index       : CARDINAL;

BEGIN   (* Main Program *)

                  (* Generate the first name in the list *)

   ALLOCATE(PlaceInList,TSIZE(FullName));
   StartOfList := PlaceInList;
   PlaceInList^.FirstName := "John ";
   PlaceInList^.Initial   := 'Q';
   PlaceInList^.LastName  := " Doe";
   PlaceInList^.Next      := NIL;

                  (* Generate another name in the list *)

   TempPlace := PlaceInList;
   ALLOCATE(PlaceInList,TSIZE(FullName));
   TempPlace^.Next := PlaceInList;
   PlaceInList^.FirstName := "Mary ";
   PlaceInList^.Initial   := 'R';
   PlaceInList^.LastName  := " Johnson";
   PlaceInList^.Next      := NIL;

                 (* Add 10 more names to complete the list *)

   FOR Index := 1 TO 10 DO
      TempPlace := PlaceInList;
      ALLOCATE(PlaceInList,TSIZE(FullName));
      TempPlace^.Next := PlaceInList;
      PlaceInList^.FirstName := "Billy ";
      PlaceInList^.Initial   := 'R';
      PlaceInList^.LastName  := " Franklin";
      PlaceInList^.Next := NIL;
   END;

                        (* Display the list on the monitor *)

   PlaceInList := StartOfList;
   WHILE PlaceInList#NIL DO
      WriteString(PlaceInList^.FirstName);
      WriteChar(PlaceInList^.Initial);
      WriteString(PlaceInList^.LastName);
      WriteLn;
      TempPlace   := PlaceInList;
      PlaceInList := PlaceInList^.Next;
   END;

                             (* Deallocate is a good habit *)

   WHILE StartOfList#NIL DO
      TempPlace := StartOfList^.Next;
      DEALLOCATE(StartOfList,TSIZE(FullName));
      StartOfList:=TempPlace;
   END;

END LinkList.
