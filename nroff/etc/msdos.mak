#____________________________________________________________________________
#
# Last Edited <060196.1555>
#
# Bug makefile.
#____________________________________________________________________________
#

#
# Compiler related varaibles
#
CC	=	gcc -D_DOS
COFF2EXE=	coff2exe
CEFLAGS	=	-s ${DJDIR}/bin/go32.exe
#CEFLAGS	=	-s d:/gcc/bin/go32.exe
LD	=	$(CC)
#CFLAGS	=	-O -DDEBUG=1
CFLAGS	=	-I. -Wall
LDFLAGS	=	-L.
# -DDEBUG

SYSLIBS	=	-lpc -lm
O	=	o
EXE	=	.exe
ESH	=	.bat
#
# Archive utility
#
AR	=	${DJDIR}/bin/ar
ARFLAGS	=	-rc 

#
# Install Utility
#
INSTALL	=	jinstall -nfd

#
# RCS related variables.
#

CO	=	co
RM	=	jrm

#
# General utilites
#
MKDIR	=	jmkdir
LIBDIR	=	$(ROOTDIR)/lib/$(TARGET)
INCDIR	=	$(ROOTDIR)/include
BINDIR	=	c:\bin
BACKDIR	=	./back.up

#
# Suffix rules.
#
.SUFFIXES:  .exe .386 .$O .i .lst .c
.c.$O:
	$(CC) $(CFLAGS) -c $<
.c.i:
	$(CC) $(CFLAGS) -E -c $< -o $*.i
.386.exe:
	$(COFF2EXE) $(CEFLAGS) $< 
.c.exe:
	$(CC) $(CFLAGS) -o $*.386 $< $(LDFLAGS) $(CLIBS) $(SYSLIBS)
	$(COFF2EXE) $(CEFLAGS) $*.386
	$(RM) $*.386

