#____________________________________________________________________________
#
# Last Edited <231205.1334>
#
# Nroff tools makefile.
#____________________________________________________________________________
#
TOPDIR	= ..
include $(TOPDIR)/etc/makeinc
# Local definitions.
# VPATH	= $(LOCALDIR)

LOCALLIBS=	-L$(INSTLIBDIR) -lutils
INCLUDES=       $(I). $(I)$(INSTINCDIR)
CLIBS	=	libnroff.$A
# -lmalloc
# -DDEBUG

HEADERS	=	nroff.def	_nroff.h	nroff.h

UTILS_H		= $(INSTINCDIR)/utils.h

RTFEXES	=	nr2rtf$(EXE)
TEXEXES	=	nr2tex$(EXE)
HTMEXES	=	nr2html$(EXE)	htmlc$(EXE)
MANEXES	=	superman$(EXE)	droff$(EXE)
TOLEXES	=	nrsearch$(EXE)	nrinfo$(EXE)	nrcheck$(EXE)	ntags$(EXE) \
		nrorder$(EXE)	difftags$(EXE)	nrar$(EXE)	hts2html$(EXE) \
		hts$(EXE)
LOCLEXES=	table$(EXE)
ALLEXES	=	$(RTFEXES) $(HTMEXES) $(MANEXES) $(TOLEXES) \
		$(CANEXES) $(TEXEXES) $(LOCLEXES)
#____________________________________________________________________________
#
# Dependencies
#____________________________________________________________________________
#
.SUFFIXES:	.o .c
%.o:%.c
	$(CC) $(CFLAGS) -c $< 
%$(EXE):%.o
	$(CC) $(CFLAGS) -o $* $< $(LDFLAGS) $(CLIBS) $(LOCALLIBS) $(SYSLIBS)
%$(EXE):%.c
	$(CC) $(CFLAGS) -c $< 
	$(CC) $(CFLAGS) -o $* $< $(LDFLAGS) $(CLIBS) $(LOCALLIBS) $(SYSLIBS)
	
all::		$(ALLEXES)

croff.$O:	$(UTILS_H) $(HEADERS)
dbzc.:		$(UTILS_H) $(HEADERS)
difftags.$O:	$(UTILS_H) $(HEADERS)
dman.$O:	$(UTILS_H) $(HEADERS)
droff.$O:	$(UTILS_H) $(HEADERS)
htmlc.$O:	$(UTILS_H) $(HEADERS)
nr2html.$O:	$(UTILS_H) $(HEADERS) html.h
nr2tex.$O:	$(UTILS_H) $(HEADERS)
nr2rtf.$O:	$(UTILS_H) $(HEADERS) rtf.h
nrcheck.$O:	$(UTILS_H) $(HEADERS)
nrinfo.$O:	$(UTILS_H) $(HEADERS)
nrorder.$O:	$(UTILS_H) $(HEADERS)
nrsearch.$O:	$(UTILS_H) $(HEADERS)
ntags.$O:	$(UTILS_H)
superman.$O:	$(UTILS_H)
nrar.$O:	$(UTILS_H)
hts2html.$O:	$(UTILS_H)
$(ALLEXES):	libnroff.$A $(INSTLIBDIR)/libutils.$A

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
install::	$(INSTBINDIR) $(ALLEXES)
		$(INSTALL_EXE) $(RTFEXES) $(INSTBINDIR)
		$(INSTALL_EXE) $(TEXEXES) $(INSTBINDIR)
		$(INSTALL_EXE) $(HTMEXES) $(INSTBINDIR)
		$(INSTALL_EXE) $(MANEXES) $(INSTBINDIR)
		$(INSTALL_EXE) $(TOLEXES) $(INSTBINDIR)

clean::
	$(RM) $(ALLEXES)
	$(RM) *.$O
	$(RM) *.$A
	@echo Cleaned

spotless:: clean
	$(RM) *~
	$(RM) tags

$(INSTBINDIR):
	$(MKDIR) $(INSTBINDIR)
