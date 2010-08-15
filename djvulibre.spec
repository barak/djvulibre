%define release 1
%define version 3.5.23

Summary: DjVu viewers, encoders and utilities.
Name: djvulibre
Version: %{version}
Release: %{release}
License: GPL
Group: Applications/Publishing
Source: djvulibre-%{version}.tar.gz
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
make depend
make

%install
rm -rf %{buildroot}
make DESTDIR=%{buildroot} install

# Quick fix to stop ldconfig from complaining
find %{buildroot}%{_libdir} -name "*.so*" -exec chmod 755 {} \;

# Quick cleanup of the docs
rm -rf doc/CVS 2>/dev/null || :

# Remove symlinks to djview when there are alternatives
if test -x /usr/sbin/update-alternatives ; then
  test -h %{buildroot}%{_bindir}/djview \
    && rm %{buildroot}%{_bindir}/djview
  test -h %{buildroot}%{_mandir}/man1/djview.1 \
    && rm %{buildroot}%{_mandir}/man1/djview.1
fi

%clean
rm -rf %{buildroot}

%post 
# LIBS: Run ldconfig
/sbin/ldconfig
# ALTERNATIVES
if test -x /usr/sbin/update-alternatives ; then
  m1=`ls -1 %{_mandir}/man1/djview3.* | head -1`
  m2=`echo $m1 | sed -e 's/djview3/djview/'`
  /usr/sbin/update-alternatives \
    --install %{_bindir}/djview djview %{_bindir}/djview3 103 \
    --slave $m2 `basename $m2` $m1
fi
# MIME TYPES
test -x /usr/share/djvu/osi/desktop/register-djvu-mime &&
  /usr/share/djvu/osi/desktop/register-djvu-mime install 2>/dev/null
# MENU
test -x /usr/share/djvu/djview3/desktop/register-djview-menu &&
  /usr/share/djvu/djview3/desktop/register-djview-menu install 2>/dev/null
# HACK: Create links to nsdejavu.so in mozilla dirs
( for n in %{_prefix}/lib/mozilla*  ; do \
  if [ -d $n ] ; then \
   test -d $n/plugins || mkdir $n/plugins ; \
   test -h $n/plugins/nsdjvu.so && rm $n/plugins/nsdjvu.so ; \
   test -h $n/plugins/nsdejavu.so && rm $n/plugins/nsdejavu.so ; \
   ln -s ../../netscape/plugins/nsdejavu.so $n/plugins/nsdejavu.so ; \
 fi ; done ) 2>/dev/null || true
exit 0

%preun
if test "$1" = 0 ; then 
 # HACK: Remove links to nsdejavu.so in all mozilla dirs
 ( if [ ! -r %{_prefix}/lib/netscape/plugins/nsdejavu.so ] ; then \
    for n in %{_prefix}/lib/mozilla* ; do \
     if [ -h $n/plugins/nsdejavu.so ] ; then \
      rm $n/plugins/nsdejavu.so ; \
      rmdir $n/plugins ; \
   fi ; done ; fi ) 2>/dev/null || true
 # MENU
 test -x /usr/share/djvu/djview3/desktop/register-djview-menu &&
   /usr/share/djvu/djview3/desktop/register-djview-menu uninstall 2>/dev/null
 # MIME TYPES
 test -x /usr/share/djvu/osi/desktop/register-djvu-mime &&
   /usr/share/djvu/osi/desktop/register-djvu-mime uninstall 2>/dev/null
 # ALTERNATIVES
 if test -x /usr/sbin/update-alternatives ; then
   /usr/sbin/update-alternatives --remove djview %{_bindir}/djview3
 fi
fi
exit 0


%postun
# LIBS: Run ldconfig
/sbin/ldconfig
exit 0


%files
%defattr(-, root, root)
%doc README COPYRIGHT COPYING INSTALL NEWS TODO doc
%{_bindir}
%{_libdir}
%{_includedir}/libdjvu
%{_datadir}/djvu
%{_mandir}

%changelog
* Sat Jan 15 2007 Leon Bottou <leon@bottou.org> 3.5.18-2
- changed postun as preun
* Tue Jan 15 2007 Leon Bottou <leon@bottou.org> 3.5.18-1
- Use xdg scripts to install mime types and menu entries
- Updated to 3.5.18-1
* Mon Oct 31 2005 Leon Bottou <leon@bottou.org> 3.5.16-1
- Updated to 3.5.16-1
* Wed Jul  6 2005 Leon Bottou <leon@bottou.org> 3.5.15-1
- Updated to 3.5.15-1
* Thu Jun 17 2004 Leon Bottou <leon@bottou.org> 3.5.13-4
- changed runtime generation of file list.
* Mon May 31 2004 Leon Bottou <leon@bottou.org> 3.5.13-3
- removed 'make install-desktop-files'
* Wed May  5 2004 Leon Bottou <leon@bottou.org> 3.5.13-2
* Fri Apr  9 2004 Leon Bottou <leon@bottou.org> 3.5.13-1
- added runtime generation of file list.
- using DESTDIR instead of %makeinstall.
- added 'make install-desktop-files'
* Wed Nov  5 2003 Leon Bottou <leon@bottou.org> 3.5.12-3
- added support for i18n
- renamed symlink in mozilla plugin dirs.
* Mon Jul  7 2003 Leon Bottou <leon@bottou.org> 3.5.12-1
* Thu Apr 24 2003 Leon Bottou <leon@bottou.org> 3.5.11-1
* Thu Feb  6 2003 Leon Bottou <leon@bottou.org> 3.5.10-2
* Fri Jan 24 2003 Leon Bottou <leon@bottou.org> 3.5.10-1
* Wed Oct  9 2002 Leon Bottou <leonb@users.sourceforge.net> 3.5.9-2
- fixed logic for uninstalling nsdejavu links.
- learned a few tricks from the freshrpms spec file.
* Sun Oct  6 2002 Leon Bottou <leonb@users.sourceforge.net> 3.5.9-1
- added logic to install nsdejavu for mozilla.
* Wed May 29 2002 Leon Bottou <leonb@users.sourceforge.net> 3.5.6-1
* Mon Apr 1 2002  Leon Bottou <leonb@users.sourceforge.net> 3.5.5-2
- changed group to Applications/Publishing.
* Tue Mar 25 2002 Leon Bottou <leonb@users.sourceforge.net> 3.5.5-2
* Tue Jan 22 2002 Leon Bottou <leonb@users.sourceforge.net> 3.5.4-2
- added macros to locate man directory.
* Wed Jan 16 2002 Leon Bottou <leonb@users.sourceforge.net> 3.5.3-1
* Fri Dec  7 2001 Leon Bottou <leonb@users.sourceforge.net> 3.5.2-1
* Wed Dec  5 2001 Leon Bottou <leonb@users.sourceforge.net> 3.5.1-1
- created initial file.

