#!/bin/sh -f
# JASSPA MicroEmacs - www.jasspa.com
# build - JASSPA MicroEmacs build shell script for unix platforms
# Copyright (C) 2001-2009 JASSPA (www.jasspa.com)
# See the file main.c for copying and conditions.
OPTIONS=
LOGFILE=
LOGFILEA=
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
    
    if   [ $PLATFORM = "AIX" ] ; then
        VERSION=`uname -v`
        if [ $VERSION = 5 ] ; then
            MAKEBAS=aix5
        else
            MAKEBAS=aix4
        fi
    elif [ `echo $PLATFORM | sed -e "s/^CYGWIN.*/CYGWIN/"` = "CYGWIN" ] ; then
        MAKEBAS=cygwin
        X11_MAKEINC=/usr/X11R6/include
        X11_MAKELIB=/usr/X11R6/lib
    elif [ $PLATFORM = "Darwin" ] ; then
        VERSION=`uname -r | cut -f 1 -d .`
        if [ $VERSION -gt 18 ] ; then
            MAKEBAS=macos64
            X11_MAKEINC=/opt/X11/include
            X11_MAKELIB=/opt/X11/lib
        elif [ $VERSION -gt 15 ] ; then
            MAKEBAS=macos32
            X11_MAKEINC=/opt/X11/include
            X11_MAKELIB=/opt/X11/lib
        else
            MAKEBAS=darwin
            X11_MAKEINC=/usr/X11R6/include
            X11_MAKELIB=/usr/X11R6/lib
        fi
    elif [ $PLATFORM = "FreeBSD" ] ; then
        MAKEBAS=freebsd
        X11_MAKEINC=/usr/X11R6/include
        X11_MAKELIB=/usr/X11R6/lib
    elif [ $PLATFORM = "HP-UX" ] ; then
        X11_MAKEINC=/usr/include/X11R6
        X11_MAKELIB=/usr/lib/X11R6
        VERSION=`uname -r | cut -f 2 -d .`
        if [ $VERSION = 11 ] ; then
            MAKEBAS=hpux11
        elif [ $VERSION = 10 ] ; then
            MAKEBAS=hpux10
        else
            MAKEBAS=hpux9
            X11_MAKEINC=/usr/include/X11R5
            X11_MAKELIB=/usr/lib/X11R5
        fi
    elif [ `echo $PLATFORM | sed -e "s/^IRIX.*/IRIX/"` = "IRIX" ] ; then
        VERSION=`uname -r | cut -f 1 -d .`
        if [ $VERSION = 5 ] ; then
            MAKEBAS=irix5
        else
            MAKEBAS=irix6
        fi
    elif [ $PLATFORM = "Linux" ] ; then
        MACHINE=`uname -m | cut -c 1-3`
        if [ $MACHINE = "arm" ] ; then
            MAKEBAS=zaurus
        else
            KERNEL_MAJOR=`uname -r | cut -c 1-1`
            KERNEL_MINOR=`uname -r | cut -c 3-3`
            MAKEBAS="linux$KERNEL_MAJOR$KERNEL_MINOR"
            if [ ! -r $MAKEBAS.gmk ] ; then
                MAKEBAS="linux2"
            fi                
        fi
        X11_MAKELIB=/usr/X11R6/lib
    elif [ `echo $PLATFORM | sed -e "s/^MINGW32_NT.*/MINGW32_NT/"` = "MINGW32_NT" ] ; then
        MAKEBAS=win32mingw
    elif [ $PLATFORM = "OpenBSD" ] ; then
        MAKEBAS=openbsd
        X11_MAKEINC=/usr/X11R6/include
        X11_MAKELIB=/usr/X11R6/lib
    elif [ $PLATFORM = "OSF1" ] ; then
        MAKEBAS=osf1
        X11_MAKEINC=/usr/X11R6/include
        X11_MAKELIB=/usr/X11R6/lib
    elif [ $PLATFORM = "SunOS" ] ; then
        # Test for x86 version of Solaris
        VERSION=`uname -p`
        if [ $VERSION = "i386" ] ; then
            MAKEBAS=sunosx86
        else
            # Must be Sparc Solaris
            MAKEBAS=sunos5
        fi
        X11_MAKEINC=/usr/openwin/include
        X11_MAKELIB=/usr/openwin/lib
    elif [ `echo $PLATFORM | sed -e "s/^MINGW.*/MINGW/"` = "MINGW" ] ; then
        MAKEBAS=mingw
    else
        echo "Error: Unrecognized platform - $PLATFORM"
        exit 1
    fi

    # use cc by default if available
    if [ -r ${MAKEBAS}cc.mak ] ; then
        # try to detect cc, if found use it in preference
        if [ "`type cc | cut -b 1-5`" = "cc is" ] ; then
            MAKEFILE=${MAKEBAS}cc.mak
        fi
        # Special rules for sun, if cc is /usr/ucb then this is a dummy.
        if [ $PLATFORM = "SunOS" ] ; then 
            WHATCC=`/usr/bin/which cc`
            if [ "$WHATCC" = "/usr/ucb/cc" ] ; then
                MAKEFILE=""
            fi
        fi                      
    fi
    if [ -z "$MAKEFILE" ] ; then
        # failed to find cc, try gcc
        if [ -r ${MAKEBAS}gcc.mak ] ; then
            # try to detect gcc, if found use it in preference
            if [ "`type gcc | cut -b 1-6`" = "gcc is" ] ; then
                MAKEFILE=${MAKEBAS}gcc.mak
            fi
        fi
    fi
    if [ -z "$MAKEFILE" ] ; then
        echo "Error: Failed to find a supported compiler (cc or gcc) for current platform ($MAKEBAS) fix PATH or use -m option"
        exit 1
    fi
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
    echo "Error: Failed to find the makefile $MAKEFILE"
fi
