#____________________________________________________________________________
#
# Last Edited <240398.1430>
#
# Nroff tools makefile.
#____________________________________________________________________________
#
TARGET	= linux
LOCALDIR= ../src
ROOTDIR = ../..
include $(ROOTDIR)/etc/$(TARGET)/makeinc
#____________________________________________________________________________
#
# Local definitions.
#____________________________________________________________________________
#
VPATH	= $(LOCALDIR)

LOCALLIBS=	-L$(INSTLIBDIR)  -lutils
INCLUDES=       $(I). $(I)$(INSTINCDIR)
CLIBS	=	libnroff.$A
# -lmalloc
# -DDEBUG

HEADERS	=	$(LOCALDIR)/nroff.def	\
		$(LOCALDIR)/_nroff.h	\
		$(LOCALDIR)/nroff.h

UTILS_H		= $(INSTINCDIR)/utils.h

RTFEXES	=	nr2rtf$(EXE)
HTMEXES	=	nr2html$(EXE)	htmlc$(EXE)
MANEXES	=	superman$(EXE)	droff$(EXE)
TOLEXES	=	nrsearch$(EXE)	nrinfo$(EXE)	nrcheck$(EXE)	ntags$(EXE) \
		nrorder$(EXE)	difftags$(EXE)	nrar$(EXE)	hts2html$(EXE)
LOCLEXES=	table$(EXE)
# nroff$(ESH)
# nroff.bat
#nrpg$(EXE)
#CANEXES	=	croff$(EXE)	dman$(EXE)	dbzc$(EXE)
ALLEXES	=	$(RTFEXES) $(HTMEXES) $(MANEXES) $(TOLEXES) \
		$(CANEXES) $(LOCLEXES)
#____________________________________________________________________________
#
# Dependencies
#____________________________________________________________________________
#
#$(ALLEXES)::	
.SUFFIXES:	.o .c
%.o:%.c
	$(CC) $(CFLAGS) -c $< 
%:%.o
	$(CC) $(CFLAGS) -o $* $*.$O $(LDFLAGS) $(CLIBS) $(LOCALLIBS) $(SYSLIBS)
%:%.c
	$(CC) $(CFLAGS) -c $< 
	$(CC) $(CFLAGS) -o $* $*.$O $(LDFLAGS) $(CLIBS) $(LOCALLIBS) $(SYSLIBS)
	
all:		$(ALLEXES)

croff.$O:	$(UTILS_H) $(HEADERS)
dbzc.:		$(UTILS_H) $(HEADERS)
difftags.$O:	$(UTILS_H) $(HEADERS)
dman.$O:	$(UTILS_H) $(HEADERS)
droff.$O:	$(UTILS_H) $(HEADERS)
htmlc.$O:	$(UTILS_H) $(HEADERS)
nr2html.$O:	$(UTILS_H) $(HEADERS) $(LOCALDIR)/html.h
nr2rtf.$O:	$(UTILS_H) $(HEADERS) $(LOCALDIR)/rtf.h
nrcheck.$O:	$(UTILS_H) $(HEADERS)
nrinfo.$O:	$(UTILS_H) $(HEADERS)
nrorder.$O:	$(UTILS_H) $(HEADERS)
nrsearch.$O:	$(UTILS_H) $(HEADERS)
ntags.$O:	$(UTILS_H)
superman.$O:	$(UTILS_H)
nrar.$O:	$(UTILS_H)
hts2html.$O:	$(UTILS_H)
#$(ALLEXES) :	%: %.o
#	$(CC) $(CFLAGS) -o $@ $@.$O $(LDFLAGS) $(CLIBS) $(LOCALLIBS) $(SYSLIBS)
$(ALLEXES):	$(INSTLIBDIR)/libutils.$A libnroff.$A

nroff.bat: nroff.bsh
	cp nroff.bsh nroff.bat

nroff:		nroff.sh
		cat $@.sh > $@
		chmod a+x $@

tab.$O:		tab.c $(UTILS_H) $(GETOPT_H) _nroff.h nroff.h
	cc -D_UNIX -D_LINUX -I../include -g -Wall -c tab.c -o tab.$O
tab$(EXE):	tab.$O libnroff.$A $(INSTLIBDIR)/libutils.$A
	$(CC) -Wall -g -o $@ tab.$O $(LDFLAGS) $(CLIBS) $(SYSLIBS)

#____________________________________________________________________________
#
# Build the nroff library.
#____________________________________________________________________________
#
catman.$O:	$(UTILS_H) $(HEADERS)
dtags.$O:	$(UTILS_H) $(HEADERS)
nroff.$O:	$(UTILS_H) $(HEADERS)
nlibrary.$O:	$(UTILS_H) $(HEADERS)

LIBSRC	=	nroff.c dtags.c catman.c nlibrary.c
LIBOBJ	=	$(LIBSRC:.c=.$O)

libnroff.$A:	$(LIBOBJ)
		$(RM) $@
		$(AR) $(ARFLAGS) $@ $(LIBOBJ)
#____________________________________________________________________________
#
# Install the files.
#____________________________________________________________________________
#
install:	$(ALLEXES)
		$(MKDIR) $(INSTBINDIR)
		$(INSTALL) $(INSTBINFLAGS) $(RTFEXES) $(INSTBINDIR)
		$(INSTALL) $(INSTBINFLAGS) $(HTMEXES) $(INSTBINDIR)
		$(INSTALL) $(INSTBINFLAGS) $(MANEXES) $(INSTBINDIR)
		$(INSTALL) $(INSTBINFLAGS) $(TOLEXES) $(INSTBINDIR)

clean::
	$(RM) $(ALLEXES) *.$O *.$A
	@echo Cleaned

spotless:: clean
	$(RM) *~
	$(RM) $(LOCALDIR)/*~
