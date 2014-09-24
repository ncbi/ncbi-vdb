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


default: std

TOP ?= $(abspath ../../..)
MODULE = libs/ext/szip

include $(TOP)/build/Makefile.env

INT_LIBS = \

EXT_LIBS = \
	libsz

ALL_LIBS = \
	$(INT_LIBS) \
	$(EXT_LIBS)

#-------------------------------------------------------------------------------
# outer targets
#
all std: makedirs download-$(OS)
	@ $(MAKE_CMD) $(TARGDIR)/std

$(INT_LIBS): makedirs download-$(OS)
	@ $(MAKE_CMD) $(ILIBDIR)/$@

$(EXT_LIBS): makedirs download-$(OS)
	@ $(MAKE_CMD) $(LIBDIR)/$@

.PHONY: all std $(ALL_LIBS)

#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: $(LIBDIR)/libsz-$(OS)

.PHONY: $(TARGDIR)/std

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean

.PHONY: clean

#-------------------------------------------------------------------------------
# libsz
#
$(LIBDIR)/libsz: $(addprefix $(LIBDIR)/libsz.,$(LIBEXT))

libsz-make:
	./configure
	make

$(LIBDIR)/libsz-linux.$(SHLX): libsz-make
	cp -a src/.libs/libsz.$(SHLX)* $(LIBDIR)

$(LIBDIR)/libsz-mac.$(SHLX): libsz-make
	cp -a src/.libs/libsz*.$(SHLX) $(LIBDIR)

$(LIBDIR)/libsz.$(SHLX): $(LIBDIR)/libsz-$(OS).$(SHLX)

$(LIBDIR)/libsz.$(LIBX): libsz-make
	cp -a src/.libs/libsz.$(LIBX) $(LIBDIR)

.PHONY: libsz-make $(LIBDIR)/libsz-$(OS).$(SHLX)

#-------------------------------------------------------------------------------
# libsz
#
$(LIBDIR)/libsz-$(OS): $(LIBDIR)/libsz

.PHONY: $(LIBDIR)/libsz-$(OS)


#-------------------------------------------------------------------------------
# libsz download
#
SZIP_VERS = 2.1
SZIP_SRC = szip-$(SZIP_VERS).tar.gz
SZIP_URL = http://www.hdfgroup.org/ftp/lib-external/szip/2.1/src/$(SZIP_SRC)

$(SZIP_SRC):
	@ echo downloading the sources
	wget $(SZIP_URL) && touch $@

download-$(OS): $(SZIP_SRC)
	@ echo unpacking the sources
	@ rm -rf $(OBJDIR)/*
	tar -C $(OBJDIR) -xzf $^
	@ ( cd $(OBJDIR); mv szip-$(SZIP_VERS)/* .; rmdir szip-$(SZIP_VERS) )

.PHONY: download-$(OS)
