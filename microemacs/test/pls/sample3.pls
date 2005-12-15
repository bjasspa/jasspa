-- available online in file  sample3
DECLARE
    x NUMBER := 0;
    counter NUMBER := 0;
BEGIN
    FOR i IN 1..4 LOOP
        x := x + 1000;
        counter := counter + 1;
        INSERT INTO temp VALUES (x, counter,  'in OUTER loop' );
        /* start an inner block */
        DECLARE x
            NUMBER := 0; -- this is a local version of x
        BEGIN
            FOR i IN 1..4 LOOP
                x := x + 1;  -- this increments the local x
                counter := counter + 1;
                INSERT INTO temp VALUES (x, counter,  'inner loop');
            END LOOP;
        END;
    END LOOP;
    COMMIT;
END;
