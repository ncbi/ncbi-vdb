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

MODULE = test/ngs

TEST_TOOLS = \
    test-ngs \
    test-ngs_sra \
 	test-ngs_sradb \
 	test-ngs_csra1 \
 	test-ngs_csra1_readcollection \
    test-ngs_csra1_refwin \
    test-ngs_csra1_pileup \

include $(TOP)/build/Makefile.env

$(TEST_TOOLS): makedirs
	@ $(MAKE_CMD) $(TEST_BINDIR)/$@

.PHONY: $(TEST_TOOLS)

clean: stdclean

#-------------------------------------------------------------------------------
# common definitions
#
INCDIRS +=  -I$(TOP)/libs/ngs

valgrind: valgrind_ngs \
          valgrind_sra \
          valgrind_sradb \
          valgrind_csra1 \
          valgrind_csra1_readcollection \
          valgrind_csra1_refwin \
          valgrind_csra1_pileup \

TEST_NGS_LIB = \
	-sncbi-vdb \
	-skapp \
    -sktst

#-------------------------------------------------------------------------------
# test-ngs
#
TEST_NGS_SRC = \
	ngstest

TEST_NGS_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_NGS_SRC))

$(TEST_BINDIR)/test-ngs: $(TEST_NGS_OBJ) 
	$(LP) --exe -o $@ $^ $(TEST_NGS_LIB)

valgrind_ngs: std
	valgrind --ncbi --suppressions=$(SRCDIR)/valgrind.suppress --error-exitcode=1 $(TEST_BINDIR)/test-ngs 
    

#-------------------------------------------------------------------------------
# test-ngs_sra
#
TEST_NGS_SRA_SRC = \
	ngstest_sra

TEST_NGS_SRA_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_NGS_SRA_SRC))

$(TEST_BINDIR)/test-ngs_sra: $(TEST_NGS_SRA_OBJ) 
	$(LP) --exe -o $@ $^ $(TEST_NGS_LIB)

valgrind_sra: std
	valgrind --ncbi --suppressions=$(SRCDIR)/valgrind.suppress --error-exitcode=1 $(TEST_BINDIR)/test-ngs_sra 
    
#-------------------------------------------------------------------------------
# test-ngs_sradb
#
TEST_NGS_SRADB_SRC = \
	ngstest_sradb

TEST_NGS_SRADB_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_NGS_SRADB_SRC))

$(TEST_BINDIR)/test-ngs_sradb: $(TEST_NGS_SRADB_OBJ) 
	$(LP) --exe -o $@ $^ $(TEST_NGS_LIB)

valgrind_sradb: std
	valgrind --ncbi --suppressions=$(SRCDIR)/valgrind.suppress  --error-exitcode=1 $(TEST_BINDIR)/test-ngs_sradb 
    
#-------------------------------------------------------------------------------
# test-ngs_csra1
#
TEST_NGS_CSRA1_SRC = \
	ngstest_csra1

TEST_NGS_CSRA1_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_NGS_CSRA1_SRC))

$(TEST_BINDIR)/test-ngs_csra1: $(TEST_NGS_CSRA1_OBJ) 
	$(LP) --exe -o $@ $^ $(TEST_NGS_LIB)

valgrind_csra1: std
	valgrind --ncbi --suppressions=$(SRCDIR)/valgrind.suppress  --error-exitcode=1 $(TEST_BINDIR)/test-ngs_csra1 

#-------------------------------------------------------------------------------
# test-ngs_csra1_readcollection
#
TEST_NGS_CSRA1_READCOLLECTION_SRC = \
	ngstest_csra1_readcollection

TEST_NGS_CSRA1_READCOLLECTION_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_NGS_CSRA1_READCOLLECTION_SRC))

$(TEST_BINDIR)/test-ngs_csra1_readcollection: $(TEST_NGS_CSRA1_READCOLLECTION_OBJ) 
	$(LP) --exe -o $@ $^ $(TEST_NGS_LIB)

valgrind_csra1_readcollection: std
	valgrind --ncbi --suppressions=$(SRCDIR)/valgrind.suppress  --error-exitcode=1 $(TEST_BINDIR)/test-ngs_csra1_readcollection     


#-------------------------------------------------------------------------------
# test-ngs_csra1_refwin
#
TEST_NGS_CSRA1_REFWIN_SRC = \
	ngstest_csra1_refwin

TEST_NGS_CSRA1_REFWIN_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_NGS_CSRA1_REFWIN_SRC))

$(TEST_BINDIR)/test-ngs_csra1_refwin: $(TEST_NGS_CSRA1_REFWIN_OBJ) 
	$(LP) --exe -o $@ $^ $(TEST_NGS_LIB)

valgrind_csra1_refwin: std
	valgrind --ncbi --suppressions=$(SRCDIR)/valgrind.suppress  --error-exitcode=1 $(TEST_BINDIR)/test-ngs_csra1_refwin     

#-------------------------------------------------------------------------------
# test-ngs_csra1_pileup
#
TEST_NGS_CSRA1_PILEUP_SRC = \
	ngstest_csra1_pileup

TEST_NGS_CSRA1_PILEUP_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_NGS_CSRA1_PILEUP_SRC))

$(TEST_BINDIR)/test-ngs_csra1_pileup: $(TEST_NGS_CSRA1_PILEUP_OBJ) 
	$(LP) --exe -o $@ $^ $(TEST_NGS_LIB)

valgrind_csra1_pileup: std
	valgrind --ncbi --suppressions=$(SRCDIR)/valgrind.suppress  --error-exitcode=1 $(TEST_BINDIR)/test-ngs_csra1_pileup     

