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

#$(error libs/ncbi is no longer supported; code moved to libs/vfs)

default: std

TOP ?= $(abspath ../..)
MODULE = libs/ncbi

INT_LIBS = \

EXT_LIBS = \
	sra-cgi-lib

ALL_LIBS = \
	$(INT_LIBS) \
	$(EXT_LIBS)

include $(TOP)/build/Makefile.env

#-------------------------------------------------------------------------------
# outer targets
#
all std: vers-includes
	@ $(MAKE_CMD) $(TARGDIR)/std

$(INT_LIBS): vers-includes
	@ $(MAKE_CMD) $(ILIBDIR)/$@

$(EXT_LIBS): vers-includes
	@ $(MAKE_CMD) $(LIBDIR)/$@

.PHONY: all std $(ALL_LIBS)

#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: \
	$(addprefix $(ILIBDIR)/,$(INT_LIBS)) \
	$(addprefix $(LIBDIR)/,$(EXT_LIBS))

.PHONY: $(TARGDIR)/std

#-------------------------------------------------------------------------------
# vers-includes
#
$(TARGDIR)/vers-includes: $(addsuffix .vers.h,$(EXT_LIBS))

.PHONY: $(TARGDIR)/vers-includes

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean
	@ rm -f $(addsuffix -dbgap*,$(addprefix $(LIBDIR)/,$(ALL_LIBS)))
	@ rm -f $(LIBDIR)/libsra-path-static.$(LIBX)


.PHONY: clean

#-------------------------------------------------------------------------------
# srapath
#
$(LIBDIR)/libsra-path: $(addprefix $(LIBDIR)/libsra-path.,$(SHLX) $(LIBX))

ifneq (win,$(OS))
$(LIBDIR)/libsra-path: $(LIBDIR)/libsra-path-static.$(LIBX)
endif

SRAPATH_SRC = \
	srapath

SRAPATH_OBJ = \
	$(addsuffix .$(LOBX),$(SRAPATH_SRC))

SRAPATH_LIB = \
	-dkfg \
	-dkfs \
	-dklib \
#	-dz \
#	-dbz2

$(LIBDIR)/libsra-path.$(SHLX): $(SRAPATH_OBJ)
	$(LD) --dlib --vers $(SRCDIR) -o $@ $^ $(SRAPATH_LIB)

$(LIBDIR)/libsra-path.$(LIBX): $(SRAPATH_OBJ)
	$(LD) --slib -o $@ $^

# hack-o-matic to force ability to link staticly
$(LIBDIR)/libsra-path-static.$(LIBX): $(LIBDIR)/libsra-path.$(LIBX)
	( cd $(LIBDIR); \
	  ln -fs libsra-path.$(LIBX) libsra-path-static.$(LIBX); \
	  if test -f libsra-path.$(LIBX).md5; \
	  then \
	    ln -fs libsra-path.$(LIBX).md5 libsra-path-static.$(LIBX).md5; \
	  fi )

#-------------------------------------------------------------------------------
# sra-cgi
#
$(LIBDIR)/sra-cgi-lib: $(addprefix $(LIBDIR)/sra-cgi-lib.,$(SHLX) $(LIBX))

CGI_SRC = \
	sra-cgi-lib \
	sra-cgi-dispatch \

CGI_OBJ = \
	$(addsuffix .$(LOBX),$(CGI_SRC))

CGI_LIB = \

$(LIBDIR)/sra-cgi-lib.$(SHLX): $(CGI_OBJ)
	$(LD) --dlib --vers $(SRCDIR) -o $@ $^ $(CGI_LIB)

$(LIBDIR)/sra-cgi-lib.$(LIBX): $(CGI_OBJ)
	$(LD) --slib -o $@ $^

