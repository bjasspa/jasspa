#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#if ((defined _HPUX) || (defined _LINUX))
#include <unistd.h>
#else
#include <getopt.h>
#endif

#include <utils.h>
#include "nroff.h"

/* Macro Definitions */
/*
 * 1.0.0a - JG 16/11/96 Integrated new utilies library.
 *
 * 1.0.0b - JG 05/12/95 Added bullet support.
 */

#define MODULE_VERSION  "1.0.0b"
#define MODULE_NAME     "nrinfo"

static char *sectionId = NULL;
static char *sectionModule = NULL;
static char *sectionComponent = NULL;
static char *progname = MODULE_NAME;
static FILE *fpr = NULL;

/*
 * addComponent
 * Add a component to the tags. Decompose into it's component parts.
 */


static void
addComponent (char *s)
{
    char *allocBuf;                     /* Allocated buffer */
    char *q;                            /* Start of current component */
    char *p;                            /* Temporary character string */
    char c;                             /* Current charater */

    allocBuf = bufNStr (NULL, s);       /* Copy string */
    if ((q = allocBuf) == NULL)         /* Quit if NULL */
        return;

    p = q;
    for (;;)
    {
        if (((c = *p) == '\0') || (c == '|'))
        {
            *p = '\0';
            if (*q == '\0')
                uError ("Null tag\n");
            else
            {
                TagP tp;

                /*
                 * Allocate a new tag and add to list.
                 */

                tp = tagAlloc (sectionModule, q, NULL, nrfp->fileName,
                               nrfp->lineNo, NULL);
                add_tag (tp);
            }
            if (c == 0)
                break;
            else
                q = ++p;                /* Reset start of tag */
        }
        else
            p++;                        /* Next character */
    }

    bufFree (allocBuf);                 /* Free off allocated buffer */
}

static void
nrIm_func (char *module, char *component)
{
    bufFree (sectionModule);
    if ((sectionModule = bufNStr (NULL, module)) == NULL)
        sectionModule = bufNStr (NULL, "UNDEFINED");
    bufFree (sectionComponent);
    sectionComponent = bufNStr (NULL, ((component == NULL) ? "misc" : component));
    addComponent (sectionComponent);
}

static void
nrTH_func (char *id, char *num, char *date, char *company, char *title)
{
    bufFree (sectionId);
    sectionId = bufStr (NULL, num);
}

static void
nrStartInc (char *s, int *imode)
{
    *imode = 1;
}

static void
nrXI_func (char *name, char *id, char *desc, char *comp)
{
    addComponent (comp);
}

static void
nrXJ_func (char *name, char *id, char *desc, char *comp)
{
    addComponent (comp);
}

static void
alpha_term (void)
{
    TagP lp;
    TagP tp;

    lp = NULL;
    for (tp = tagIterateInit (); tp != NULL; tp = tagIterate (tp)) {
        if (lp != NULL) {
            if ((strcmp (lp->name, tp->name) == 0) &&
                (strcmp (lp->section, tp->section) == 0))
                lp = tp;
        }
        if (lp != tp)
            fprintf ((fpr == NULL) ? stdout : fpr, "Module [%s]. Component [%s]: ", &tp->name[1], tp->section);
        fprintf ((fpr == NULL) ? stdout : fpr, "%s\n", tp->file);
        lp = tp;
    }
}

static void
usage (void)
{
    fprintf (stdout, "%s : Usage %s [Options] <files>\n", progname, progname);
    fprintf (stdout, "\n");
    fprintf (stdout, "-?/h      : Help - this page\n");
    fprintf (stdout, "-E <file> : Log errors to <file> (append)\n");
    fprintf (stdout, "-e <file> : Log errors to <file> (re-write)\n");
    fprintf (stdout, "-I        : Version Information\n");
    fprintf (stdout, "-o <file> : Output to <file> default is <files>.out\n");
    fprintf (stdout, "-q        : Quiet mode\n");
    exit (1);
}

static void
infoInitialise (void)
{
    static  nrFUNCTION funcTab = {NULL};

    /* Headers */
    nrInstall (funcTab, TH_func, nrTH_func);

    /* Identification */
    nrInstall (funcTab, XI_func, nrXI_func);
    nrInstall (funcTab, XJ_func, nrXJ_func);
    nrInstall (funcTab, Im_func, nrIm_func);

    /* Files */
    nrInstall (funcTab, startFile_func, NULL);
    nrInstall (funcTab, includeFile_func, nrStartInc);
    nrInstall (funcTab, endFile_func, NULL);

    nrInstallFunctionTable (&funcTab);
}

int
main (int argc, char *argv [])
{
    char    *oname = NULL;              /* Output file name */
    int     ecount = 0;                 /* Error count */
    int     wcount = 0;                 /* Warn count */
    nrFILE  *nrfp;
    int     c;
    int     i;

    /* Initialise the error channel */
    infoInitialise ();
    uErrorSet (0, &ecount);             /* Max Num entries + counter */
    uWarnSet (&wcount);                 /* Warning count */
    uUtilitySet (progname);             /* Set name of program */

    while (1) {
        c = getopt (argc, argv, "e:E:?hqIo:");
        if (c == EOF)
            break;
        switch (c) {
        case 'e':
            uErrorChannelSet (optarg, UERROR_LOG_CREATE);
            break;
        case 'E':
            uErrorChannelSet (optarg, UERROR_LOG_APPEND);
            break;
        case 'I':
            fprintf (stdout, "%s. Version %s. %s. %s\n",
                     progname, MODULE_VERSION, __DATE__, nroffVersion);
            exit (0);
            break;
        case 'q':
            uInteractiveSet (0);        /* Disable interactive mode */
            break;
        case 'o':
            oname = optarg;
            break;
        case '?':
        case 'h':
            usage ();
            break;
        default:
            uError ("Cannot understand option [-%c]\n", c);
            usage();
            break;
        }
    }

    uOpenErrorChannel ();
    argv = getfiles (&argc, argv, optind);

    if (argc > 1) {
        if (oname != NULL) {
            if ((fpr = fopen (oname, "wb")) == NULL)
                uFatal ("Cannot open file [%s]\n", oname);
            uVerbose (0, "Output in file [%s].\n", oname);
        }
        else
            fpr = NULL;
    }
    else
        fpr = NULL;

    /* Process all of the files */

    for (i = 1; i < argc; i++) {
        if ((nrfp = nrFilePush (argv[i])) == NULL)
            exit (1);

        /* Set up the conversion */
        nroff (nrfp, NROFF_MODE_DEFAULT);
    }
    alpha_term ();
    if (fpr != NULL)
        fclose (fpr);

    uCloseErrorChannel ();
    return (ecount);
}
