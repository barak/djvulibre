%define release 1
%define version 8.01
%define prefix %{?_prefix:%{_prefix}}%{!?_prefix:/usr}
%define mandir %{?_mandir:%{_mandir}}%{!?_mandir:%{prefix}/man}

Prefix: %{prefix}
Summary: Alternate ghostscript interpreter with djvu devices.
Name: gsdjvu
Version: %{version}
Release: %{release}
License: GPL
Group: Applications/Publishing
BuildRoot: %{_tmppath}/%{name}-root
URL: http://djvu.sourceforge.net

Source0: ghostscript-%{version}.tar.bz2
Source1: ghostscript-fonts-std-8.11.tar.bz2
Source2: gsdjvu-1.0.tar.gz
Source3: jpegsrc.v6b.tar.bz2
Source4: libpng-1.2.6.tar.bz2
Source5: zlib-1.1.4.tar.bz2

%description 
The djvulibre program 'djvudigital' attempts to 
locate a ghostscript interpreter that supports the 
djvu output device. This package provides such 
a ghostscript interpreter.

%prep
%setup -n ghostscript-%{version}
%setup -n ghostscript-%{version} -T -D -a 1
%setup -n ghostscript-%{version} -T -D -a 2
%setup -n ghostscript-%{version} -T -D -a 3
%setup -n ghostscript-%{version} -T -D -a 4
%setup -n ghostscript-%{version} -T -D -a 5

%build
gsfiles/gsdjvu_configure.sh
make

%install
rm -rf %{buildroot}
DESTDIR=%{buildroot} gsfiles/gsdjvu_install.sh

%clean
rm -rf %{buildroot}

%files
%defattr(-, root, root)
%doc gsfiles/README
%{_bindir}/*
%{_libdir}/*

%changelog
* Sat Aug 28 2004 Leon Bottou <leon@bottou.org>
- Now uses ghostscript-8.01
* Thu Apr  8 2004 Leon Bottou <leon@bottou.org>
- Initial version using ghostscript-7.07


