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

MODULE = test/ngs-c++

TEST_TOOLS = \
    test-ngs_sra-c++ \
    test-ngs_sradb-c++ \
    test-ngs_csra1-c++ \
	test-ngs_csra1pileup-c++ \


SLOW_TEST_TOOLS = \
    test-ngs_sra_pileup_diff
    
include $(TOP)/build/Makefile.env

ifeq (,$(NGS_LIBDIR))

$(TEST_TOOLS):
	@true 
	
runtests slowtests:
	@ echo "NGS C++ tests are not run since NGS library does not appear to be installed"

else

$(TEST_TOOLS) $(SLOW_TEST_TOOLS): makedirs
	@ $(MAKE_CMD) $(TEST_BINDIR)/$@

.PHONY: $(TEST_TOOLS) $(SLOW_TEST_TOOLS)

clean: stdclean

#-------------------------------------------------------------------------------
# common defs 
#
INCDIRS += -I$(NGS_INCDIR)

valgrind: valgrind_sra-c++ \
          valgrind_sradb-c++ \
          valgrind_csra1-c++ 

VALGRIND = valgrind --ncbi --suppressions=$(SRCDIR)/valgrind.suppress --error-exitcode=1
		  
TEST_NGS_CPP_LIB = \
	-sncbi-ngs-c++ \
	-sncbi-vdb \
	-skapp \
    -sktst \
    -L$(NGS_LIBDIR) \
	-sngs-c++ \

#-------------------------------------------------------------------------------
# test-ngs_sra-c++
#
TEST_NGS_SRA_CPP_SRC = \
	SRAtest

TEST_NGS_SRA_CPP_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_NGS_SRA_CPP_SRC))

$(TEST_BINDIR)/test-ngs_sra-c++: $(TEST_NGS_SRA_CPP_OBJ) 
	$(LP) --exe -o $@ $^ $(TEST_NGS_CPP_LIB)

valgrind_sra-c++: std
	 $(VALGRIND) $(TEST_BINDIR)/test-ngs_sra-c++     

#-------------------------------------------------------------------------------
# test-ngs_sradb-c++
#
TEST_NGS_SRADB_CPP_SRC = \
	SRADBtest

TEST_NGS_SRADB_CPP_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_NGS_SRADB_CPP_SRC))

$(TEST_BINDIR)/test-ngs_sradb-c++: $(TEST_NGS_SRADB_CPP_OBJ) 
	$(LP) --exe -o $@ $^ $(TEST_NGS_CPP_LIB)

valgrind_sradb-c++: std
	$(VALGRIND) $(TEST_BINDIR)/test-ngs_sradb-c++     
    
#-------------------------------------------------------------------------------
# test-ngs_csra1-c++
#
TEST_NGS_CSRA1_CPP_SRC = \
	CSRA1test

TEST_NGS_CSRA1_CPP_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_NGS_CSRA1_CPP_SRC))

$(TEST_BINDIR)/test-ngs_csra1-c++: $(TEST_NGS_CSRA1_CPP_OBJ) 
	$(LP) --exe -o $@ $^ $(TEST_NGS_CPP_LIB)

valgrind_csra1-c++: std
	$(VALGRIND) $(TEST_BINDIR)/test-ngs_csra1-c++     
 
#-------------------------------------------------------------------------------
# test-ngs_csra1pileup-c++
#
TEST_NGS_CSRA1PILEUP_CPP_SRC = \
	CSRA1PileupTest

TEST_NGS_CSRA1PILEUP_CPP_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_NGS_CSRA1PILEUP_CPP_SRC))

$(TEST_BINDIR)/test-ngs_csra1pileup-c++: $(TEST_NGS_CSRA1PILEUP_CPP_OBJ) 
	$(LP) --exe -o $@ $^ $(TEST_NGS_CPP_LIB)

valgrind_csra1pileup-c++: std
	$(VALGRIND) $(TEST_BINDIR)/test-ngs_csra1pileup-c++     

endif

#-------------------------------------------------------------------------------
# test-ngs_sra_pileup_diff
#
TEST_NGS_SRA_PILEUP_DIFF_SRC = \
	ngs_sra_pileup_diff

TEST_NGS_SRA_PILEUP_DIFF_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_NGS_SRA_PILEUP_DIFF_SRC))

$(TEST_BINDIR)/test-ngs_sra_pileup_diff: $(TEST_NGS_SRA_PILEUP_DIFF_OBJ) 
	$(LP) --exe -o $@ $^ $(TEST_NGS_CPP_LIB)

valgrind_ngs_sra_pileup_diff: std
	valgrind --ncbi --suppressions=$(SRCDIR)/valgrind.suppress  --error-exitcode=1 $(TEST_BINDIR)/test-ngs_sra_pileup_diff    

runtests:
	$(BINDIR)/vdb-config
	$(BINDIR)/vdb-dump SRR1237963 -E
    
