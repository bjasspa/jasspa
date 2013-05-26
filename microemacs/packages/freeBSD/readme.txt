> -!- document -!-
>
>  Copyright 2009 Jon Green.
>
>  Created By    : Jon Green
>  Created       : Thu Oct 22 16:20:28 2009
>  Last Modified : <091022.1626>
>

FreeBSD files
=============

Firstly you need to install "ports" which provides the build environment.

Then change to the subdirectory in "editors".

Then unpack the the package files in the "/usr/ports/editors"

Then build the required subsystems i.e.

    % cd /usr/ports/editors
    % cd me-jasspa
    % make install
    % cd ../me-jasspa-data
    % make install

Build Notes
===========

To install to a temporary location to create the package list:

#!/bin/sh
mkdir /var/tmp/$(make -V PORTNAME)
mtree -U -f $(make -V MTREE_FILE) -d -e -p /var/tmp/$(make -V PORTNAME)
make depends PREFIX=/var/tmp/$(make -V PORTNAME)
(cd /var/tmp/$(make -V PORTNAME) && find -d * -type d) | sort > OLD-DIRS
:>pkg-plist
make install PREFIX=/var/tmp/$(make -V PORTNAME)
(cd /var/tmp/$(make -V PORTNAME) && find -d * \! -type d) | sort > pkg-plist
(cd /var/tmp/$(make -V PORTNAME) && find -d * -type d) | sort | comm -13 OLD-DIRS - | sort -r | sed -e 's#^#@dirrm #' >> pkg-plist

Once the package is created and is OK then as root run:

    % make install

Once installed then create the package file:

    % pkg_create -b me-jasspa-2009.10.11

To deinstall the package:

    % make deinstall

To install from a package file.

    % pkg_add me-jasspa-2009.10.11.tbz

To subsequently deinstall

    % pkg_delete me-jasspa-2009.10.11
