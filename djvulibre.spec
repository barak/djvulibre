%define release 3
%define version 3.5.13
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

# BuildRequires: qt-devel
# BuildRequires: qt-designer
# BuildRequires: libjpeg-devel
# BuildRequires: libtiff-devel
# BuildRequires: glibc-devel
# BuildRequires: gcc-c++

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
%setup -q

%build
%configure
make

%install
rm -rf %{buildroot}
make DESTDIR=%{buildroot} install

# Quick fix to stop ldconfig from complaining
find %{buildroot}%{_libdir} -name "*.so*" -exec chmod 755 {} \;

# Quick cleanup of the docs
rm -rf doc/CVS 2>/dev/null || :

# Compute file list
find %{buildroot} -print | \
 sed -e 's:^%{buildroot}::'                \
     -e 's:^%{_mandir}/.*$:#&:'            \
     -e 's:^%{_datadir}/djvu/osi/.*$:#&:'  \
      > files.list

%clean
rm -rf %{buildroot}

%post 
# LIBS: Run ldconfig
/sbin/ldconfig
# MENU: For debian-style menu systems
if test -x /usr/bin/update-menus; then /usr/bin/update-menus; fi
# HACK: Create links to nsdejavu.so in mozilla dirs
( for n in %{prefix}/lib/mozilla*  ; do \
  if [ -d $n ] ; then \
   test -d $n/plugins || mkdir $n/plugins ; \
   test -h $n/plugins/nsdjvu.so && rm $n/plugins/nsdjvu.so ; \
   test -h $n/plugins/nsdejavu.so && rm $n/plugins/nsdejavu.so ; \
   ln -s ../../netscape/plugins/nsdejavu.so $n/plugins/nsdejavu.so ; \
 fi ; done ) 2>/dev/null || true

%postun 
# HACK: Remove links to nsdejavu.so in all mozilla dirs
( if [ ! -r %{prefix}/lib/netscape/plugins/nsdejavu.so ] ; then \
   for n in %{prefix}/lib/mozilla* ; do \
    if [ -h $n/plugins/nsdejavu.so ] ; then \
     rm $n/plugins/nsdejavu.so ; \
     rmdir $n/plugins ; \
  fi ; done ; fi ) 2>/dev/null || true
# MENU: For debian-style menu systems
if test -x /usr/bin/update-menus; then /usr/bin/update-menus; fi
# LIBS: Run ldconfig
/sbin/ldconfig

%files -f files.list
%defattr(-, root, root)
%doc README COPYRIGHT COPYING INSTALL NEWS TODO doc
%{_mandir}/man?
%{_datadir}/djvu/osi/en
%lang(ja) %{_datadir}/djvu/osi/ja*
%lang(fr) %{_datadir}/djvu/osi/fr*
%lang(de) %{_datadir}/djvu/osi/de*
%lang(zh) %{_datadir}/djvu/osi/zh*
%lang(ja) %{_mandir}/ja*
%lang(fr) %{_mandir}/fr*
%lang(de) %{_mandir}/de*

%changelog
* Mon May 31 2004 Leon Bottou <leon@bottou.org> 3.5.13-3
- use libtool
* Wed May  5 2004 Leon Bottou <leon@bottou.org> 3.5.13-2
- own all directories
* Fri Apr  9 2004 Leon Bottou <leon@bottou.org> 3.5.13-1
- release 3.5.13
* Thu Apr  8 2004 Leon Bottou <leon@bottou.org> 3.5.13.0.4
- runtime generation of file list.
* Fri Apr  2 2004 Leon Bottou <leon@bottou.org> 3.5.13.0.3
- use DESTDIR instead of %makeinstall
- make install-desktop-files.
* Tue Nov 18 2003 Leon Bottou <leon@bottou.org> 3.5.13-0.1
- new version
* Wed Nov  5 2003 Leon Bottou <leon@bottou.org> 3.5.12-3
- support for i18n
- rename symlink in mozilla plugin dirs.
* Mon Jul  7 2003 Leon Bottou <leon@bottou.org> 3.5.12-1
- new version
* Thu Apr 24 2003 Leon Bottou <leon@bottou.org> 3.5.11-1
- new version
* Thu Feb  6 2003 Leon Bottou <leon@bottou.org> 3.5.10-2
- new version
* Fri Jan 24 2003 Leon Bottou <leon@bottou.org> 3.5.10-1
- new version
* Wed Oct  9 2002 Leon Bottou <leonb@users.sourceforge.net> 3.5.9-2
- fixed logic for uninstalling nsdejavu links.
- copy stuff from the freshrpms spec file.
* Sun Oct  6 2002 Leon Bottou <leonb@users.sourceforge.net> 3.5.9-1
- added logic to install nsdejavu for mozilla.
* Wed May 29 2002 Leon Bottou <leonb@users.sourceforge.net> 3.5.6-1
- new version
* Mon Apr 1 2002  Leon Bottou <leonb@users.sourceforge.net> 3.5.5-2
- changed group to Applications/Publishing
* Tue Mar 25 2002 Leon Bottou <leonb@users.sourceforge.net> 3.5.5-2
- new version
* Tue Jan 22 2002 Leon Bottou <leonb@users.sourceforge.net> 3.5.4-2
- fixed for properly locating the man directory.
* Wed Jan 16 2002 Leon Bottou <leonb@users.sourceforge.net> 3.5.3-1
- new version
* Fri Dec  7 2001 Leon Bottou <leonb@users.sourceforge.net> 3.5.2-1
- new version
* Wed Dec  5 2001 Leon Bottou <leonb@users.sourceforge.net> 3.5.1-1
- created spec file for rh7.x.

