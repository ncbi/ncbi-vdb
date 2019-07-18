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

TOP ?= $(abspath ../..)
MODULE = libs/svdb

INT_LIBS = \

EXT_LIBS = \
	libsvdb

ALL_LIBS = \
	$(INT_LIBS) \
	$(EXT_LIBS)

include $(TOP)/build/Makefile.env

#-------------------------------------------------------------------------------
# outer targets
#
all std: makedirs
	@ $(MAKE_CMD) $(TARGDIR)/std

$(INT_LIBS): makedirs
	@ $(MAKE_CMD) $(ILIBDIR)/$@

$(EXT_LIBS): makedirs
	@ $(MAKE_CMD) $(LIBDIR)/$@

.PHONY: all std $(ALL_LIBS)

#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: \
	$(addprefix $(ILIBDIR)/,$(INT_LIBS)) \
	$(addprefix $(LIBDIR)/,$(EXT_LIBS))

.PHONY: $(TARGDIR)/all $(TARGDIR)/std

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean

.PHONY: clean

#-------------------------------------------------------------------------------
# virtual db library
#
$(LIBDIR)/libsvdb: $(addprefix $(LIBDIR)/libsvdb.,$(LIBEXT))

SVDB_SRC = \
	svdb

SVDB_OBJ = \
	$(addsuffix .$(LOBX),$(SVDB_SRC))

SVDB_LIB = \
	-lncbi-vdb \
	-lm

#ifeq (win,$(OS))
#	SVDB_LIB += -lShell32
#endif

$(LIBDIR)/libsvdb.$(SHLX): $(SVDB_OBJ)
	$(LD) --dlib --vers $(SRCDIR)/../ncbi-vdb/libncbi-vdb.vers -o $@ $^ $(SVDB_LIB)

$(LIBDIR)/libsvdb.$(LIBX): $(SVDB_OBJ)
	$(LD) --slib --vers $(SRCDIR)/../ncbi-vdb/libncbi-vdb.vers -o $@ $^ $(SVDB_LIB)

#-------------------------------------------------------------------------------
# fuzz
#
fuzz: std
	cp $(addsuffix .$(LIBX),$(addprefix $(ILIBDIR)/,$(INT_LIBS))) $(FLIBDIR)/
