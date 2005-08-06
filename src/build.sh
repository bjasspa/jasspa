#!/bin/sh -f
# JASSPA MicroEmacs - www.jasspa.com
# build - JASSPA MicroEmacs build shell script for unix platforms
# Copyright (C) 2001-2005 JASSPA (www.jasspa.com)
# See the file main.c for copying and conditions.
OPTIONS=
LOGFILE=
LOGFILEA=
MAINTYPE=me
METYPE=
MEDEBUG=
MAKEFILE=
MAKECDEFS=

while [ ".$1" != "." ]
do
    if [ $1 = "-h" ] ; then
        echo "usage: build [options]"
        echo ""
        echo "Where options can be:-"
        echo "   -C   : Build clean."
        echo "   -d   : For debug build (output is med)."
        echo "   -h   : For this help page."
        echo "   -l <logfile>"
        echo "        : Set the compile log file"
        echo "   -la <logfile>"
        echo "        : Append the compile log to the given file"
        echo "   -m <makefile>"
        echo "        : Sets the makefile to use where <makefile> can be"
        echo "            aix4.mak, freebsd.mak, freebsd.gmk etc."
        echo "   -ne  : for NanoEmacs build (output is ne)."
        echo "   -p <search-path>"
        echo "        : Sets the default system search path to <search-path>,"
        echo "          default is "'"'"/usr/local/microemacs"'"'
        echo "   -S   : Build clean spotless."
        echo "   -t <type>"
        echo "        : Sets build type:"
        echo "             c  Console support only (Termcap)"
        echo "             w  Wondow support only (XTerm)"
        echo "             cw Console and window support (default)"
        echo ""
        exit 1
    elif [ $1 = "-C" ] ; then
        OPTIONS=clean
    elif [ $1 = "-d" ] ; then
        MEDEBUG=d
    elif [ $1 = "-l" ] ; then
        shift
        LOGFILE="$1"
    elif [ $1 = "-la" ] ; then
        shift
        LOGFILEA="$1"
    elif [ $1 = "-m" ] ; then
        shift
        MAKEFILE=$1
    elif [ $1 = "-ne" ] ; then
        MAINTYPE=ne
    elif [ $1 = "-p" ] ; then
        shift
        MAKECDEFS="-D_SEARCH_PATH=\\"'"'"$1\\"'"'
    elif [ $1 = "-S" ] ; then
        OPTIONS=spotless
    elif [ $1 = "-t" ] ; then
        shift
        METYPE=$1
    else
        echo "Error: Unkown option $1, run build -h for help"
        echo ""
        exit 1
    fi
    shift
done

if [ ".$MAKEFILE" = "." ] ; then
    
    PLATFORM=`uname`
    
    if   [ $PLATFORM = "AIX" ] ; then
        VERSION=`uname -v`
        if [ $VERSION = 5 ] ; then
            MAKEBAS=aix5
        else
            MAKEBAS=aix4
        fi
    elif [ $PLATFORM = "Darwin" ] ; then
        MAKEBAS=darwin
    elif [ $PLATFORM = "FreeBSD" ] ; then
        MAKEBAS=freebsd
    elif [ $PLATFORM = "HP-UX" ] ; then
        VERSION=`uname -r | cut -f 2 -d .`
        if [ $VERSION = 11 ] ; then
            MAKEBAS=hpux11
        elif [ $VERSION = 10 ] ; then
            MAKEBAS=hpux10
        else
            MAKEBAS=hpux9
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
    elif [ $PLATFORM = "OpenBSD" ] ; then
         MAKEBAS=openbsd
    elif [ $PLATFORM = "OSF1" ] ; then
         MAKEBAS=osf1
    elif [ $PLATFORM = "SunOS" ] ; then
        # Test for x86 version of Solaris
        VERSION=`uname -p`
        if [ $VERSION = "i386" ] ; then
            MAKEBAS=sunosx86
        else
            # Must be Sparc Solaris
            MAKEBAS=sunos5
        fi
    elif [ `echo $PLATFORM | sed -e "s/^CYGWIN.*/CYGWIN/"` = "CYGWIN" ] ; then
        MAKEBAS=cygwin
        # Check for an X11 install.
        if [ "$OPTIONS" = "" ] ; then
            if [ ! -f /usr/X11R6/lib/libX11-6.dll.a ] ; then
                echo "No X-11 support found, forcing terminal only."
                METYPE=c
            else
                echo "Found version of X11 installed"                
            fi
            sleep 5
        fi
    elif [ `echo $PLATFORM | sed -e "s/^MINGW.*/MINGW/"` = "MINGW" ] ; then
        MAKEBAS=mingw
    else
        echo "Error: Unrecognized platform - $PLATFORM"
        exit 1
    fi

    # use cc by default if available
    if [ -r $MAKEBAS.mak ] ; then
        # try to detect cc, if found use it in preference
        if [ "`type cc | cut -b 1-5`" = "cc is" ] ; then
            MAKEFILE=$MAKEBAS.mak
        fi
        # Special rules for sun, if cc is /usr/ucb then this is a dummy.
        if [ $PLATFORM = "SunOS" ] ; then 
            WHATCC=`/usr/bin/which cc`
            if [ "$WHATCC" = "/usr/ucb/cc" ] ; then
                MAKEFILE=""
            fi
        fi                      
    fi
    if [ ".$MAKEFILE" = "." ] ; then
        # failed to find cc, try gcc
        if [ -r $MAKEBAS.gmk ] ; then
            # try to detect gcc, if found use it in preference
            if [ "`type gcc | cut -b 1-6`" = "gcc is" ] ; then
                MAKEFILE=$MAKEBAS.gmk
            fi
        fi
    fi
    if [ ".$MAKEFILE" = "." ] ; then
        echo "Error: Failed to find a compiler (cc or gcc) fix PATH or use -m option"
        exit 1
    fi
fi

if [ ".$MAKECDEFS" != "." ] ; then
    MAKECDEFS="MAKECDEFS=$MAKECDEFS"
fi
if [ ".$OPTIONS" = "." ] ; then
    OPTIONS="$MAINTYPE$MEDEBUG$METYPE"
fi
if [ -r $MAKEFILE ] ; then
    if [ ".$LOGFILE" != "." ] ; then
        echo "make -f $MAKEFILE $OPTIONS $MAKECDEFS" > $LOGFILE 2>&1
        make -f $MAKEFILE $OPTIONS $MAKECDEFS > $LOGFILE 2>&1
    else
        if [ ".$LOGFILEA" != "." ] ; then
            echo "make -f $MAKEFILE $OPTIONS $MAKECDEFS" >> $LOGFILEA 2>&1
            make -f $MAKEFILE $OPTIONS $MAKECDEFS >> $LOGFILEA 2>&1
        else
            echo "make -f $MAKEFILE $OPTIONS $MAKECDEFS"
            make -f $MAKEFILE $OPTIONS $MAKECDEFS
        fi
    fi
else
    echo "Error: Failed to find the makefile $MAKEBAS.$MAKEEXT"
fi
