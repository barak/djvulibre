%define release 1
%define version 3.5.9
%define prefix %{?_prefix:%{_prefix}}%{!?_prefix:/usr}
%define mandir %{?_mandir:%{_mandir}}%{!?_mandir:%{prefix}/man}

Prefix: %{prefix}
Summary: DjVu viewers, encoders and utilities.
Name: djvulibre
Version: %{version}
Release: %{release}
Copyright: GPL
Group: Applications/Publishing
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

%changelog
* Sun Oct  6 2002 Leon Bottou <leonb@users.sourceforge.net> 3.5.9-1
- version 3.5.9-1
- added logic to 
* Wed May 29 2002 Leon Bottou <leonb@users.sourceforge.net> 3.5.6-1
- bumped to version 3.5.6-1
* Mon Apr 1 2002  Leon Bottou <leonb@users.sourceforge.net> 3.5.5-2
- bumped to version 3.5.5-2
- changed group to Applications/Publishing
* Tue Mar 25 2002 Leon Bottou <leonb@users.sourceforge.net> 3.5.5-2
- bumped to version 3.5.5-2
* Tue Jan 22 2002 Leon Bottou <leonb@users.sourceforge.net> 3.5.4-2
- bumped to version 3.5.4-1.
- fixed for properly locating the man directory.
- bumped to version 3.5.4-2.
* Wed Jan 16 2002 Leon Bottou <leonb@users.sourceforge.net> 3.5.3-1
- bumped to version 3.5.3-1
* Fri Dec  7 2001 Leon Bottou <leonb@users.sourceforge.net> 3.5.2-1
- bumped to version 3.5.2-1.
* Wed Dec  5 2001 Leon Bottou <leonb@users.sourceforge.net> 3.5.1-1
- created spec file for rh7.x.

%prep
%setup

%build
./configure --prefix=%{prefix} --mandir=%{mandir}
make

%install
rm -rf $RPM_BUILD_ROOT
install -d $RPM_BUILD_ROOT
make prefix="$RPM_BUILD_ROOT"%{prefix} \
     mandir="$RPM_BUILD_ROOT"%{mandir} \
     install
mkdir "$RPM_BUILD_ROOT"%{prefix}/lib/mozilla
mkdir "$RPM_BUILD_ROOT"%{prefix}/lib/mozilla/plugins
( cd "$RPM_BUILD_ROOT"%{prefix}/lib/mozilla/plugins ; \
  ln -s ../../netscape/plugins/nsdejavu.so . )

%clean
rm -rf $RPM_BUILD_ROOT

%post 
/sbin/ldconfig
# Create links to nsdejavu.so in mozilla dirs
( for n in %{prefix}/lib/mozilla*  ; do \
  if [ -d $n ] ; then \
   test -d $n/plugins || mkdir $n/plugins ; \
   ln -s %{prefix}/lib/netscape/plugins/nsdejavu.so $n/plugins ; \
 fi ; done ) 2>/dev/null || true

%postun 
/sbin/ldconfig
# Remove links to nsdejavu.so in all mozilla dirs
( for n in %{prefix}/lib/mozilla* ; do \
  if [ -h $n/plugins/nsdejavu.so ] ; then \
   rm $n/plugins/nsdejavu.so ; \
   rmdir $n/plugins ; \
 fi ; done ) 2>/dev/null || true

%files
%defattr(-, root, root)
%doc README COPYRIGHT COPYING INSTALL NEWS TODO
%doc doc/*
%{prefix}/bin
%{prefix}/lib
%{prefix}/share/djvu
%{mandir}
