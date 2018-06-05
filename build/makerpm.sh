#!/usr/bin/env bash
pwd
ls
mkdir -p BUILD RPMS SOURCES SPECS SRPMS
cd SOURCES
tar -xavf ../vdb.tar.gz
cp build/vdb.spec ../SPECS
cd ..

rpmbuild -v -bs --clean SPECS/vdb.spec

rpmlint 
rpm -Vp RPMS/x86_64/vdb*rpm
