/* This is a comment in PL/SQL mode we have to see if the comment will
 * re-format itself correctly. This is a bit of a test */
DECLARE
   x NUMBER := 100;                     /* This is a comment */
BEGIN
   FOR i IN 1..10 LOOP
      IF MOD(i,2) = 0 THEN -- i is even
         INSERT INTO temp VALUES (i, x,  i is even );
      ELSE
         INSERT INTO temp VALUES (i, x,  i is odd );
      END IF;
      x := x + 100;
      END LOOP;
   COMMIT;
END;
IF  THEN
ELSIF  THEN      
ELSIF  THEN
END IF;
