#____________________________________________________________________________
#
# Last Edited <240509.0021>
#
# Bug makefile.
#____________________________________________________________________________
#

TARGET	=	win32
ROOTDIR	=	..
include	..\etc\win32v16.mak

#____________________________________________________________________________
#
# Local definitions.
#____________________________________________________________________________
#
LOCALLIBS=	$(INSTLIBDIR)\libutils.$A
HEADERS	=	idc.h		list.h		idc.def
UTILS_H	=	$(INSTINCDIR)\utils.h

INCLUDES=	$(I). $(I)$(INSTINCDIR)
EXES	=	idc$(EXE)

SRCS	=	idc.c		line.c		name.c		list.c \
		tek.c		rel.c		pat.c		bug.c \
		faq.c		new.c		inf.c		html.c \
		hml.c		rtf.c
OBJS	=	$(SRCS:.c=.o)

#____________________________________________________________________________
#
# Dependencies
#____________________________________________________________________________
#

all::	$(EXES)

$(EXES):	$(OBJS) $(INSTLIBDIR)\libutils.$(A)
		$(LD) $(LDFLAGS) /out:$@ $(OBJS) $(LOCALLIBS) $(SYSLIBS)

install::	$(EXES)
		- $(MKDIR) $(INSTBINDIR)
		$(INSTALL) $(INSTBINFLAGS) $(EXES) $(INSTBINDIR)

idc.$O:		idc.h
line.$O:	idc.h idc.def
hash.$O:	hash.h
name.$O:	idc.h
list.$O:	list.h
tek.$O:		idc.h
rel.$O:		idc.h
pat.$O:		idc.h
bug.$O:		idc.h
faq.$O:		idc.h
new.$O:		idc.h
inf.$O:		idc.h
idc.h:		$(INSTINCDIR)\utils.h list.h hml.h
