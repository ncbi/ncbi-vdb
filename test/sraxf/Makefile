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

MODULE = test/sraxf

TEST_TOOLS = \
	test-format_spot_name \
	test-fix_read_seg \
	test_syn_quality \

include $(TOP)/build/Makefile.env

$(TEST_TOOLS): makedirs
	@ $(MAKE_CMD) $(TEST_BINDIR)/$@

.PHONY: $(TEST_TOOLS) produce_test_src

clean: stdclean

#----------------------------------------------------------------
# fix_read_seg-test
#

SRATEST_SRC = \
	fix_read_seg-test

SRATEST_OBJ = \
	$(addsuffix .$(OBJX),$(SRATEST_SRC))

SRATEST_LIB = \
	-skapp \
	-sktst \
	-sncbi-vdb \

$(TEST_BINDIR)/test-fix_read_seg: $(SRATEST_OBJ)
	$(LP) --exe -o $@ $^ $(SRATEST_LIB)


#----------------------------------------------------------------
# test-format_spot_name
#

TFSN_SRC = \
	test-format_spot_name

TFSN_OBJ = \
	$(addsuffix .$(OBJX),$(TFSN_SRC))

$(TEST_BINDIR)/test-format_spot_name: $(TFSN_OBJ)
	$(LP) --exe -o $@ $^ $(SRATEST_LIB)


#----------------------------------------------------------------
# test_syn_quality
#

TST_SYN_QUAL_SRC = \
	test_syn_quality

TST_SYN_QUAL_OBJ = \
	$(addsuffix .$(OBJX),$(TST_SYN_QUAL_SRC))

TST_SYN_QUAL_LIB = \
	-skapp \
	-sktst \
	-sncbi-vdb \

$(TEST_BINDIR)/test_syn_quality: $(TST_SYN_QUAL_OBJ)
	$(LP) --exe -o $@ $^ $(TST_SYN_QUAL_LIB)

#----------------------------------------------------------------
# overriden runtests, to pass the SRCDIR into test_syn_quality
#
#runtests: test_syn_quality test-fix_read_seg
#	test-fix_read_seg
#	test_syn_quality $(SRCDIR)
