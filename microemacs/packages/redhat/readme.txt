> -!- document; fill-column:78; fill-mode:both; -!-
>
>  Copyright 2004 JASSPA.
>
>  Created By    : Jon Green
>  Created       : Fri Feb 6 22:33:31 2004
>  Last Modified : <040317.2129>
>

Building the RedHat RPM
-----------------------

    To build a RedHat RPM in user mode then the RedHat installation  directory
    has to be set up.

Create a artificial home for user build
---------------------------------------

    A top level file called  ".rpmmacros" is created, this tells the build the
    new directory path information. The contents of this file are:-

    > %_topdir %(echo $HOME)/rpmbuild

Create the directory structure
------------------------------

    Create the standard RedHat directory structure:-

    >   ~ +- .rpmmacros
    >     +-+ rpmbuild
    >       +- BUILD
    >       +-+ RPMS
    >       | +- athlon
    >       | +- i386
    >       | +- i486
    >       | +- i586
    >       | +- i686
    >       | +- noarch
    >       +- SOURCES
    >       +- SPECS
    >       +- SRPMS

Copy the information into the directory
---------------------------------------

    Copy the source files into the tree for build:-

    > jasspa.spec                   -> $(HOME)/rpmbuild/SPECS/
    > jasspa-mesrc-20040301.tar.gz  -> $(HOME)/rpmbuild/SOURCES
    > jasspa-metree-20040301.tar.gz -> $(HOME)/rpmbuild/SOURCES/

Build the RPM
-------------

    The RPM is build using the command "rpmbuild" the resultant output is
    generated as follows:-

    >% $(HOME)/rpmbuild/RPMS/i386/jasspa-me-20040301-1.i386.rpm
    >% $(HOME)/rpmbuild/SRPMS/jasspa-me-20040301-1.src.rpm

To install the RPM
------------------

    As root:

    >% rpm -i jasspa-me-20040301-1.i386.rpm

To query what is installed
--------------------------

    >% rpm -q jasspa-me

To remove the RPM
-----------------

    >% rpm -e jasspa-me
