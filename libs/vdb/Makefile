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
MODULE = libs/vdb

INT_LIBS = \
	libvdb \
	libwvdb

ALL_LIBS = \
	$(INT_LIBS)

OBJFILES =     \
	$(VDB_OBJ) \
	$(WVDB_OBJ)

include $(TOP)/build/Makefile.env

#-------------------------------------------------------------------------------
# outer targets
#
all std: makedirs compile
	@ $(MAKE_CMD) $(TARGDIR)/std

$(INT_LIBS): makedirs
	@ $(MAKE_CMD) $(ILIBDIR)/$@

.PHONY: all std $(ALL_LIBS)

#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: \
	$(addprefix $(ILIBDIR)/,$(INT_LIBS))

.PHONY: $(TARGDIR)/all $(TARGDIR)/std

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean

.PHONY: clean

#-------------------------------------------------------------------------------
# virtual db library
#
$(ILIBDIR)/libvdb: $(addprefix $(ILIBDIR)/libvdb.,$(ILIBEXT))

VDBCMN_SRC = \
	dbmgr-cmn \
	schema-tok \
	schema-parse \
	schema-type \
	schema-func \
	schema-prod \
	schema-expr \
	schema-eval \
	schema-tbl \
	schema-view \
	schema-db \
	schema-dump \
	schema-int \
	schema \
	linker-int \
	linker-cmn \
	database-cmn \
	database-depend \
	database-load \
	table-cmn \
	table-load \
	cursor-cmn \
	column-cmn \
	prod-cmn \
	prod-expr \
	prod-func \
	phys-cmn \
	phys-load \
	blob \
	blob-headers \
	page-map \
	row-id \
	row-len \
	fixed-row-len \
	merge \
	split \
	compare \
	meta-read \
	meta-value \
	meta-attr-read \
	cast \
	range-validate \
	parameter-read \
	redimension \
	index_project \
	index_lookup \
	transpose \
	delta_average \
	report-vdb \
	bytecode \
	view \
	cursor-view \
	cursor-table \

VDB_SRC = \
	$(VDBCMN_SRC) \
	dbmgr \
	linker \
	database \
	table \
	cursor \
	column \
	prod \
	phys

VDB_OBJ = \
	$(addsuffix .$(LOBX),$(VDB_SRC))

VDB_LIB = \
	-dkdb \
	-dvfs \
	-dkfg \
	-dkfs \
	-dkproc \
	-dklib \
	-dz

$(ILIBDIR)/libvdb.$(LIBX): $(VDB_OBJ)
	$(LD) --slib -o $@ $^ $(VDB_LIB)

#-------------------------------------------------------------------------------
# update version
#
$(ILIBDIR)/libwvdb: $(addprefix $(ILIBDIR)/libwvdb.,$(ILIBEXT))

WVDB_SRC = \
	$(VDBCMN_SRC) \
	wdbmgr \
	wlinker \
	wdatabase \
	wtable \
	wcursor \
	wcolumn \
	wprod \
	wphys \
	meta-write \
	meta-attr-write \
	meta-append \
	index_insert

WVDB_OBJ = \
	$(addsuffix .$(LOBX),$(WVDB_SRC))

WVDB_LIB = \
	-dwkdb \
	-dvfs \
	-dkfg \
	-dkfs \
	-dkproc \
	-dklib \
	-dz

$(ILIBDIR)/libwvdb.$(LIBX): $(WVDB_OBJ)
	$(LD) --slib -o $@ $^ $(WVDB_LIB)

compile: stdcompile

$(TARGDIR)/compile: $(OBJFILES)

#-------------------------------------------------------------------------------
# fuzz
#
fuzz: std
	cp $(addsuffix .$(LIBX),$(addprefix $(ILIBDIR)/,$(INT_LIBS))) $(FLIBDIR)/
