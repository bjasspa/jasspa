/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * winterm.h - Windows shared definitions.
 *
 * Copyright (C) 1999-2001 Jon Green
 * Copyright (C) 2002-2009 JASSPA (www.jasspa.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/*
 * Created:     Mon May 24 1999
 * Synopsis:    Windows shared definitions.
 * Authors:     Jon Green
 * Description:
 *     Export shared definitions of the windows specific environment with
 *     the different windows specific source files.
 */

#ifndef __WINTERM_H__
#define __WINTERM_H__

/* Font type settings */
extern LOGFONT ttlogfont;               /* Current logical font */

/* The metrics associated with a charcter cell */
typedef struct
{
    int sizeX;                          /* Character cell size in X width */
    int sizeY;                          /* Character cell size in Y (height) */
    int midX;                           /* Mid position of the cell */
    int midY;                           /* Mid position of the cell */
}  CharMetrics;

#endif /* __WINTERM_H__ */

