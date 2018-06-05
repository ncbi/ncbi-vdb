%define _topdir /tmp/vdb
%define version 3.0.0
%define name vdb
%define buildroot %{_topdir}/%{name}-%{version}-root


Name: %{name}
Version: %{version}
Release: 1%{?dist}
Summary: NCBI VDB
BuildRequires: bison,flex,libxml2,gcc-c++
Requires: libxml2,zlib
BuildArch: x86_64
License: Public Domain
Packager: mike.vartanian@nih.gov
Source: https://github.com/ncbi/ncbi-vdb
URL: https://github.com/ncbi/ncbi-vdb

%description
The SRA Toolkit and SDK from NCBI is a collection of tools and libraries 
for using data in the INSDC Sequence Read Archives.

%prep
cd $RPM_BUILD_ROOT
rm -rf vdb*
tar -xaf vdb.tar.gz

%build
./configure --without-debug
make

%install
mkdir -p %{buildroot}/{%_bindir}
mkdir -p %{buildroot}/ncbi-outdir

# %{_bindir} is /usr/bin
%make_install
make INSTALL_ROOT=$RPM_BUILD_ROOT install
ls -lR ${buildroot}/ncbi-outdir

%files
%license LICENSE
%defattr(-,root,root)
%{_bindir}/
