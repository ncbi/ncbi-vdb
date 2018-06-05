#!/usr/bin/env bash
pwd
mkdir  BUILD RPMS SOURCES SPECS SRPMS
cd SOURCES
cp ../../vbd.tar.gz .
cd ..

rpmbuild -v -bs --clean SPECS/vdb.spec

rpm -Vp RPMS/x86_64/vdb*rpm
