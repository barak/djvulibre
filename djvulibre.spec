%define prefix /usr
%define version 3.5.2
%define release 1

Prefix: %{prefix}
Summary: DjVu viewers, encoders and utilities.
Name: djvulibre
Version: %{version}
Release: %{release}
Copyright: GPL
Group: Applications/Graphics
Source: http://prdownloads.sourceforge.net/djvu/djvulibre-%{version}.tar.gz
BuildRoot: /var/tmp/djvulibre-root
URL: http://djvu.sourceforge.net

%description 

DjVu is a web-centric format and software platform for distributing documents
and images.  DjVu content downloads faster, displays and renders faster, looks
nicer on a screen, and consume less client resources than competing formats.
DjVu was originally developped at AT&T Labs-Research by Leon Bottou, Yann
LeCun, Patrick Haffner, and many others.  In March 2000, AT&T sold DjVu to
LizardTech Inc. who now distributes Windows/Mac plug-ins, and commercial
encoders (mostly on Windows)

In an effort to promote DjVu as a Web standard, LizardTech's management was
enlightened enough to release the reference implementation of DjVu under the
GNU GPL in October 2000.  DjVuLibre (which means free DjVu), is an enhanced
version of that code maintained by the original inventors of DjVu. It is
compatible with version 3.5 of LizardTech's DjVu software suite.

DjVulibre-3.5 contains:
- a standalone DjVu viewer based on the Qt library. 
- A browser plugin that works with most Unix browsers.
- A full-fledged wavelet-based compressor for pictures. 
- A simple compressor for bitonal (black and white) scanned pages. 
- A compressor for palettized images (a la GIF/PNG). 
- A set of utilities to manipulate and assemble DjVu images and documents. 
- A set of decoders to convert DjVu to a number of other formats. 
- An up-to-date version of the C++ DjVu Reference Library.

%changelog
* Wed Dec  5 2001 Leon Bottou <leonb@users.sourceforge.net>
- created spec file for rh7.x
* Fri Dec  7 2001 Leon Bottou <leonb@users.sourceforge.net>
- bumped to version 3.5.2

%prep
%setup

%build
./configure --prefix=%{prefix}
make

%install
rm -rf $RPM_BUILD_ROOT
install -d $RPM_BUILD_ROOT
make prefix="$RPM_BUILD_ROOT"%{prefix} install
install -d $RPM_BUILD_ROOT%{prefix}/lib/mozilla/plugins
cd $RPM_BUILD_ROOT%{prefix}/lib/mozilla/plugins && ln -s ../../netscape/plugins/nsdejavu.so .

%clean
rm -rf $RPM_BUILD_ROOT

%post 
/sbin/ldconfig

%postun 
/sbin/ldconfig

%files
%defattr(-, root, root)
%doc README COPYRIGHT COPYING INSTALL NEWS TODO
%doc doc/*
%{prefix}/bin
%{prefix}/lib
%{prefix}/man
%{prefix}/share
