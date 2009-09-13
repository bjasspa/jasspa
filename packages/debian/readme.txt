> -!- document -!-
>
>  Copyright 2009 Jon Green.
>
>  Created By    : Jon Green
>  Created       : Sun Sep 13 18:09:55 2009
>  Last Modified : <090913.1959>
>

Follows are some notes on building the Debian packages for JASSPA  MicroEmacs.
The notes were compiled when building on Ubuntu 9.04 (jaunty)

The Debian package script was created and evolved by the following people

  Patrick Das Gupta - 2005 (Original)
  Christof Boeckler - 2006 (Maintained)
  Jon Green - 2009         (Updated for spelling + /use/share/jasspa)

There are two basic Debian  package  files that are supplied  that contain the
rules to build the Debian packages:-

  me-jasspa.tar.gz       - Executable only (Architecture dependent)
  me-jasspa-data.tar.gz  - JASSPA tree + spelling (Any architecture)
  
Firstly unpack the files:

  % tar zxvf me-jasspa-0.0.20090909.tar.gz
  % tar zxvf me-jasspa-data-0.0.20090909.tar.gz
  
Rename the directories to the name of the new release i.e.

  % mv me-jasspa-0.0.20090909      me-jasspa-0.0.yyyymmdd
  % mv me-jasspa-data-0.0.20090909 me-jasspa-data-0.0.yyyymmdd
  
Edit the me-jasspa(-data)-0.0.20090909/debian/rules files to match the name of
the current release that you are packaging.

  edit line 5: "UPSTREAM_VERSION=090909" <= "UPSTREAM_VERSION=yymmdd"
  
From directory  me-jasspa(-data)-0.0.20090909  issue the following  command to
get the source files and to build the xxx.orig.tar.gz package:

  % make -f debian/rules get-orig-source

From directory  me-jasspa(-data)-0.0.20090909 build the packages

  # Build the binary package
  % dpkg-buildpackage -rfakeroot -tc 
  # Build the source package
  % dpkg-buildpackage -S -rfakeroot -tc

Check  that the Debian  script  file is OK from the parent  directory  that is
holding me-jasspa(-data)-0.0.20090909:-

  lintian -i me-jasspa-0.0.20090909_386.changes

Finished.
