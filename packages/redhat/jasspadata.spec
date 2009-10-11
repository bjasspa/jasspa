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
#  Last Modified : <091011.2244>
#  Description   : Produces RedHat RPM package for JASSPA MicroEmacs

%define name            me-jasspa-data
%define yymmdd_version  091010
%define version 20%{yymmdd_version}
%define base_ver 20090909

%define release 1

Name: %{name}
Version: %{version}
Release: %{release}
License: GPLv2
BuildArch: noarch
#Copyright: GPLv2
URL: http://www.jasspa.com/
buildroot: %{_tmppath}/%{name}-%{version}-%{release}-root
#BuildRoot: /var/tmp/%{name}-buildroot
Source0: http://www.jasspa.com/release_%{base_ver}/jasspa-metree-%{version}.tar.gz
Source1: http://www.jasspa.com/spelling/ls_dede.tar.gz
Source2: http://www.jasspa.com/spelling/ls_engb.tar.gz
Source3: http://www.jasspa.com/spelling/ls_enus.tar.gz
Source4: http://www.jasspa.com/spelling/ls_eses.tar.gz
Source5: http://www.jasspa.com/spelling/ls_fifi.tar.gz
Source6: http://www.jasspa.com/spelling/ls_frfr.tar.gz
Source7: http://www.jasspa.com/spelling/ls_itit.tar.gz
Source8: http://www.jasspa.com/spelling/ls_plpl.tar.gz
Source9: http://www.jasspa.com/spelling/ls_ptpt.tar.gz
Source10: http://www.jasspa.com/spelling/ls_ruye.tar.gz
Source11: http://www.jasspa.com/spelling/ls_ruyo.tar.gz
Source12: http://www.jasspa.com/release_%{base_ver}/jasspame.pdf
Packager: Jon Green <support@jasspa.com>

Prefix:    %{_prefix}
#
Summary: JASSPA MicroEmacs common macros
Group: Applications/Editors
%description
JASSPA Microemacs common macros
#
%package -n me-jasspa-dede
Summary: JASSPA MicroEmacs German spelling dictionary
Group: Applications/Editors
%description -n me-jasspa-dede
JASSPA MicroEmacs German spelling dictionary
#
%package -n me-jasspa-engb
Summary: JASSPA MicroEmacs English (British) spelling dictionary
Group: Applications/Editors
%description -n me-jasspa-engb
JASSPA MicroEmacs English (British) spelling dictionary
#
%package -n me-jasspa-enus
Summary: JASSPA MicroEmacs English (American) spelling dictionary
Group: Applications/Editors
%description -n me-jasspa-enus
JASSPA MicroEmacs English (American) spelling dictionary
#
%package -n me-jasspa-eses
Summary: JASSPA MicroEmacs Spanish spelling dictionary
Group: Applications/Editors
%description -n me-jasspa-eses
JASSPA MicroEmacs Spanish spelling dictionary
#
%package -n me-jasspa-fifi
Summary: JASSPA MicroEmacs Finnish spelling dictionary
Group: Applications/Editors
%description -n me-jasspa-fifi
JASSPA MicroEmacs Finnish spelling dictionary
#
%package -n me-jasspa-frfr
Summary: JASSPA MicroEmacs French spelling dictionary
Group: Applications/Editors
%description -n me-jasspa-frfr
JASSPA MicroEmacs French spelling dictionary
#
%package -n me-jasspa-itit
Summary: JASSPA MicroEmacs Italian spelling dictionary
Group: Applications/Editors
%description -n me-jasspa-itit
JASSPA MicroEmacs Italian spelling dictionary
#
%package -n me-jasspa-plpl
Summary: JASSPA MicroEmacs Polish spelling dictionary
Group: Applications/Editors
%description -n me-jasspa-plpl
JASSPA MicroEmacs Polish spelling dictionary
#
%package -n me-jasspa-ptpt
Summary: JASSPA MicroEmacs Portuguese spelling dictionary
Group: Applications/Editors
%description -n me-jasspa-ptpt
JASSPA MicroEmacs Portuguese spelling dictionary
#
%package -n me-jasspa-ruye
Summary: JASSPA MicroEmacs Russian (YE) spelling dictionary
Group: Applications/Editors
%description -n me-jasspa-ruye
JASSPA MicroEmacs Russian (YE) spelling dictionary
#
%package -n me-jasspa-ruyo
Summary: JASSPA MicroEmacs Russian (YO) spelling dictionary
Group: Applications/Editors
%description -n me-jasspa-ruyo
JASSPA MicroEmacs Russian (YO) spelling dictionary

%prep
exit 0

%build
exit 0

#me-jasspa
%install
install -d -m 0755 %{buildroot}%{_docdir}/jasspa
install -c -m 0644 %{_sourcedir}/jasspame.pdf %{buildroot}%{_docdir}/jasspa
install -c -m 0644 %{_sourcedir}/change.log %{buildroot}%{_docdir}/jasspa
install -d -m 0755 %{buildroot}%{_datadir}
tar -C %{buildroot}%{_datadir} -zxf %{_sourcedir}/jasspa-metree-%{version}.tar.gz
tar -C %{buildroot}%{_datadir}/jasspa/spelling -zxf %{_sourcedir}/ls_dede.tar.gz
tar -C %{buildroot}%{_datadir}/jasspa/spelling -zxf %{_sourcedir}/ls_engb.tar.gz
tar -C %{buildroot}%{_datadir}/jasspa/spelling -zxf %{_sourcedir}/ls_enus.tar.gz
tar -C %{buildroot}%{_datadir}/jasspa/spelling -zxf %{_sourcedir}/ls_eses.tar.gz
tar -C %{buildroot}%{_datadir}/jasspa/spelling -zxf %{_sourcedir}/ls_fifi.tar.gz
tar -C %{buildroot}%{_datadir}/jasspa/spelling -zxf %{_sourcedir}/ls_frfr.tar.gz
tar -C %{buildroot}%{_datadir}/jasspa/spelling -zxf %{_sourcedir}/ls_itit.tar.gz
tar -C %{buildroot}%{_datadir}/jasspa/spelling -zxf %{_sourcedir}/ls_plpl.tar.gz
tar -C %{buildroot}%{_datadir}/jasspa/spelling -zxf %{_sourcedir}/ls_ptpt.tar.gz
tar -C %{buildroot}%{_datadir}/jasspa/spelling -zxf %{_sourcedir}/ls_ruye.tar.gz
tar -C %{buildroot}%{_datadir}/jasspa/spelling -zxf %{_sourcedir}/ls_ruyo.tar.gz

%clean
echo Removing %{buildroot}
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%{_datadir}/jasspa/company
%{_datadir}/jasspa/contrib
%{_datadir}/jasspa/macros
%{_datadir}/jasspa/pixmaps
%{_datadir}/jasspa/spelling/README.txt
%{_docdir}/jasspa
#
%files -n me-jasspa-dede
%{_datadir}/jasspa/spelling/lsrdede.emf
%{_datadir}/jasspa/spelling/lsdmdede.edf
%{_datadir}/jasspa/spelling/lsdxdede.edf
#
%files -n me-jasspa-engb
%{_datadir}/jasspa/spelling/lsrengb.emf
%{_datadir}/jasspa/spelling/lsdmengb.edf
%{_datadir}/jasspa/spelling/lsdxengb.edf
#
%files -n me-jasspa-enus
%{_datadir}/jasspa/spelling/lsrenus.emf
%{_datadir}/jasspa/spelling/lsdmenus.edf
%{_datadir}/jasspa/spelling/lsdxenus.edf
#
%files -n me-jasspa-eses
%{_datadir}/jasspa/spelling/lsreses.emf
%{_datadir}/jasspa/spelling/lsdmeses.edf
%{_datadir}/jasspa/spelling/COPYING
#
%files -n me-jasspa-fifi
%{_datadir}/jasspa/spelling/lsrfifi.emf
%{_datadir}/jasspa/spelling/lsdmfifi.edf
#
%files -n me-jasspa-frfr
%{_datadir}/jasspa/spelling/lsrfrfr.emf
%{_datadir}/jasspa/spelling/lsdmfrfr.edf
%{_datadir}/jasspa/spelling/COPYING
#
%files -n me-jasspa-itit
%{_datadir}/jasspa/spelling/lsritit.emf
%{_datadir}/jasspa/spelling/lsdmitit.edf
%{_datadir}/jasspa/spelling/COPYING
#
%files -n me-jasspa-plpl
%{_datadir}/jasspa/spelling/lsrplpl.emf
%{_datadir}/jasspa/spelling/lsdmplpl.edf
%{_datadir}/jasspa/spelling/COPYING
#
%files -n me-jasspa-ptpt
%{_datadir}/jasspa/spelling/lsrptpt.emf
%{_datadir}/jasspa/spelling/lsdmptpt.edf
%{_datadir}/jasspa/spelling/COPYING
#
%files -n me-jasspa-ruye
%{_datadir}/jasspa/spelling/lsrruye.emf
%{_datadir}/jasspa/spelling/lsdmruye.edf
#
%files -n me-jasspa-ruyo
%{_datadir}/jasspa/spelling/lsrruyo.emf
%{_datadir}/jasspa/spelling/lsdmruyo.edf

%changelog
* Sun Oct 11 2009 - Jon Green <support@jasspa.com>
- RPM package build for release 20091010
