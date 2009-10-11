# JASSPA MicroEmacs - www.jasspa.com
# jasspa.spec - RedHat package specification for JASSPA MicroEmacs
#
# Copyright (C) 2003-2006 JASSPA (www.jasspa.com)
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 675 Mass Ave, Cambridge, MA 02139, USA.

#  System        : MicroEmacs
#  Module        : Package File.
#  Created By    : Jon Green
#  Created       : Sun Aug 17 12:58:23 2003
#  Last Modified : <091011.2047>
#  Description   : Produces RedHat RPM package for JASSPA MicroEmacs

%define name            me-jasspa
%define yymmdd_version  091010
%define version 20%{yymmdd_version}
%define base_ver 20090909

%define release 1

Name: %{name}
Version: %{version}
Release: %{release}
License: GPLv2
#Copyright: GPLv2
URL: http://www.jasspa.com/
buildroot: %{_tmppath}/%{name}-%{version}-%{release}-root
#BuildRoot: /var/tmp/%{name}-buildroot
Source0: http://www.jasspa.com/release_%{base_ver}/jasspa-mesrc-%{version}.tar.gz
Source1: http://www.jasspa.com/release_%{base_ver}/ne.1.gz
Source2: http://www.jasspa.com/release_%{base_ver}/me.1.gz
Source3: http://www.jasspa.com/release_%{base_ver}/jasspa-microemacs.desktop
Packager: Jon Green <support@jasspa.com>
BuildRequires: gcc
#BuildRequires: libx11-devel
#BuildRequires: xpm-devel
#BuildRequires: libxorg-x11-devel
BuildRequires: ncurses-devel

Prefix:    %{_prefix}

Summary: A much enhanced version of Daniel Lawrence's original MicroEmacs 3.8 of 1988
Group: Applications/Editors
%description
JASSPA MicroEmacs is an enhanced version of Daniel Lawrence's original
MicroEmacs 3.8. It has a small memory and disk footprint while still providing
most of the useful Emacs functionality. MicroEmacs includes X-window support,
an integrated spell-checker, macro language, major modes for most languages,
color syntax highlighting in X and console modes, online help, file browser,
and much more!

%package -n me-jasspa-nox
Summary: JASSPA MicroEmacs Console
Group: Applications/Editors
%description -n me-jasspa-nox
The console version of JASSPA MircoEmacs an enhanced version of Daniel
Lawrence's original MicroEmacs 3.8. It has a small memory and disk footprint
while still providing most of the useful Emacs functionality. MicroEmacs includes
an integrated spell-checker, macro language, major modes for most languages,
color syntax highlighting in X and console modes, online help, file browser,
and much more!

%package -n ne-jasspa
Summary: JASSPA NanoEmacs
Group: Applications/Editors
%description -n ne-jasspa
A minimal tiny editor version of JASSPA MircoEmacs. NanoEmacs provides the
very minimal functionality of an Emacs type editor.

%prep
# Only unpack the source directory.
%setup -a 0 -n me%{yymmdd_version}

%build
(cd ./src; sh ./build -t cw)
(cd ./src; sh ./build -ne)
(cd ./src; sh ./build -t c)

#me-jasspa
%install
install -d -m 0755 %{buildroot}%{_docdir}/jasspa
install -c -m 0644 %{_sourcedir}/change.log %{buildroot}%{_docdir}/jasspa
install -d -m 0755 %{buildroot}%{_bindir}
install -c -m 0755 ./src/mecw %{buildroot}%{_bindir}/me
install -c -m 0755 ./src/mec %{buildroot}%{_bindir}
install -c -m 0755 ./src/ne %{buildroot}%{_bindir}
install -d -m 0755 %{buildroot}%{_mandir}/man1
install -c -m 0644 %{_sourcedir}/ne.1.gz %{buildroot}%{_mandir}/man1
install -c -m 0644 %{_sourcedir}/me.1.gz %{buildroot}%{_mandir}/man1
install -d -m 0755 %{buildroot}%{_datadir}/applications
install -c -m 0644 %{_sourcedir}/jasspa-microemacs.desktop %{buildroot}%{_datadir}/applications
#
# For the console then create a symbolic link to "me" if it does not exist.
#
%post -n me-jasspa-nox
if [ ! -f %{_bindir}/me ] ; then 
    ln -s %{_bindir}/mec %{_bindir}/me
fi  
%postun -n me-jasspa-nox
if [ -h %{_bindir}/me ] ; then
    rm -f %{_bindir}/me
fi

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%{_bindir}/me
%{_mandir}/man1/me.1.gz
%{_docdir}/jasspa
%{_datadir}/applications

%files  -n me-jasspa-nox
%defattr(-,root,root)
%{_bindir}/mec
%{_mandir}/man1
%{_docdir}/jasspa

%files -n ne-jasspa
%defattr(-,root,root)
%{_bindir}/ne
%{_mandir}/man1/ne.1.gz

%changelog
* Sun Oct 11 2009 - Jon Green <support@jasspa.com>
- RPM package build for release 20091010
