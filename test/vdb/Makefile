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

MODULE = test/vdb

TEST_TOOLS = \
	test-vdb \
	test-wvdb \
	test-dependencies \
	test-sparse-col-rw \
	test-sparse-col-rd \
	test-blob-val \
	test-VDB-3060
    
include $(TOP)/build/Makefile.env


# make sure runs are not cached in the user repository when running tests
ifeq ($(wildcard ../../../asm-trace),)
	ifeq (,$(VDB_CONFIG))
		VDB_CONFIG = $(shell pwd)/../only-remote-repository.kfg
	endif
endif


$(TEST_TOOLS): makedirs
	@ $(MAKE_CMD) $(TEST_BINDIR)/$@

runtests: setup 

setup:
	@ mkdir -p db

.PHONY: $(TEST_TOOLS) setup

clean: stdclean


#-------------------------------------------------------------------------------
# test-vdb
#
TEST_VDB_SRC = \
	test-vdb

TEST_VDB_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_VDB_SRC))

TEST_VDB_LIB = \
	-skapp \
	-sktst \
	-sncbi-vdb \

$(TEST_BINDIR)/test-vdb: $(TEST_VDB_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_VDB_LIB)

vdb: test-vdb
	$(TEST_BINDIR)/test-vdb    

valgrind_vdb: std
	valgrind --ncbi --show-reachable=no $(TEST_BINDIR)/test-vdb    

#-------------------------------------------------------------------------------
# test-dependencies
#
TEST_DEPS_SRC = \
	test-dependencies

TEST_DEPS_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_DEPS_SRC))

TEST_DEPS_LIB = \
	-skapp \
	-sktst \
	-sncbi-vdb \

$(TEST_BINDIR)/test-dependencies: $(TEST_DEPS_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_DEPS_LIB)

valgrind_deps: std
	valgrind --ncbi --show-reachable=no $(TEST_BINDIR)/test-dependencies    

#-------------------------------------------------------------------------------
# test-wvdb
#
TEST_WVDB_SRC = \
	test-wvdb

TEST_WVDB_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_WVDB_SRC))

TEST_WVDB_LIB = \
	-skapp \
	-sktst \
	-sncbi-wvdb \

$(TEST_BINDIR)/test-wvdb: $(TEST_WVDB_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_WVDB_LIB)

wvdb: test-wvdb
	$(TEST_BINDIR)/test-wvdb    

valgrind_wvdb: std
	valgrind --ncbi --show-reachable=no $(TEST_BINDIR)/test-wvdb    

#-------------------------------------------------------------------------------
# test-sparse-col
#  We will compile the same test file first in read/write mode, create DB and
#  run tests using libncbi-wvdb library; then compile the same test with -DREAD_ONLY 
#  and link it against read-only lincbi-vdb library
TEST_SPARSE_COL_SRC = \
	test-sparse-col

TEST_SPARSE_COL_OBJ_RW = \
	$(addsuffix .$(OBJX),$(TEST_SPARSE_COL_SRC)-rw)
	
TEST_SPARSE_COL_OBJ_RD = \
	$(addsuffix .$(OBJX),$(TEST_SPARSE_COL_SRC)-rd)

TEST_SPARSE_COL_LIB = \
	-skapp \
	-sktst \

$(TEST_SPARSE_COL_OBJ_RW): $(TEST_SPARSE_COL_SRC).cpp
	$(CP) -o $@ $(OPT) $< $(LOC_INFO) -D__file_ext__=cpp

$(TEST_SPARSE_COL_OBJ_RD): $(TEST_SPARSE_COL_SRC).cpp
	$(CP) -o $@ $(OPT) $< $(LOC_INFO) -D__file_ext__=cpp -DREAD_ONLY	

$(TEST_BINDIR)/test-sparse-col-rw: $(TEST_SPARSE_COL_OBJ_RW)
	$(LP) --exe -o $@ $^ $(TEST_SPARSE_COL_LIB) -sncbi-wvdb

$(TEST_BINDIR)/test-sparse-col-rd: $(TEST_SPARSE_COL_OBJ_RD)
	$(LP) --exe -o $@ $^ $(TEST_SPARSE_COL_LIB) -sncbi-vdb

sparse-col: test-sparse-col-rw test-sparse-col-rd
	$(TEST_BINDIR)/test-sparse-col-rw
	$(TEST_BINDIR)/test-sparse-col-rd

valgrind_sparse-col: std
	valgrind --ncbi --show-reachable=no $(TEST_BINDIR)/test-sparse-col-rw
	valgrind --ncbi --show-reachable=no $(TEST_BINDIR)/test-sparse-col-rd   
	
#-------------------------------------------------------------------------------
# test-blob-val
#
TEST_BLOB_VAL_SRC = \
	test-blob-val

TEST_BLOB_VAL_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_BLOB_VAL_SRC))
	
TEST_BLOB_VAL_LIB = \
	-skapp \
	-sktst \
	-sncbi-vdb \

$(TEST_BINDIR)/test-blob-val: $(TEST_BLOB_VAL_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_BLOB_VAL_LIB)

blob-val: test-blob-val
	$(TEST_BINDIR)/test-blob-val

#-------------------------------------------------------------------------------
# SYS-224064
#
SYS_224064_SRC = \
	SYS-224064

SYS_224064_OBJ = \
	$(addsuffix .$(OBJX),$(SYS_224064_SRC))

SYS_224064_LIB = \
	-sncbi-vdb \

$(TEST_BINDIR)/SYS-224064: $(SYS_224064_OBJ)
	$(LP) --exe -o $@ $^ $(SYS_224064_LIB)

SYS-224064: makedirs
	@ $(MAKE_CMD) $(TEST_BINDIR)/$@


#-------------------------------------------------------------------------------
# VDB-3060
#
VDB_3060_SRC = \
	test-VDB-3060

VDB_3060_OBJ = \
	$(addsuffix .$(OBJX),$(VDB_3060_SRC))

VDB_3060_LIB = \
	-skapp \
	-sktst \
	-sncbi-vdb \

$(TEST_BINDIR)/test-VDB-3060: $(VDB_3060_OBJ)
	$(LP) --exe -o $@ $^ $(VDB_3060_LIB)

VDB-3060: test-VDB-3060
	$(TEST_BINDIR)/test-VDB-3060
