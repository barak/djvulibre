%define prefix /usr

Prefix: %{prefix}
Summary: DjVu viewers, encoders and utilities.
Name: djvulibre
Version: 3.5.1
Release: 1
Copyright: GPL
Group: Applications/Graphics
Source: http://prdownloads.sourceforge.net/djvu/djvulibre-3.5.1.tar.gz
BuildRoot: /var/tmp/djvulibre-root
URL: http://djvu.sourceforge.net

%description

%changelog
* Wed Dec  5 2001 Leon Bottou <leonb@users.sourceforge.net>
- created spec file

%prep
%setup

%build
configure --prefix=%{prefix}
make

%install
rm -rf $RPM_BUILD_ROOT
install -d $RPM_BUILD_ROOT
make prefix="$RPM_BUILD_ROOT"%{prefix} install
install -d $RPM_BUILD_ROOT%{prefix}
( cd $RPM_BUILD_ROOT%{prefix}/lib/mozilla/plugins ; \
  ln -s ../../netscape/plugins/nsdejavu.so . )

%clean
rm -rf $RPM_BUILD_ROOT

%post 
/sbin/ldconfig

%postun 
/sbin/ldconfig

%files
%defattr(-, root, root)
%doc README COPYRIGHT COPYING INSTALL TODO
%doc doc/*
%{prefix}/bin
%{prefix}/lib
%{prefix}/man
%{prefix}/share
