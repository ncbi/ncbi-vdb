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
MODULE = libs/kdb

INT_LIBS = \
	libkdb \
	libwkdb

ALL_LIBS = \
	$(INT_LIBS)

OBJFILES = \
	$(KDB_OBJ) \
	$(WKDB_OBJ)

include $(TOP)/build/Makefile.env

DEFINES += -DUSE_BSTREE_IN_COLUMN_IDX1


#-------------------------------------------------------------------------------
# outer targets
#
all std: makedirs compile
	@ $(MAKE_CMD) $(TARGDIR)/std

$(INT_LIBS): makedirs
	@ $(MAKE_CMD) $(ILIBDIR)/$@

.PHONY: all std $(ALL_LIBS) $(ALL_DEFS)


#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: \
	$(addprefix $(ILIBDIR)/,$(INT_LIBS))

.PHONY: $(TARGDIR)/std

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean

.PHONY: clean

#-------------------------------------------------------------------------------
# kdb
#
$(ILIBDIR)/libkdb: $(addprefix $(ILIBDIR)/libkdb.,$(ILIBEXT))

KDB_CMN = \
	btree \
	rowset \
	rowset-simple \
	dbmgr-cmn \
#	database-cmn

KDB_SRC = \
	$(KDB_CMN) \
	kdb \
	dbmgr \
	dbcc-cmn \
	database \
	database-cc \
	table \
	table-cc \
	column \
	column-cc \
	coldata \
	colidx \
	colidx0 \
	colidx1 \
	colidx2 \
	idxblk \
	meta \
	index \
	idstats \
	trieidx-v1 \
	trieval-v1 \
	ptrieval-v1 \
	trieidx-v2 \
	trieval-v2 \
	ptrieval-v2 \
	u64idx-v3

KDB_OBJ = \
	$(addsuffix .$(LOBX),$(KDB_SRC))

KDB_LIB = \
	-dvfs \
	-dkrypto \
	-dkfs \
	-dklib

$(ILIBDIR)/libkdb.$(LIBX): $(KDB_OBJ)
	$(LD) --slib -o $@ $^ $(KDB_LIB)


#-------------------------------------------------------------------------------
# wkdb
#
$(ILIBDIR)/libwkdb: $(addprefix $(ILIBDIR)/libwkdb.,$(ILIBEXT))

WKDB_SRC = \
	$(KDB_CMN) \
	wkdb \
	wdbmgr \
	wdatabase \
	wtable \
	wcolumn \
	wcoldata \
	wcolidx \
	wcolidx0 \
	wcolidx1 \
	wcolidx2 \
	idxblk \
	widxblk \
	wmeta \
	windex \
	wtrieidx-v1 \
	wtrieidx-v2 \
	wu64idx-v3

WKDB_OBJ = \
	$(addsuffix .$(LOBX),$(WKDB_SRC))

WKDB_LIB = \
	-dvfs \
	-dkrypto \
	-dkfs \
	-dklib

$(ILIBDIR)/libwkdb.$(LIBX): $(WKDB_OBJ)
	$(LD) --slib -o $@ $^ $(WKDB_LIB)

compile: stdcompile

$(TARGDIR)/compile: $(OBJFILES)

#-------------------------------------------------------------------------------
# fuzz
#
fuzz: std
	cp $(addsuffix .$(LIBX),$(addprefix $(ILIBDIR)/,$(INT_LIBS))) $(FLIBDIR)/