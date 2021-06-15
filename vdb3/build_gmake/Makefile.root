# ===========================================================================
#
#                            PUBLIC DOMAIN NOTICE
#               National Center for Biotechnology Information
#
#  This software/database is a "United States Government Work" under the
#  terms of the United States Copyright Act.  It was written as part of
#  the author's official duties as a United States Government employee and
#  thus cannot be copyrighted.  This software/database is freely available
#  to the public for use. The National Library of Medicine and the U.S.
#  Government have not placed any restriction on its use or reproduction.
#
#  Although all reasonable efforts have been taken to ensure the accuracy
#  and reliability of the software and data, the NLM and the U.S.
#  Government do not and cannot warrant the performance or results that
#  may be obtained by using this software or data. The NLM and the U.S.
#  Government disclaim all warranties, express or implied, including
#  warranties of performance, merchantability or fitness for any particular
#  purpose.
#
#  Please cite the author in any work or product based on this material.
#
# ===========================================================================

#
# Makefile for VDB3
#

#-------------------------------------------------------------------------------
# targets
#
# help 		- list targets
# build		- build (default)
# config 	- display(/set?) configuration
# clean 	- remove build outputs
# test, check	- run tests
# release, debug	- switch between builds
# out 		- change output directory
# install 	- install on the development system (may require sudo)
# uninstall - uninstall from the development system (may require sudo)
# package 	- create a Docker container with a full installation
# docs 		- generate Doxygen

#-------------------------------------------------------------------------------
# default
#
default: build

#-------------------------------------------------------------------------------
# environment
#
TOP ?= $(abspath .)
SUBDIRS = build platform db services tools tests

ifeq (no, $(shell test -f $(TOP)/Makefile.config && echo yes || echo no))
# handle missing Makefile.config
# by default, create a Debug build, output to ~/ncbi-outdir/vdb3
DEFAULT_BUILD  = dbg
DEFAULT_OUTDIR = $(wildcard ~)/ncbi-outdir/vdb3
$(info $(TOP)/Makefile.config is not found, creating...)
$(info   current build is $(DEFAULT_BUILD))
$(info   output target directory is $(DEFAULT_OUTDIR))
$(shell echo "BUILD=$(DEFAULT_BUILD)" >$(TOP)/Makefile.config)
$(shell echo "OUTDIR=$(DEFAULT_OUTDIR)" >>$(TOP)/Makefile.config)
endif

include $(TOP)/build/Makefile.env

#-------------------------------------------------------------------------------
# help
#
help: stdhelp
	@echo "    release  - switch to a release build"
	@echo "    debug    - switch to a debug build"
	@echo "    out OUTDIR=<path> - switch to a different build output directory"
	@echo "    docs     - generate Doxygen documentation"
	@echo "    install  - install on the development system (may require sudo)"
	@echo "    uninstall - uninstall from the development system (may require sudo)"
	@echo "    package  - create a Docker container with the full installation"

#-------------------------------------------------------------------------------
# clean
#
clean:
	@ rm -rf $(OUTDIR)/$(BUILD)

#-------------------------------------------------------------------------------
# configuration change
#
release:
	@ sed -i -e 's/BUILD.*dbg/BUILD=rel/' $(CONFIG_FILE)
	@ $(MAKE) --no-print-directory config

debug:
	@ sed -i -e 's/BUILD.*rel/BUILD=dbg/' $(CONFIG_FILE)
	@ $(MAKE) --no-print-directory config

# requires OUTDIR to be specified on the command line
out:
	@ sed -i -e '/OUTDIR.*=/d' $(CONFIG_FILE)
	echo OUTDIR=$(abspath $(OUTDIR)) >>$(CONFIG_FILE)
	$(MAKE) --no-print-directory OUTDIR=$(abspath $(OUTDIR)) config

#-------------------------------------------------------------------------------
# docs
#
docs:
	@ ( which doxygen >/dev/null && doxygen $(TOP)/docs/doxygen.config && echo "Documentation has been generated in $(TOP)/docs/html" ) || \
      ( echo "This target requires doxygen to be in the PATH." )

#-------------------------------------------------------------------------------
# install
#
# if not already in Makefile.config, expects INSTDIR to be specified on the command line
# NB: INSTDIR is expected  to be an absolute path

install:
	sed -i -e '/INSTDIR.*=/d' $(CONFIG_FILE)
	echo INSTDIR=$(INSTDIR) >>$(CONFIG_FILE)
	mkdir -p $(INSTDIR)
	$(MAKE) --no-print-directory -C services subinstall
	$(MAKE) --no-print-directory -C tools subinstall

#-------------------------------------------------------------------------------
# uninstall
#
uninstall:
	@ rm -f $(INSTDIR)/*

#-------------------------------------------------------------------------------
# package
#
PACKAGE ?= ncbi-vdb3

package: build
	docker image build -f $(TOP)/build/Dockerfile.package -t $(PACKAGE):latest $(BINDIR)

.PHONY: release debug out install package docs
