#include    <conio.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <dos.h>

#define LINE_LEN    79
#define PAGE_LEN    24

#define CHAR_NORM   0
#define CHAR_BOLD   1
#define CHAR_UND    2

FILE    *fp;
int     lineNo;
union REGS rg;
int     compaq = 0;

void Lmoveto(void)
{
    rg.h.ah = 2;
    rg.h.dl = 0;
    rg.h.dh = 24;
    rg.h.bh = 0;
    int86 (0x10, &rg, &rg);
}

void noBlink (void)
{
    rg.x.ax = 0x1303;
    int86 (0x10, &rg, &rg);
}

void Lceol (void)
{
    int i;
#if 1
#if 0
    Lmoveto();
    for (i = 0; i < (LINE_LEN-1); i++)
        printf (" ");
    fflush (stdout);
    Lmoveto();
#else
    char disp [] = "\\|/-";
    for (i = 0; i < 10000; i++) {
        Lmoveto();
        printf ("%c", disp [i&3]);
    }
#endif

#else

    printf ("%c%c%c", 8, ' ', 8);
    fflush (stdout);
#endif
}

int readLine (char **cbuf)
{
    static char buffer [1024];
    int     c;
    int     len;

    *cbuf = buffer;
    len = 0;
    while ((c = fgetc (fp)) != EOF)
    {
        if (c == '\n') {
            buffer [len] = '\0';
            return (len);
        }
        else if (c == '\r')
            continue;
        buffer [len++] = c;
    }   /* End of 'while' */
    if (len == 0)
        return (-1);
    return (len);

}   /* End of 'readLine' */

void displayChar (char c, int mode, int i)
{
/*    int attrib [4] = {8,7,0x1e,0x4f};*/
/*     int attrib [4] = {7,14,0x1e,0x4f}; GOOD */
/*    int attrib [4] =  */ /* BETTER COMPAQ */
    static int *attrib = NULL;
    int attribCpq [4] = {7,0x70,0x4f,0x4f};
    int attribNorm [4] = {7, 0xf, 0x4e, 0x4f};
    int req_attrib;

    if (attrib == NULL)
        attrib = (compaq == 1) ? attribCpq : attribNorm;

    req_attrib = attrib [mode];

    /* c = (int)('0') + mode;*/

    rg.h.ah = 9;
    rg.h.al = c;
    rg.h.bl = req_attrib;
    rg.h.bh = 0;
    rg.x.cx = 1;
    int86(0x10,&rg,&rg);

    rg.h.ah = 14;
    rg.h.al = c;
    rg.h.bl = req_attrib;
    rg.h.bh = 0;
    int86(0x10,&rg,&rg);

}

void displayLine (char *s)
{

typedef struct {
    char    c;
    int     n;
}   tchar;

    int     i;
    char    c;
    int     mode = 0;
    tchar   cf, cl;
    int     index;
    int     column;

    column = 0;
    i = 0;
    while ((c = *s++) != 0) {
        mode = 0;
        if (++i > LINE_LEN)
            break;
        /*printf ("%c", c);*/
        cf.c = c;
        cf.n = 1;
        cl.c = 0;
        cl.n = 0;
        index = 0;
        for (;;) {
            if (s[index] != 8)
                break;
            mode |= 0x80;
            index++;
            if ((c = s[index]) != 0) {
                index++;
                if (cl.n == 0) {
                    cl.c = c;
                    cl.n = 1;
                }
                else if (cl.c == c)
                    cl.n++;
                else if (cf.c == c)
                    cf.n++;
                else
                    break;
            }   /* End of 'if' */
        }   /* End of 'for' */
        if (mode != 0) {
            if (cl.n != 0)
                c = cl.c;
            else
                c = cf.c;
            if ((cf.c == '_' || cl.c == '_') && cf.c != cl.c) {
                if (cl.c == '_')
                    c = cf.c;
                else
                    c = cl.c;
                mode |= CHAR_UND;
            }
            else if ((cl.n + cf.n) > 2)
                mode |= CHAR_BOLD;
            else
                mode |= CHAR_UND;
            if (cf.n > 1 || cl.n > 1) {
                c = cl.c;
                mode |= CHAR_BOLD;
            }
            s = &s[index];
            mode &= 3;
        }

        if (c == '\t') {
            i = (column + 8) & ~7;
            while (i > column) {
                displayChar (' ', 0, column);
                column++;
            }
        }
        else {
            displayChar (c, mode, column);
            column++;
        }
    }   /* End of 'while' */
    printf ("\n");
    fflush (stdout);
}

int prompt (void)
{
    int c;
    int next;

    Lmoveto ();
    printf (":");
    fflush (stdout);
    next = -1;
    while (next < 0) {
        c = getch ();
        switch (c) {
            case ' ':
                next = 1;
                break;
            case 'q' :
                next = 0;
                break;
            case 13 :
                next = PAGE_LEN;
                break;
            default:
                break;
        }
    }
    Lmoveto ();
    printf (" ");
    fflush (stdout);
    Lmoveto ();
    return (next);
}

int pg (void)
{
    char    *inLine;
    int     onscreen = 0;
    int     endOfFile = 0;
    int     whatNext;

    Lmoveto ();
    for (;;) {
        while (onscreen <  PAGE_LEN) {
            if (readLine (&inLine) != -1) {
                lineNo++;
                displayLine (inLine);
                onscreen++;
            }
            else
            {
                endOfFile = 1;
                break;
            }
        }
        if ((whatNext = prompt()) != 0)
            onscreen -= whatNext;
        else
            return (1);
    }
    return (1);
}

int main (int argc, char *argv [])
{
    char *fname;

    noBlink();
    if (argc < 2) {
        fprintf (stderr, "Need some parameters sucker !!\n");
        exit (1);
    }
    if (argc == 3) {
        if (argv[1][0] == '-') {
            if (strcmp (&argv[1][1], "cpq") == 0)
                compaq = 1;
        }
        else
            exit (1);
        fname = argv[2];
    }
    else
        fname = argv[1];

    if ((fp = fopen (fname, "r")) == NULL) {
        fprintf (stderr, "Cannot open file %s\n", fname);
        exit (1);
    }
    pg ();
    fclose (fp);
    return (0);
}
