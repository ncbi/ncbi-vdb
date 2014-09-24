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
MODULE = libs/ext/regex

INT_LIBS = \

EXT_LIBS = \
	libregex

ALL_LIBS = \
	$(INT_LIBS) \
	$(EXT_LIBS)

include $(TOP)/build/Makefile.env

# don't compile in pedantic mode
PED =

#-------------------------------------------------------------------------------
# outer targets
#
all std: makedirs
	@ $(MAKE_CMD) $(TARGDIR)/$@

full: makedirs
	@ [ -f libregex.vers ] && $(MAKE_CMD) $(TARGDIR)/all || $(MAKE_CMD) $(TARGDIR)/std

$(INT_LIBS): makedirs
	@ $(MAKE_CMD) $(ILIBDIR)/$@

$(EXT_LIBS): makedirs
	@ $(MAKE_CMD) $(LIBDIR)/$@

.PHONY: all std $(ALL_LIBS)

#-------------------------------------------------------------------------------
# all
#
$(TARGDIR)/all: \
	$(addprefix $(ILIBDIR)/,$(INT_LIBS)) \
	$(addprefix $(LIBDIR)/,$(EXT_LIBS))

.PHONY: $(TARGDIR)/all

#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: $(LIBDIR)/libregex-$(OS).exp

.PHONY: $(TARGDIR)/std

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean

.PHONY: clean

#-------------------------------------------------------------------------------
# libregex
#
$(LIBDIR)/libregex: $(addprefix $(LIBDIR)/libregex.,$(LIBEXT))

REGEX_SRC = \
	regex

REGEX_OBJ = \
	$(addsuffix .$(LOBX),$(REGEX_SRC))

REGEX_LIB = \

$(LIBDIR)/libregex.$(SHLX): $(REGEX_OBJ)
	$(LD) --dlib --vers $(SRCDIR) -o $@ $^ $(REGEX_LIB)

$(LIBDIR)/libregex.$(LIBX): $(REGEX_OBJ)
	$(LD) --slib --vers $(SRCDIR) -o $@ $^ $(REGEX_LIB)


#-------------------------------------------------------------------------------
# libregex.exp
#
$(LIBDIR)/libregex-linux.exp $(LIBDIR)/libregex-mac.exp $(LIBDIR)/libregex-sun.exp:
	@ true

$(LIBDIR)/libregex-win.exp: $(LIBDIR)/libregex.exp

$(LIBDIR)/libregex.exp: $(SRCDIR)/libregex.def
	$(LD) --stub -o $(LIBDIR)/libregex
