#!/usr/bin/env bash
pwd
ls
mkdir -p BUILD RPMS SOURCES SPECS SRPMS
cd SOURCES
tar -xaf ../vdb.tar.gz
cp build/vdb.spec ../SPECS
cd ..

rpmbuild -v -bs --clean SPECS/vdb.spec

rpmlint --version
rpm -Vp RPMS/x86_64/vdb*rpm
