#____________________________________________________________________________
#
# Last Edited <240398.1434>
#
# Nroff tools makefile.
#____________________________________________________________________________
#

#TARGET	= hpux
TARGET	= dos
LOCALDIR= ../src
ROOTDIR= ../..
include $(ROOTDIR)/etc/$(TARGET)/makeinc.mk
#____________________________________________________________________________
#
# Local definitions.
#____________________________________________________________________________
#
VPATH	= $(LOCALDIR)

#CFLAGS	=	-I. -Wall
#CFLAGS	=	-I$(INCDIR) -DNDEBUG -O -Wall
CFLAGS	=	-I$(INCDIR) -I$(LOCALDIR) -O3 -Wall
#CFLAGS	=	-I$(INCDIR) -g -Wall
LDFLAGS =	-L$(LIBDIR)
CLIBS	=	libnroff.a -lutils
# -DDEBUG

HEADERS	=	$(LOCALDIR)/nroff.h \
		$(LOCALDIR)/nroff.def \
		$(LOCALDIR)/_nroff.h

RTFEXES	=	nr2rtf$(EXE)
HTMEXES	=	nr2html$(EXE)	htmlc$(EXE)
MANEXES	=	superman$(EXE)	droff$(EXE)
TOLEXES	=	nrsearch$(EXE)	nrinfo$(EXE)	nrcheck$(EXE)	ntags$(EXE) \
		nrorder$(EXE)	difftags$(EXE)	nrar$(EXE)	hts2html$(EXE) \
		hts$(EXE)	nr2ehf$(EXE)
#nroff$(ESH)
#table$(EXE)
# nroff.bat
#nrpg$(EXE)
#CANEXES	=	croff$(EXE)	dman$(EXE)	dbzc$(EXE)
CANEXES	=

ALLEXES	=	$(RTFEXES) $(HTMEXES) $(MANEXES) $(TOLEXES) $(CANEXES)

all::		$(ALLEXES)

#;$(SRC) $(HEADERS):	RCS/$$@
#;			$(CO) $@
GETOPT_H	=
UTILS_H		= $(INCDIR)/utils.h $(LIBDIR)/libutils.a

croff$(EXE):		$(UTILS_H) $(GETOPT_H) $(HEADERS) libnroff.a
dbzc$(EXE):		$(UTILS_H) $(GETOPT_H) $(HEADERS) libnroff.a
difftags$(EXE):		$(UTILS_H) $(GETOPT_H)
dman$(EXE):		$(UTILS_H) $(GETOPT_H) $(HEADERS) libnroff.a
droff$(EXE):		$(UTILS_H) $(GETOPT_H) $(HEADERS) libnroff.a
htmlc$(EXE):		$(UTILS_H) $(GETOPT_H)
nr2html$(EXE):		$(UTILS_H) $(GETOPT_H) $(HEADERS) libnroff.a $(LOCALDIR)/html.h
droff.o:		$(UTILS_H) $(GETOPT_H) $(HEADERS) libnroff.a $(LOCALDIR)/rtf.h
droff$(EXE):		droff.o
	$(CC) $(CFLAGS) -o $*.386 $< $(LDFLAGS) $(CLIBS) $(SYSLIBS)
	$(COFF2EXE) $(CEFLAGS) $*.386
nr2rtf.o:		$(UTILS_H) $(GETOPT_H) $(HEADERS) libnroff.a $(LOCALDIR)/rtf.h
nr2rtf$(EXE):		nr2rtf.o
	$(CC) $(CFLAGS) -o $*.386 $< $(LDFLAGS) $(CLIBS) $(SYSLIBS)
	$(COFF2EXE) $(CEFLAGS) $*.386

nrcheck$(EXE):		$(UTILS_H) $(GETOPT_H) $(HEADERS) libnroff.a
nrinfo$(EXE):		$(UTILS_H) $(GETOPT_H) $(HEADERS) libnroff.a
nrorder$(EXE):		$(UTILS_H) $(GETOPT_H) $(HEADERS) libnroff.a
nrsearch$(EXE):		$(UTILS_H) $(GETOPT_H) $(HEADERS) libnroff.a $(LOCALDIR)/nrsearch.def
ntags$(EXE):		$(UTILS_H) $(GETOPT_H)
superman$(EXE):		$(UTILS_H) $(GETOPT_H)
nrar$(EXE):		$(UTILS_H) $(GETOPT_H) $(HEADERS) libnroff.a
hts2html$(EXE):		$(UTILS_H) $(GETOPT_H)
nroff.bat: nroff.bsh
	cp nroff.bsh nroff.bat

nroff:			nroff.sh
			cat $@.sh > $@
			chmod a+x $@

table.o:		$(UTILS_H) $(GETOPT_H) $(LOCALDIR)/nroff.h
table.exe:		table.o libnroff.a $(LIBDIR)/libutils.a
	$(CC) $(CFLAGS) -o $*.386 $*.o $(LDFLAGS) $(CLIBS) $(SYSLIBS)
	$(COFF2EXE) $(CEFLAGS) $*.386


#____________________________________________________________________________
#
# Build the nroff library.
#____________________________________________________________________________
#

catman.$O::	$(INCDIR)/utils.h $(HEADERS)
dtags.$O::	$(INCDIR)/utils.h $(HEADERS)
nroff.$O::	$(INCDIR)/utils.h $(HEADERS) nroff.c
nlibrary.$O::	$(HEADERS)

LIBOBJS	=	nroff.$O dtags.$O catman.$O nlibrary.$O

libnroff.a::	$(LIBOBJS)
		$(RM) $@
		$(AR) $(ARFLAGS) $@ $(LIBOBJS)
#____________________________________________________________________________
#
# Install the files.
#____________________________________________________________________________
#
RELBINDIR=../../bin
install:	$(ALLEXES)
		$(INSTALL) $(RELBINDIR) $(RTFEXES)
		$(INSTALL) $(RELBINDIR) $(HTMEXES)
		$(INSTALL) $(RELBINDIR) $(MANEXES)
		$(INSTALL) $(RELBINDIR) $(TOLEXES)
#		$(INSTALL) -nfd $(INSTDIR) $(RTFEXES)
#		$(INSTALL) -f $(INSTDIR) $(HTMEXES)
#		$(INSTALL) -f $(INSTDIR) $(MANEXES)
#		$(INSTALL) -f $(INSTDIR) $(TOLEXES)
#		$(INSTALL) $(CANEXES)	$(INSTDIR)

backup:

	pkzip -ex c:/backup/nroff.zip *.*
clean::
	$(RM) $(ALLEXES) *.o *.a *.386 *.??~
	@echo Cleaned
