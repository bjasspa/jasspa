/*****************************************************************************
*
*	Title:		%M%
*
*	Synopsis:	Key and name table porcessing.
*
******************************************************************************
*
*	Filename:		%P%
*
*	Author:			Jon Green
*
*	Creation Date:		02/12/91
*
*	Modification date:	%G%		<000723.1912>
*
*	Current rev:		%I%
*
*	Compilation Flags:	
*
*	Contents Description:	
*
******************************************************************************
*                                                                            
* Copyright (C) 1991 - 1999, JASSPA 
* 
* The MicroEmacs Jasspa distribution can be copied and distributed freely for
* any non-commercial purposes. The MicroEmacs Jasspa Distribution can only be
* incorporated into commercial software with the expressed permission of
* JASSPA.
* 
****************************************************************************/

#define	__KEYC			/* Define the filename */

/*---	Include files */
#include "emain.h"
#include "efunc.h"

/* Counts the size of the key table and stores it in the global variable
** keyTableSize
*/
void
count_key_table(void)
{
    register KEYTAB *ktp;		/* Keyboard character array */
    register int     ii;		/* Index counter */

/*---	Find end of table, incrementing i. */

    for (ktp=keytab, ii=0 ; ktp->code != ME_INVALID_KEY ; ii++, ktp++)
        ;
    keyTableSize = (uint16) ii ;
}

/*****************************************************************************
*								x.
*	Function Title:	decode_key ()
*
******************************************************************************
*
*	Synopsis:	Decode a key into an index value.
*
*	Invocation:	int	decode_key (code)
*			int	code;
*
*	Description:	This routine will decode a character sequence into
*			a character key.
*
*	Inputs:		code - The character code.
*
*	Outputs:	Returns the function name or NULL.
*
*****************************************************************************/

/* This is altered by isearch because it uses mlBinds, but when gets exit
 * command it must re-evaluate the same key but with no mlBinds
 */

int
decode_key(register uint16 code, uint32 *arg)
{
    register KEYTAB  *ktp;			/* Keyboard character array */
    register int      low;			/* Lowest index in table. */
    register int      hi;			/* Hightest index in table. */
    register int      mid;			/* Mid value. */
    register int      status;			/* Status of comparison. */
    
try_again:
#if LCLBIND
    if(useMlBinds)
    {
        ktp = mlBinds ;
        mid = mlNoBinds ;
    }
    else
    {
        mid = curbp->nobbinds ;
        ktp = curbp->bbinds ;
    }
    while(mid>0)
        if(ktp[--mid].code == code)
        {
            *arg = ktp[mid].arg ;
            return ktp[mid].index ;
        }

#endif
    /* binary chop through the key table looking for the character code.  
    ** If found then return the index into the names table.
    */
    ktp = keytab ;
    hi  = keyTableSize-1;	/* Set hi water to end of table */
    low = 0;			/* Set low water to start of table */
    
    do
    {
        mid = (low + hi) >> 1;		/* Get mid value. */
        if ((status=code-ktp[mid].code) == 0)
        {
            /* Found - return index */
            *arg = ktp[mid].arg ;
            return ktp[mid].index ;
        }
        else if (status < 0)
            hi = mid - 1;		/* Discard bottom half */
        else
            low = mid + 1;		/* Discard top half */
    } while (low <= hi);		/* Until converges */
    /* If this is an Alt key without any prefix's (esc, C-x etc.) then
     * change the key to be the 1st prefix without the Alt, e.g.
     *     A-space -> esc space
     * and then look this up
     */
    if(code & ME_ALT)
    {
#if MEOSD
        if((meSystemCfg & meSYSTEM_ALTMENU) &&
           ((code & (ME_SPECIAL|ME_PREFIX_MASK)) == 0) &&
           ((status = osdMainMenuCheckKey(code & 0xff)) != 0))
        {
            /* Found - return index */
            *arg = (uint32) (status+0x80000000) ;
            return CK_OSD ;
        }
#endif
        if((meSystemCfg & meSYSTEM_ALTPRFX1) &&
           ((code & ME_PREFIX_MASK) == 0))
        {
            code = (code & ~ME_ALT)|ME_PREFIX1 ;
            goto try_again ;
        }
    }
    *arg = 0 ;
    return -1 ;				/* Not found - return error */
}	/* End of "decode_key" () */

/*****************************************************************************
*								x.
*	Function Title:	delete_key ()
*
******************************************************************************
*
*	Synopsis:	Delete a key from the key table.
*
*	Invocation:	void	delete_key (index)
*			int	index;
*
*	Description:	This deletes the  specified key from  the key table.
*			The key is speficied with it's absolute table index.
*
*	Inputs:		index - The key table item to delete.
*
*	Outputs:	None - deletes item from table.
*
*****************************************************************************/

int
delete_key(register uint16 code)
{
    register KEYTAB	*ktp;			/* Keyboard character array */
    register int	ii ;			/* Index counter */

    /* move to the right part of the table */
    ii = keyTableSize ;
    ktp = keytab ;
    while(--ii >= 0)
    {
        if(ktp->code == code)
            break ;
        ktp++ ;
    }
    if(ii < 0)
        return FALSE ;
    keyTableSize-- ;
    /* move all infront back one */
    while(--ii >= 0)
    {
        memcpy(ktp,ktp+1,sizeof(KEYTAB)) ;
        ktp++ ;
    }
    return TRUE ;
}	/* End of "delete_key" () */

/*****************************************************************************
*								x.
*	Function Title:	insert_key ()
*
******************************************************************************
*
*	Synopsis:	Insert a new key into the key table.
*
*	Invocation:	int	insert_key (code, index)
*			int	code;
*			int	index;
*
*	Description:	This routine  will  insert a  new  key into  the key
* 			table.  The new  key is  always added to  the end of
* 			the table.
*
*	Inputs:		code - The new key code to add.
*			index - The new index to add.
*
*	Outputs:	Returns NULL if error.
*
*****************************************************************************/

int
insert_key (register uint16 code, uint16 index, uint32 arg)
/*  code - Code to add to key table. */
/* index - Index for the code. */
{
    register KEYTAB	*ktp;			/* Keyboard character array */
    register int	i;			/* Index counter */

    /* Is there room in the table */
    if(keyTableSize >= NBINDS-1)		/* No space in table ?? */
        return FALSE ;                          /* No - return error. */

/*--- Go backwards down table finding place in table and moving rest back. */
    i = keyTableSize++ ;
    ktp=&keytab[i] ;
    do {
        memcpy(ktp+1,ktp,sizeof(KEYTAB)) ;
        if((--ktp)->code <= code)
            break ;
    } while(--i > 0) ;

 /*---	Fill in the new code at new position */
    ktp++ ;
    ktp->index = index ;	/* Add new index */
    ktp->code  = code ;		/* Add new function code */
    ktp->arg   = arg ;		/* Add new argument */

    return TRUE ;
}	/* End of "insert_key" () */


