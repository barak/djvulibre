%define release 1
%define version 3.5.10
%define prefix %{?_prefix:%{_prefix}}%{!?_prefix:/usr}
%define mandir %{?_mandir:%{_mandir}}%{!?_mandir:%{prefix}/man}

Prefix: %{prefix}
Summary: DjVu viewers, encoders and utilities.
Name: djvulibre
Version: %{version}
Release: %{release}
License: GPL
Group: Applications/Publishing
Source: http://prdownloads.sourceforge.net/djvu/djvulibre-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-root
URL: http://djvu.sourceforge.net

#These are RH specific:
# Requires: qt, libstdc++
# BuildRequires: qt-devel, libstdc++-devel

%description 

DjVu is a web-centric format and software platform for distributing documents
and images.  DjVu content downloads faster, displays and renders faster, looks
nicer on a screen, and consume less client resources than competing formats.
DjVu was originally developed at AT&T Labs-Research by Leon Bottou, Yann
LeCun, Patrick Haffner, and many others.  In March 2000, AT&T sold DjVu to
LizardTech Inc. who now distributes Windows/Mac plug-ins, and commercial
encoders (mostly on Windows)

In an effort to promote DjVu as a Web standard, the LizardTech management was
enlightened enough to release the reference implementation of DjVu under the
GNU GPL in October 2000.  DjVuLibre (which means free DjVu), is an enhanced
version of that code maintained by the original inventors of DjVu. It is
compatible with version 3.5 of the LizardTech DjVu software suite.

DjVulibre-3.5 contains:
- a standalone DjVu viewer based on the Qt library. 
- A browser plugin that works with most Unix browsers.
- A full-fledged wavelet-based compressor for pictures. 
- A simple compressor for bitonal (black and white) scanned pages. 
- A compressor for palettized images (a la GIF/PNG). 
- A set of utilities to manipulate and assemble DjVu images and documents. 
- A set of decoders to convert DjVu to a number of other formats. 
- An up-to-date version of the C++ DjVu Reference Library.

%prep
%setup -q -n djvulibre-3.5

%build
%configure
make

%install
rm -rf %{buildroot}
%makeinstall
# Quick fix to stop ldconfig from complaining
find %{buildroot}%{_libdir} -name "*.so*" -exec chmod 755 {} \;
# Quick cleanup of the docs
rm -rf doc/CVS 2>/dev/null || :

%clean
rm -rf %{buildroot}

%post 
/sbin/ldconfig
# Create links to nsdejavu.so in mozilla dirs
# The name of the link is nsdjvu.so in order to make
# sure they do not get removed when upgrading rpms.
( for n in %{prefix}/lib/mozilla*  ; do \
  if [ -d $n ] ; then \
   test -d $n/plugins || mkdir $n/plugins ; \
   ln -s ../../netscape/plugins/nsdejavu.so $n/plugins/nsdjvu.so ; \
 fi ; done ) 2>/dev/null || true

%postun 
/sbin/ldconfig
# Remove links to nsdejavu.so in all mozilla dirs
( if ! [ -r %{prefix}/lib/netscape/plugins/nsdejavu.so ] ; then \
   for n in %{prefix}/lib/mozilla* ; do \
    if [ -h $n/plugins/nsdjvu.so ] ; then \
     rm $n/plugins/nsdjvu.so ; \
     rmdir $n/plugins ; \
  fi ; done ; fi ) 2>/dev/null || true

%files
%defattr(-, root, root)
%doc README COPYRIGHT COPYING INSTALL NEWS TODO doc
%{_bindir}/*
%{_libdir}/*.so*
%{_libdir}/*/plugins/*.so*
%{_datadir}/djvu
%{_mandir}/man?/*

%changelog
* Fri Jan 24 2003 Leon Bottou <leon@bottou.org> 3.5.10-1
- prepared for version 3.5.10
- added option -n in setup.
* Wed Oct  9 2002 Leon Bottou <leonb@users.sourceforge.net> 3.5.9-2
- fixed logic for uninstalling nsdejavu links.
- copy stuff from the freshrpms spec file.
* Sun Oct  6 2002 Leon Bottou <leonb@users.sourceforge.net> 3.5.9-1
- added logic to install nsdejavu for mozilla.
* Wed May 29 2002 Leon Bottou <leonb@users.sourceforge.net> 3.5.6-1
- bumped to version 3.5.6-1
* Mon Apr 1 2002  Leon Bottou <leonb@users.sourceforge.net> 3.5.5-2
- changed group to Applications/Publishing
* Tue Mar 25 2002 Leon Bottou <leonb@users.sourceforge.net> 3.5.5-2
* Tue Jan 22 2002 Leon Bottou <leonb@users.sourceforge.net> 3.5.4-2
- fixed for properly locating the man directory.
* Wed Jan 16 2002 Leon Bottou <leonb@users.sourceforge.net> 3.5.3-1
* Fri Dec  7 2001 Leon Bottou <leonb@users.sourceforge.net> 3.5.2-1
* Wed Dec  5 2001 Leon Bottou <leonb@users.sourceforge.net> 3.5.1-1
- created spec file for rh7.x.

