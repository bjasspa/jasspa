#!/bin/sh -f
# JASSPA MicroEmacs - www.jasspa.com
# build - JASSPA MicroEmacs build shell script for unix platforms
# Copyright (C) 2001-2009 JASSPA (www.jasspa.com)
# See the file main.c for copying and conditions.
OPTIONS=
LOGFILE=
LOGFILEA=
METYPE=
MEDEBUG=
MAKEFILE=
MAKECDEFS=
X11_MAKEINC=/usr/include
X11_MAKELIB=

while [ -n "$1" ]
do
    if [ $1 = "-h" ] ; then
        echo "usage: build [options]"
        echo ""
        echo "Where options can be:-"
        echo "   -C   : Build clean."
        echo "   -D <define>[=<value>]"
        echo "        : Build with given define, (e.g. -D _USETPARM)."
        echo "   -d   : For debug build (output is med)."
        echo "   -h   : For this help page."
        echo "   -l <logfile>"
        echo "        : Set the compile log file"
        echo "   -la <logfile>"
        echo "        : Append the compile log to the given file"
        echo "   -m <makefile>"
        echo "        : Sets the makefile to use where <makefile> can be"
        echo "            aix4.mak, freebsd.mak, freebsd.gmk etc."
        echo "   -S   : Build clean spotless."
        echo ""
        exit 1
    elif [ $1 = "-C" ] ; then
        OPTIONS=" clean"
    elif [ $1 = "-D" ] ; then
        shift
        if [ -n "$MAKECDEFS" ] ; then
            MAKECDEFS="$MAKECDEFS -D$1"
        else
            MAKECDEFS="-D$1"
        fi
    elif [ $1 = "-d" ] ; then
        MEDEBUG=" BCFG=debug"
    elif [ $1 = "-l" ] ; then
        shift
        LOGFILE="$1"
    elif [ $1 = "-la" ] ; then
        shift
        LOGFILEA="$1"
    elif [ $1 = "-m" ] ; then
        shift
        MAKEFILE=$1
    elif [ $1 = "-S" ] ; then
        OPTIONS=" spotless"
    else
        echo "Error: Unkown option $1, run build -h for help"
        echo ""
        exit 1
    fi
    shift
done

if [ -z "$MAKEFILE" ] ; then
    
    PLATFORM=`uname`
    
    if [ `echo $PLATFORM | sed -e "s/^MINGW.*/MINGW/"` = "MINGW" ] ; then
        MAKEBAS=win32mingw
    else
        echo "Error: Unrecognized or unsupported platform - $PLATFORM"
        exit 1
    fi

    if [ -r $MAKEBAS.mak ] ; then
        # try to detect cc, if found use it in preference
        if [ "`type cc | cut -b 1-5`" = "cc is" ] ; then
            MAKEFILE=$MAKEBAS.mak
        fi
    fi
    if [ -z "$MAKEFILE" ] ; then
        # failed to find cc, try gcc
        if [ -r $MAKEBAS.gmk ] ; then
            # try to detect gcc, if found use it in preference
            if [ "`type gcc | cut -b 1-6`" = "gcc is" ] ; then
                MAKEFILE=$MAKEBAS.gmk
            fi
        fi
    fi
    if [ -z "$MAKEFILE" ] ; then
        echo "Error: Failed to find a compiler (cc or gcc) fix PATH or use -m option"
        exit 1
    fi
elif [ ! "$MAKEFILE" = "win32mingw.gmk" ] ; then
    echo "Error: Unsupported platform makefile - $MAKEFILE"
    exit 1
fi

OPTIONS="$MEDEBUG$OPTIONS"
MAKECDEFS="MAKECDEFS=$MAKECDEFS"
if [ -r $MAKEFILE ] ; then
    if [ -n "$LOGFILE" ] ; then
        echo "make -f $MAKEFILE $OPTIONS \"$MAKECDEFS\"" > $LOGFILE 2>&1
        make -f $MAKEFILE $OPTIONS "$MAKECDEFS" > $LOGFILE 2>&1
    else
        if [ -n "$LOGFILEA" ] ; then
            echo "make -f $MAKEFILE $OPTIONS \"$MAKECDEFS\"" >> $LOGFILEA 2>&1
            make -f $MAKEFILE $OPTIONS "$MAKECDEFS" >> $LOGFILEA 2>&1
        else
            echo "make -f $MAKEFILE $OPTIONS \"$MAKECDEFS\""
            make -f $MAKEFILE $OPTIONS "$MAKECDEFS"
        fi
    fi
else
    echo "Error: Failed to find the makefile $MAKEBAS.$MAKEEXT"
fi
