Summary: A library of functions for manipulating MNG format files.
Name: libmng
Version: 0.5.2
Release: 1
Copyright: AS IS
Group: System Environment/Libraries
Source0: libmng-0.5.2.tar.gz 
Patch: libmng-0.5.2-rhconf.patch 
URL: http://www.libpng.org/pub/mng/
BuildRoot: /var/tmp/libmng-root
BuildPrereq: libjpeg-devel, zlib-devel, lcms-devel

%description
libmng - library for reading, writing, displaying and examing
Multiple-Image Network Graphics. MNG is the animation extension to the
popular PNG image-format.

%package devel
Summary: Development tools for programs to manipulate MNG format files.
Group: Development/Libraries
Requires: libmng = %{PACKAGE_VERSION}
%description devel
The libmng-devel package contains the header files and static
libraries necessary for developing programs using the MNG
(Multiple-Image Network Graphics) library.

If you want to develop programs which will manipulate MNG image format
files, you should install libmng-devel.  You'll also need to install
the libmng package.

%changelog 
* Sat Jun 24 2000 MATSUURA Takanori <t-matsuu@protein.osaka-u.ac.jp>
- 1st release for RPM

%prep
%setup -n libmng     
ln -s makefiles/makefile.linux Makefile
%patch -p1 -b .rhconf

%build
make RPM_OPT_FLAGS="$RPM_OPT_FLAGS"

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/{lib,include}
make install prefix=$RPM_BUILD_ROOT/usr
strip -R .comments --strip-unneeded $RPM_BUILD_ROOT/usr/lib/libmng.so.0.%{PACKAGE_VERSION}

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root)
%doc changes.readme license.readme readme doc/*
/usr/lib/libmng.so.*

%files devel
%defattr(-,root,root)
/usr/include/*
/usr/lib/libmng.a
/usr/lib/libmng.so

