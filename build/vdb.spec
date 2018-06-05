%define _topdir /tmp
%define version 3.0.0
%define name vdb
%define buildroot %{_topdir}/%{name}-%{version}-root

BuildRoot: %{buildroot}
Summary: NCBI VDB
License: Public Domain
Version: %{version}
Release: %{version}
Copyright: Public Domain
Group: Development/Tools
Source: https://github.com/ncbi/ncbi-vdb
URL: https://github.com/ncbi/ncbi-vdb

%description
The SRA Toolkit and SDK from NCBI is a collection of tools and libraries 
for using data in the INSDC Sequence Read Archives.

%prep
tar -xaf vdb.tar.gz

%setup -q

%build
./configure --without-debug
make

%install
rm -rf $RPM_BUILD_ROOT
make INSTALL_ROOT=$RPM_BUILD_ROOT install

%clean
make clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{directory}/lib
%{directory}/bin
%{directory}/include
%{directory}/man/man1
%{directory}/man/man3
%{directory}/man/mann

