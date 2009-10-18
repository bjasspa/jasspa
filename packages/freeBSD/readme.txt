FreeBSD files

Firstly you need to install "ports" which provides the build environment.

Then create a subdirectory in "editors" called "me-jasspa"

Then install the package files.

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

pkg_add me-jasspa-2009.10.11.tbz

To subsequently deinstall

pkg_delete me-jasspa-2009.10.11





