#!/bin/sh -f
# JASSPA MicroEmacs - www.jasspa.com
# build - JASSPA MicroEmacs build shell script for unix platforms
# Copyright (C) 2001-2024 JASSPA (www.jasspa.com)
# See the file main.c for copying and conditions.
BITSIZE=
LOGFILE=
LOGFILEA=
MEARCH=
MECORE=
MEPROF=
METYPE=
MEDEBUG=
MAKEFILE=
MAKECDEFS=
OPTIONS=

while [ -n "$1" ]
do
    if [ $1 = "-h" ] ; then
        echo "usage: build [options]"
        echo ""
        echo "Where options can be:-"
        echo "   -32  : Build 32bit binary."
        echo "   -64  : Build 64bit binary."
        echo "   -a <value>"
        echo "        : Define the required architecture."
        echo "   -C   : Build clean."
        echo "   -D <define>[=<value>]"
        echo "        : Build with given define, (e.g. -D _USETPARM)."
        echo "   -d   : For debug build."
        echo "   -h   : For this help page."
        echo "   -l <logfile>"
        echo "        : Set the compile log file"
        echo "   -la <logfile>"
        echo "        : Append the compile log to the given file"
        echo "   -m <makefile>"
        echo "        : Sets the makefile to use where <makefile> can be"
        echo "            linuxgcc.mak, macoscc.mak etc."
        echo "   -ne  : for NanoEmacs build (output is ne)."
        echo "   -P   : Build with profiling instructions."
        echo "   -p <search-path>"
        echo "        : Sets the default system search path to <search-path>,"
        echo "          default is "'"'"/usr/local/microemacs"'"'
        echo "   -S   : Build clean spotless."
        echo "   -t <type>"
        echo "        : Sets build type:"
        echo "             c   Console support only (Termcap/ncurses - default)"
        echo "             w   Window support only (XTerm)"
        echo "             cw  Console and window support"
        echo "   -v <variable>=<value>"
        echo "        : Build with given variable override, (e.g. -v OPENSSLPATH=...)"
        echo ""
        exit 1
    elif [ $1 = "-32" ] ; then
        BITSIZE=" BIT_SIZE=32"
    elif [ $1 = "-64" ] ; then
        BITSIZE=" BIT_SIZE=64"
    elif [ $1 = "-a" ] ; then
        shift
        MEARCH=" ARCHITEC=$1"
    elif [ $1 = "-C" ] ; then
        OPTIONS=" clean"
    elif [ $1 = "-D" ] ; then
        shift
        MAKECDEFS="$MAKECDEFS -D$1"
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
    elif [ $1 = "-ne" ] ; then
        MECORE=" BCOR=ne"
    elif [ $1 = "-P" ] ; then
        MEPROF=" BPRF=1"
    elif [ $1 = "-p" ] ; then
        shift
        MAKECDEFS="$MAKECDEFS -D_SEARCH_PATH=\\"'"'"$1\\"'"'
    elif [ $1 = "-S" ] ; then
        OPTIONS=" spotless"
    elif [ $1 = "-t" ] ; then
        shift
        METYPE=" BTYP=$1"
    elif [ $1 = "-v" ] ; then
        shift
        OPTIONS=" $1$OPTIONS"
    else
        echo "Error: Unkown option $1, run build -h for help"
        echo ""
        exit 1
    fi
    shift
done

if [ -z "$MAKEFILE" ] ; then
    
    PLATFORM=`uname`
    
    if [ $PLATFORM = "Darwin" ] ; then
        VERSION=`uname -r | cut -f 1 -d .`
        if [ $VERSION -gt 15 ] ; then
            MAKEBAS=macos
        else
            MAKEBAS=darwin
        fi
    elif [ $PLATFORM = "Linux" ] ; then
        MAKEBAS=linux
    elif   [ $PLATFORM = "AIX" ] ; then
        VERSION=`uname -v`
        if [ $VERSION = 5 ] ; then
            MAKEBAS=aix5
        else
            MAKEBAS=aix4
        fi
    elif [ `echo $PLATFORM | sed -e "s/^CYGWIN.*/CYGWIN/"` = "CYGWIN" ] ; then
        MAKEBAS=cygwin
    elif [ `echo $PLATFORM | sed -e "s/^MSYS.*/MSYS/"` = "MSYS" ] ; then
        MAKEBAS=msyswin
    elif [ $PLATFORM = "FreeBSD" ] ; then
        MAKEBAS=freebsd
    elif [ `echo $PLATFORM | sed -e "s/^MINGW.*/MINGW/"` = "MINGW" ] ; then
        MAKEBAS=winmingw
        MAKEFILE=${MAKEBAS}.mak
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

MAKE=make
if [ "$MAKEFILE" = "winmingw.mak" ] ; then
    # when cross compiling the system gnu make can be used
    if [ "`type mingw32-make | cut -b 1-15`" = "mingw32-make is" ] ; then
        MAKE=mingw32-make
    fi
    if [ "`type windres | cut -b 1-10`" = "windres is" ] ; then
      # Using native tools
      :
    else
      if [ "$BITSIZE" = " BIT_SIZE=64" ] ; then
        if [ "`type x86_64-w64-mingw32-windres | cut -b 1-29`" = "x86_64-w64-mingw32-windres is" ] ; then
          # Using a cross compiler
          OPTIONS=" MK=$MAKE TOOLPREF=x86_64-w64-mingw32-$OPTIONS"
        else
          echo "Failed to find 64bit windres!"
          exit 1
        fi
      elif [ "`type i686-w64-mingw32-windres | cut -b 1-27`" = "i686-w64-mingw32-windres is" ] ; then
        # Using a cross compiler
        OPTIONS=" MK=$MAKE TOOLPREF=i686-w64-mingw32-$OPTIONS"
      else
        echo "Failed to find 32bit windres!"
        exit 1
      fi
      if [ -z "$INCLUDE" ] ; then
         OPTIONS=" RCOPTS=-I$INCLUDE$OPTIONS"
      fi
    fi
fi
OPTIONS="$MEARCH$MECORE$MEDEBUG$METYPE$MEPROF$BITSIZE$OPTIONS"
MAKECDEFS="MAKECDEFS=$MAKECDEFS"
if [ -r $MAKEFILE ] ; then
    if [ -n "$LOGFILE" ] ; then
        echo "$MAKE -f $MAKEFILE $OPTIONS \"$MAKECDEFS\"" > $LOGFILE 2>&1
        $MAKE -f $MAKEFILE $OPTIONS "$MAKECDEFS" > $LOGFILE 2>&1
    else
        if [ -n "$LOGFILEA" ] ; then
            echo "$MAKE -f $MAKEFILE $OPTIONS \"$MAKECDEFS\"" >> $LOGFILEA 2>&1
            $MAKE -f $MAKEFILE $OPTIONS "$MAKECDEFS" >> $LOGFILEA 2>&1
        else
            echo "$MAKE -f $MAKEFILE $OPTIONS \"$MAKECDEFS\""
            $MAKE -f $MAKEFILE $OPTIONS "$MAKECDEFS"
        fi
    fi
else
    echo "Error: Failed to find the makefile $MAKEFILE"
    exit 1
fi
