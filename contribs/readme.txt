Contributions to the current JASSPA release

*.reg

    Microsoft  Windows registry files to pre-load the registry with MicroEmacs
    bindings.

    Edit with  specific  path  information  and then run to  install  into the
    registry.

user.emf

    Extensions which may be added to your own <user.emf>. Append to the end of
    your <user.emf> file.

    Your <user>.emf file is located in

    UNIX:       ~/.jasspa/<user>.emf
    WINDOWS:    c:/Documents and Settings/<user name>/Application Data/jasspa/
                <user.emf>

password.emf

    Utilities to handle passwords.

    n insert-password

        Generates  a new  password  of 'n'  characters  in  length.  If 'n' is
        omitted then a 8 character  password is generated. The minimum  length
        of a password is 3. All generated  passwords are guaranteed to have at
        least one each of lowercase,  uppercase  and numeric  characters.  The
        random number generator is always seeded before use.

        The generated password is inserted into the current buffer, the output
        is as follows:-

        > M-x insert-password
        > jvV37sJ9 (juliet - victor - VICTOR - Three - Seven - sierra - JULIET
                    - Nine)

        > 4 M-x insert-password
        > Lf6O (LIMA - foxtrot - Six - OSCAR)

    password-to-phonic

        Converts a password string to a phonic alphabet string, typically used
        to communicate a password to another user  unambiguously.  The user is
        prompted  for the  string to  translate  and the  resultant  string is
        inserted into the current buffer.

        The alphabetic phonic conversion of

            "1EhJ"

        is

            One - ECHO - hotel - JULIET
