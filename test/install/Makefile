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

default: runtests

TOP ?= $(abspath ../..)

MODULE = test/install

ALL_TARGS = \

include $(TOP)/build/Makefile.env

ROOTDIR = $(SRCDIR)/root

SUBMAKE = $(MAKE) --no-print-directory

ifeq (true,$(LINUX_ROOT))
    #settings for the fake root mode
    INSTALL_DIR = $(ROOTDIR)
    KONFIG_DIR = $(ROOTDIR)/etc/ncbi
    SUBMAKE += LINUX_ROOT=$(LINUX_ROOT) ROOT=$(INSTALL_DIR)
else
    INSTALL_DIR = $(SRCDIR)/install
    KONFIG_DIR = $(INSTALL_DIR)/lib$(BITS)/ncbi
endif

# Override installation locations
INST_BINDIR = $(INSTALL_DIR)/bin
INST_LIBDIR = $(INSTALL_DIR)/lib$(BITS)
INST_INCDIR = $(INSTALL_DIR)/include

SUBMAKE += -C $(TOP) INST_BINDIR=$(INST_BINDIR) INST_LIBDIR=$(INSTALL_DIR)/lib INST_INCDIR=$(INST_INCDIR)

#-------------------------------------------------------------------------------
# runtests
#

INSTALLED_FILES = \
    $(INST_LIBDIR)/libncbi-ngs-c++-static.a     \
	$(INST_LIBDIR)/libncbi-ngs-c++.a            \
	$(INST_LIBDIR)/libncbi-ngs-c++.a.$(MAJVERS) \
	$(INST_LIBDIR)/libncbi-ngs-c++.a.$(VERSION) \
	$(INST_LIBDIR)/libncbi-vdb-static.a         \
	$(INST_LIBDIR)/libncbi-vdb.a                \
	$(INST_LIBDIR)/libncbi-vdb.a.$(MAJVERS)     \
	$(INST_LIBDIR)/libncbi-vdb.a.$(VERSION)     \
	$(INST_LIBDIR)/libncbi-vdb.so               \
	$(INST_LIBDIR)/libncbi-vdb.so.$(MAJVERS)    \
	$(INST_LIBDIR)/libncbi-vdb.so.$(VERSION)    \
	$(INST_LIBDIR)/libncbi-wvdb-static.a        \
	$(INST_LIBDIR)/libncbi-wvdb.a               \
	$(INST_LIBDIR)/libncbi-wvdb.a.$(MAJVERS)    \
	$(INST_LIBDIR)/libncbi-wvdb.a.$(VERSION)    \
	$(INST_LIBDIR)/libncbi-wvdb.so              \
	$(INST_LIBDIR)/libncbi-wvdb.so.$(MAJVERS)   \
	$(INST_LIBDIR)/libncbi-wvdb.so.$(VERSION)   \
	$(KONFIG_DIR)/certs.kfg                     \
	$(KONFIG_DIR)/default.kfg                   \
	$(INST_INCDIR)/ncbi-vdb/NGS.hpp             \


runtests: test-install

test-install: cleanup
	@ echo "installing to $(INSTALL_DIR)"
	@ #
	@ echo "1. clean install"
	@ #
	@ $(SUBMAKE) install
	@ for i in $(INSTALLED_FILES); do test -f "$$i" || ( echo "$$i is not found" >>$(INST_INCDIR)/missing ); done
	@ ! test -f $(INST_INCDIR)/missing || ! cat $(INST_INCDIR)/missing
	@ #
	@ echo "2. uninstall"
	@ #
	@ $(SUBMAKE) uninstall
	@ for i in $(INSTALLED_FILES); do ! test -f "$$i" || ( echo "$$i still exists" >>$(INST_INCDIR)/exist ); done
	@ ! test -f $(INST_INCDIR)/exist || ! cat $(INST_INCDIR)/exist
	@ #
	@ echo "3. re-install, preserving modified .kfg files"
	@ #
	@ # install
	@ $(SUBMAKE) install
	@ # modify .kfg files
	@ echo newcert  >$(KONFIG_DIR)/certs.kfg
	@ echo newdflt  >$(KONFIG_DIR)/default.kfg
	@ echo newncbi  >$(KONFIG_DIR)/ncbi-vdb.kfg     # this should be renamed and never installed again
	@ # reinstall
	@ $(SUBMAKE) install
	@ # verify changed files have been saved
	@ grep newcert    $(KONFIG_DIR)/certs.kfg.orig       >/dev/null
	@ grep newdflt    $(KONFIG_DIR)/default.kfg.orig     >/dev/null
	@ grep newncbi    $(KONFIG_DIR)/ncbi-vdb.kfg.orig    >/dev/null
	@ ! test -f $(KONFIG_DIR)/ncbi-vdb.kfg          # gone ...
	@ # verify that further reinstall does not overwrite previously saved files if there is no change
	@ $(SUBMAKE) install
	@ # verify saved files are still there
	@ grep newcert    $(KONFIG_DIR)/certs.kfg.orig       >/dev/null
	@ grep newdflt    $(KONFIG_DIR)/default.kfg.orig     >/dev/null
	@ grep newncbi    $(KONFIG_DIR)/ncbi-vdb.kfg.orig    >/dev/null
	@ ! test -f $(KONFIG_DIR)/ncbi-vdb.kfg          # ... and stays gone
	@ #
	@ #
ifeq (,$(LINUX_ROOT))
	#
	# Run the same tests in simulated root mode
	#
	@ $(MAKE) LINUX_ROOT=true
endif

cleanup:
	rm -rf $(INSTALL_DIR)
ifeq (true,$(LINUX_ROOT))
	# in root mode, recreate "system" directories
	mkdir -p $(ROOTDIR)/usr/include  $(ROOTDIR)/etc/profile.d
endif

.PHONY: install