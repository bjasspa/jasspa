%define name            jasspa-me
%define yymmdd_version  040206
%define version 20%{yymmdd_version}

%define release 1

Summary: A much enhanced version of Daniel Lawrence's original MicroEmacs 3.8 of 1988
Name: %{name}
Version: %{version}
Release: %{release}
Copyright: GPL
Group: Applications/Editors
URL: http://www.jasspa.com/
buildroot: %{_tmppath}/%{name}-%{version}-%{release}-root
#BuildRoot: /var/tmp/%{name}-buildroot
Source0: %{name}src-%{version}.tar.gz
Source1: %{name}tree-%{version}.tar.gz
Packager: Jon Green <support@jasspa.com>

Prefix:    %{_prefix}

%description
JASSPA MicroEmacs is an enhanced version of Daniel Lawrence's original
MicroEmacs 3.8. It has a small memory and disk footprint while still providing
most of the useful Emacs functionality. MicroEmacs includes X-window support,
an integrated spell-checker, macro language, major modes for most languages,
color syntax highlighting in X and console modes, online help, file browser,
and much more !

%prep
# Only unpack the source directory. 
%setup -a 0 -n me%{yymmdd_version}

%build
(cd ./src; sh ./build)

%install
install -d -m 0755 %{buildroot}%{_bindir}
install -d -m 0755 %{buildroot}%{_datadir}
tar -C %{buildroot}%{_datadir} -zxf %{_sourcedir}/%{name}tree-%{version}.tar.gz
install -c -m 0755 ./src/me %{buildroot}%{_bindir}

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%{_bindir}/me
%{_datadir}/jasspa
# %dir /usr/share/doc/microemacs-jasspa
# %doc /usr/share/doc/microemacs-jasspa/readme.txt
%changelog
