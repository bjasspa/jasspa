/* -*- C++ -*- ***************************************************************************
 *
 *  			Copyright 1997 Division Ltd.
 *			      All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: t.c,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2005-02-18 22:57:21 $
 *  Author        : $Author: jon $
 *  Created       : Thu Dec 18 18:03:33 1997
 *  Last Modified : <000229.2000>
 *
 *  Description	
 *
 *  Notes
 *
 *  History
 *	
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1.1.1  2000/07/14 22:05:26  Phillips
 *  Import
 *
 *
 ****************************************************************************
 *
 *  Copyright (c) 1997 Diesion Ltd.
 * 
 *  All Rights Reserved.
 * 
 * This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
 * reproduced,  translated,  or  reduced to any  electronic  medium or machine
 * readable form without prior written consent from Division Ltd.
 *
 ****************************************************************************/

static const char rcsid[] = "@(#) : $Id: t.c,v 1.1 2005-02-18 22:57:21 jon Exp $";

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

// Tuner information
struct s_TunerInfo
{
    uuuint32 state;                                   // Tuner state.
#define S_TUNE_IN_PROGRESS             0x00000001       // A tune operation is in progress.
#define S_AUDIO_MUTED                  0x00000004       // Audio output is muted.
#define S_TUNER_TUNED                  0x00000008       // Tuner is configured to correct frequency
#define S_VIDEO_TUNED                  0x00000010       // Video processing is configured to correct frequency
#define S_AUDIO_TUNED                  0x00000020       // Audio is configured to the correct frequency.
#define S_VIDEO_VALID                  0x00000040       // Visual output valid.
#define S_AUDIO_VALID                  0x00000080       // Audio output is valid.
#define S_VIDEO_MONITOR                0x00000100       // Video monior is enabled.
#define S_AUDIO_MONITOR                0x00000200       // Audio monioring is enabled.
#define S_AUDIO_PENDING                0x00000400       // Audio tune pending
#define S_VIDEO_PENDING                0x00000800       // Video tune pending
#define S_TUNER_PENDING                0x00001000       // Tuner tune pending
#define S_MUTE_PENDING                 0x00002000       // Mute operation pending.
#define S_TUNER_PRESENT                0x80000000       // Tuner is present
#define S_TUNE_PENDING                 (S_AUDIO_PENDING| \
                                        S_VIDEO_PENDING| \
                                        S_TUNER_PENDING)
    Tuning_t data;                                  // Current tuning information.
    
};

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

(
#def \
if(df) \
    sdfsdf \
else \
    sdf
 )

{
#define S_TUNE_PENDING                 S_AUDIO_PENDING| \
sdf \
dfgg
    dfg ;
    
    {
        //******************
    } ;
    
}
enum Dimensionality {
    D_2D_NonRational,
    D_2D_Rational,
    D_3D_NonRational,
    D_3D_Rational,
};

typedef struct
{
    int a ;
    //*************************
    // some comment
    //*************************
    int b ;
} BAR ;

#define foo() \
(dflkjg g \
 s;dgjdlf k \
 fklghj gflkh)

#define foo \
dflkjg g \
s;dgjdlf k \
fklghj gflkh 

enum {
    sgdfg,
    sdfgsdf,
}

enum
{
    sgdfg,
    sdfgsdf,
}

{
    enum
    {
        sgdfg,
        sdfgsdf,
    }
}
{
    if(sdfgsdgg
       dfhfgh) {
        sdfsdf ;
    }
    
}
main()
{
    if (s != NULL)
        if (s != NULL)
            while (*s != 0)
                if (*s++ == ' ')
                {
                    putchar(' ');
                }
                else if(*s++ == ' ')
                    putchar('-');
                else
                    putchar('-');
        else if(*s++ == ' ')
            putchar('-');
        else
            putchar('-');
    else if(*s++ == ' ')
        putchar('-');
    else
        putchar('-');
    
    if (s != NULL)
        while (*s != 0)
            if (*s++ == ' ')
                putchar(' ');
            else if(*s++ == ' ')
                putchar('-');
            else
                putchar('-');
    else if(*s++ == ' ')
        putchar('-');
    else
        putchar('-');
    
    
    printf("hello\n");
} // PrintRecHeading
{
    /*
     * Cheat here. Get the information from IOR[1]
     * 
     * Tap [0]
     *   use = 22 (0x0016) [BIOP_DELIVERY_PARA_USE]
     *   id = 0 (0x0000)
     *   assocTag = 34 (0x0022)               <<< Association Tag
     *   selector_length = 10 (0x000a)
     *   selector_type = 1 (0x0001)
     *   transactionId = 2 (0x00000002)       <<< Transaction Id 
     *   {
     *     updated_flag = 0
     *     identification = 1 (0x0001)
     *     version = 0 (0x0000)
     *     originator = 0 (0x0)
     *   }
     */
    
    /* Determine if we are in the selection region. If so then set
     * $mouse_pos to reflect the fact that we are in a region. 
       if ((curbp == wp->w_bufp) && (selhilight.flags & SELHIL_ACTIVE))
       {
       if ((selhilight.dlineno > selhilight.mlineno) ||
       ((selhilight.dlineno == selhilight.mlineno) && 
       (selhilight.dlineoff>=selhilight.mlineoff)))
       {
       if (((wp->line_no > selhilight.mlineno) ||
       ((wp->line_no == selhilight.mlineno) &&
       (wp->w_doto >= selhilight.mlineoff))) &&
       ((wp->line_no < selhilight.dlineno) ||
       ((wp->line_no == selhilight.dlineno) &&
       (wp->w_doto <= selhilight.dlineoff))))
       mouse_pos |= MIREGION;
       }
       else
       {
       if (((wp->line_no > selhilight.dlineno) ||
       ((wp->line_no == selhilight.dlineno) &&
       (wp->w_doto >= selhilight.dlineoff))) &&
       ((wp->line_no < selhilight.mlineno) ||
       ((wp->line_no == selhilight.mlineno) &&
       (wp->w_doto <= selhilight.mlineoff))))
       mouse_pos |= MIREGION;
       }
       }
     */
    
    if(!strncmp(fn,"http://",7) || !strncmp(fn,"ftp://",6))
    {
        /* djhfgh
         * 1) dfkjghdfgkj
         * 2) dlkfghd hlkgh
         * 3) ldfghkdfkgj
         * 4) dfljkghdkfgjd
         * 5) ;dfljfglhkfjgh
         */
    }
    do
        if(dfgdf)
        {
        }
    while() ;
    
    for(;;)
        if()
        {
        }
    
    
    MYClass::StaticMemberFunction(wibble, wobble);
    
    if(   space after (   tester
                          and )
          it seems to work!)
    {
        forsdfs()
        {
        }
        
        {
        }
    }
#define foo(); \
dflkjg g \
s;dgjdlf k \
fklghj gflkh 
    
}

class complex
{
    double re, im;
    
public:
    complex (double r, double i)	// Constructor !!
    {				// PROBLEM - '{' at wrong tab stop
        re = r;
        im = i;
    }				// PROBLEM - '}' at wrong tab stop.
    friend complex operator+(complex, complex);
}


int
tester(int i)
{
}

class complex
{
    double re, im;
    
public:
    complex (double r, double i)	// Constructor !!
    {				
        re = r;
        im = i;
    }				
    friend complex operator+(complex, complex);
}

struct foo
{
    int a;
    int b;
    struct bar	
    {			/* PROBLEM - similar to above. */		
        int c;
        int d;
    };
    int c;
    int d;
}

struct foo FooData =
{
    1,2,
    {
        3,4
    },		/* PROBLEM - not similar to above - just wierd */
    5,
    6
};

struct bar BarData =
{
    {'a','b'},
    {'c','d'},
    {
        {e,f},
        {g,h},          /* PROBLEM - why here ?? '{' opened line !! (similar to above I guess) */
        {i,j},
        {k,l}
    },
    {'d','g'}
}

int tester()
{
    /*
     * SDE_FILTER
     * 
     * The filter description structure for the encoder delivery module. The
     * interface is based on the LSI L64108 "MPEG-2 Transport with embedded MIPS
     * CPU (CW40001) and control chip"
     * 
     */
    if(this is a test...)
    {
        this indent is okay? ;
    }
    return ;
}

/* like this things are fine */
#define dnrb3DAplusCtimesB( r, a, c, b )         \
do {                                             \
    /* comment                                   \
* should be indented                        \
     */                                          \
    ((r)[DMD_X] = (a)[DMD_X] + (c) * (b)[DMD_X], \
     (r)[DMD_Y] = (a)[DMD_Y] + (c) * (b)[DMD_Y], \
     (r)[DMD_Z] = (a)[DMD_Z] + (c) * (b)[DMD_Z]);\
} while(0)

#define dnrb3DAplusCtimesB( r, a, c, b ) \
((r)[DMD_X] = (a)[DMD_X] + (c) * (b)[DMD_X], \
 (r)[DMD_Y] = (a)[DMD_Y] + (c) * (b)[DMD_Y], \
 (r)[DMD_Z] = (a)[DMD_Z] + (c) * (b)[DMD_Z])

#d ( \
     \
     )

/* like this things break */
#define dnrb3DScale( r, a, c ) ((r)[DMD_X] = (a)[DMD_X] * (c) , \
                                (r)[DMD_Y] = (a)[DMD_Y] * (c) , \
                                (r)[DMD_Z] = (a)[DMD_Z] * (c) )

#define dnrb3DScale( r, a, c ) ((r)[DMD_X] = (a)[DMD_X] * (c) , \
                                (r)[DMD_Y] = (a)[DMD_Y] * (c) , \
                                (r)[DMD_Z] = (a)[DMD_Z] * (c) )



while(lnk != NULL)
{
    /* The last line will be too long - dont remove the last \n and the offset
     * on the last line, i.e. if G is a wanted char and D is a char that should
       be deleted, we start with:
     * 
     * GGGGGGDDDDDDDDDD
     * DDDDDDDDDDDDDDDD
     * DDDDDDDDDDDDDDDD
     * DDDDDDDGGGGGGGG
     * 
     * We will try to get it to
     * 
     * GGGGGG
     * DDDDDDDGGGGGGGG
     * 
     * All we have to do is set the variables as if this is what happend
     */
    
    if(lnk->next == NULL)
        dglSubCoord(vv,chain->link_hd->uv,lnk->uv) ;
    else
        dglSubCoord(vv,lnk->next->uv,lnk->uv) ;
#if 0
    if((lnk->vert->point != llnk->vert->point) &&
       ((((uu[0]*vv[1]) - (uu[1]*vv[0])) != 0.0) ||
        (((uu[0]*vv[0]) + (uu[1]*vv[1])) >= 0.0) ))
#else
    if((((uu[0]*vv[1]) - (uu[1]*vv[0])) != 0.0) ||
       (((uu[0]*vv[0]) + (uu[1]*vv[1])) >= 0.0) )
#endif
    {
        dpgMonitor(0x10,("Keeping  link %d %d (%g,%g),(%g,%g) - uu (%g,%g), vv (%g,%g)\n",cc,
                         (lnk->vert->point != llnk->vert->point),
                         lnk->uv[0],lnk->uv[1],llnk->uv[0],llnk->uv[1],
                         uu[0],uu[1],vv[0],vv[1])) ;
    }
}

{
#ifdef _WIN32
    ss = WinLaunchProgram(cmdstr,LAUNCH_SYSTEM, NULL, NULL, &systemRet) ;
#else
#ifdef _UNIX
    /* if no data is piped in then pipe in /dev/null */
    if(strchr(cmdstr,'<') == NULL)
        strcat(cmdstr," </dev/null") ;
#endif
    systemRet = system(cmdstr) ;
#ifdef _UNIX
    if(WIFEXITED(systemRet))
    {
        systemRet = WEXITSTATUS(systemRet) ;
        ss = TRUE ;
    }
    else
        ss = FALSE ;
#else
#ifdef _DOS
    /* dos is naughty with modes, a system call could call a progam that 
     * changes the screen stuff under our feet and not restore the current
     * mode! The only thing we can do is call TTopen to ensure we restore
     * our mode.
     * We might check all states, but with hidden things like flashing etc.
     * its not worth the effort - sorry, you do it if you want.
     */
    TTopen() ;
#endif
    ss = (systemRet < 0) ? FALSE:TRUE ;
#endif
#endif
    /* dfkgjhdfgfjh \
     */
}
{
    {
    case LC_PMT:
        xpInitPIDRegister(whichLC);   
        
        /* Send to Messge Queue to Downstream Task for PLAYing Audio/Video Play          
        ** Let's play current program for 'ACS'Mode 
        */    
        tx_event.message.type = XP_DS_ACS_MODE;
        tx_event.message.id_type = ID_TASKID;  
        
        /*////////////////////////////////////
        ** Audio[1] : speaking "test card M "
        ** Audio[0] : speaking "? Hz "
        */
    case LC_PMT:
        xpInitPIDRegister(whichLC);   
    }
}

{
    if (test)
        result.cleanse("\\\"",'_');
    else
        sdf ;
}
