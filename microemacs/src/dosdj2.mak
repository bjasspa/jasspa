# -!- makefile -!- ###########################################################
#
#			MicroEmacs Jasspa Distribution 1999.
#
#  System        : MicroEmacs
#  Description	 : Make file for Dos using djgpp v2.0
#  Created       : Sat Jan 24 00:01:40 1998
#
#  Last Modified : <000420.0745>
#
#  Notes
#	DJDIR must be setup to point to your DJGPP installation (normally
#		done in djgpp.env)
#
#	Run "make -f dosdj1.mak"      for optimised build produces ./me.exe
#	Run "make -f dosdj1.mak med"  for debug build produces     ./med.386
#
#	Run "make -f dosdj1.mak clean"      to clean source directory
#	Run "make -f dosdj1.mak spotless"   to clean source directory even more
#
#  History
#
##############################################################################
#
# Installation Directory
INSTDIR		= c:\emacs
INSTPROGFLAGS	= 
#
# Local Definitions
COFF          = coff2exe
RM            = del
CC            = gcc
LD            = $(CC)
STRIP         =	strip
CDEBUG        =	-g -Wall
COPTIMISE     =	-O2 -DNDEBUG=1 -Wall
CDEFS         = -D_DOS -D__DJGPP2__ -I.
LIBS          = -lpc
LDFLAGS       = 
INSTALL       =	copy
#
# Rules
.SUFFIXES: .c .o .od

.c.o:	
	$(CC) $(COPTIMISE) $(CDEFS) -c $<

.c.od:	
	$(CC) $(CDEBUG) $(CDEFS) -o $*.od -c $<
#
# Source files
STDHDR	= ebind.h edef.h eextrn.h efunc.h emain.h emode.h eprint.h \
	  esearch.h eskeys.h estruct.h eterm.h evar.h evers.h \
	  ebind.def efunc.def eprint.def evar.def etermcap.def emode.def eskeys.def
STDSRC	= abbrev.c basic.c bind.c buffer.c crypt.c dirlist.c display.c \
	  eval.c exec.c file.c fileio.c hilight.c history.c input.c \
	  isearch.c key.c line.c macro.c main.c narrow.c next.c osd.c \
	  print.c random.c regex.c region.c registry.c search.c spawn.c \
	  spell.c tag.c termio.c time.c undo.c window.c word.c

PLTHDR  = 
PLTSRC  = dosterm.c
#
# Object files
STDOBJ	= $(STDSRC:.c=.o)	
PLTOBJ  = $(PLTSRC:.c=.o)
OBJ	= $(STDOBJ) $(PLTOBJ)

DSTDOBJ	= $(STDSRC:.c=.od)	
DPLTOBJ = $(PLTSRC:.c=.od)
DOBJ	= $(DSTDOBJ) $(DPLTOBJ)
#
# Targets
all: me

install: me
	$(INSTALL) $(INSTPROGFLAGS) me $(INSTDIR)
	@echo "install done"

clean:
	$(RM) me.exe
	$(RM) med.exe
	$(RM) me.386
	$(RM) med.386
	$(RM) *.o
	$(RM) *.od

spotless: clean
	$(RM) tags
	$(RM) *~

me:	me.exe

me.exe:	$(OBJ)
	$(RM) me.386
	$(RM) me.exe
	$(LD) $(LDFLAGS) -o me.386 $(OBJ) $(LIBS)
	$(STRIP) me.386
	$(COFF) me.386

med:	med.exe

med.exe: $(DOBJ)
	$(RM) med.386
	$(RM) med.exe
	$(LD) $(LDFLAGS) -o med.386 $(DOBJ) $(LIBS)
	$(COFF) med.386
#
# Dependancies
$(STDOBJ): $(STDHDR)
$(PLTOBJ): $(STDHDR) $(PLTHDR)

$(DSTDOBJ): $(STDHDR)
$(DPLTOBJ): $(STDHDR) $(PLTHDR)

