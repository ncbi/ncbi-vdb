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

MODULE = test/schema

TEST_TOOLS = \
	wb-test-schema-lex \
	wb-test-schema-parser \
	wb-test-schema-ast \

INT_TOOLS = \
	test-schema-scan \
	test-schema-parse \
	test-schema-diff \

ALL_TOOLS = $(TEST_TOOLS) $(INT_TOOLS)

include $(TOP)/build/Makefile.env

$(ALL_TOOLS): makedirs
	@ $(MAKE_CMD) $(TEST_BINDIR)/$@

.PHONY: $(ALL_TOOLS)

clean: stdclean

TEST_LIB = \
	-sktst \
	-skapp \
    -sncbi-vdb \
    -sschema \

#-------------------------------------------------------------------------------
# white box lexer test
#
TESTLEX_SRC = \
	wb-test-schema-lex

TESTLEX_OBJ = \
	$(addsuffix .$(OBJX),$(TESTLEX_SRC))

$(TEST_BINDIR)/wb-test-schema-lex: $(TESTLEX_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_LIB)

lex: wb-test-schema-lex
	$(TEST_BINDIR)/$^

#-------------------------------------------------------------------------------
# black box scanner test
#
TESTSCAN_SRC = \
	test-schema-scan

TESTSCAN_OBJ = \
	$(addsuffix .$(OBJX),$(TESTSCAN_SRC))

$(TEST_BINDIR)/test-schema-scan: $(TESTSCAN_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_LIB)

#-------------------------------------------------------------------------------
# white box parser test
#
TESTGRAM_SRC = \
	wb-test-schema-parser

TESTGRAM_OBJ = \
	$(addsuffix .$(OBJX),$(TESTGRAM_SRC))

$(TEST_BINDIR)/wb-test-schema-parser: $(TESTGRAM_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_LIB)

gram: wb-test-schema-parser
	@mkdir -p $(SRCDIR)/actual
	$(TEST_BINDIR)/$^

#-------------------------------------------------------------------------------
# black box parser test
#
TESTPARSE_SRC = \
	test-schema-parse

TESTPARSE_OBJ = \
	$(addsuffix .$(OBJX),$(TESTPARSE_SRC))

$(TEST_BINDIR)/test-schema-parse: $(TESTPARSE_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_LIB)

#-------------------------------------------------------------------------------
# old vs new schema matching
#
TESTDIFF_SRC = \
	test-schema-diff

TESTDIFF_OBJ = \
	$(addsuffix .$(OBJX),$(TESTDIFF_SRC))

$(TEST_BINDIR)/test-schema-diff: $(TESTDIFF_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_LIB)

#-------------------------------------------------------------------------------
# AST test
#
TESTAST_SRC = \
	AST_Fixture \
	wb-test-schema-ast \

TESTAST_OBJ = \
	$(addsuffix .$(OBJX),$(TESTAST_SRC))

$(TEST_BINDIR)/wb-test-schema-ast: $(TESTAST_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_LIB)

ast: wb-test-schema-ast
	$(TEST_BINDIR)/$^

#-------------------------------------------------------------------------------
# runtests
#
runtests: existing-schema-tests

wb-tests: lex gram ast

existing-schema-tests: wb-tests
	$(MAKE) scantest parsetest difftest

SCHEMATA := $(shell find $(TOP) -name "*.vschema")

scantest: test-schema-scan
	$(TEST_BINDIR)/test-schema-scan $(SCHEMATA)
	@echo "Schema scanner test complete"

parsetest: test-schema-parse
	-$(TEST_BINDIR)/test-schema-parse $(SCHEMATA)
	@echo "Schema parser test complete"

mv:test-schema-diff
	mkdir -p $(OBJDIR)/data
	-$(TEST_BINDIR)/test-schema-diff -I$(TOP)/interfaces -o$(OBJDIR)/data mv.vschema

difftest: test-schema-diff
	mkdir -p $(OBJDIR)/data
	VDB_CONFIG=$(SRCDIR)/local.kfg $(TEST_BINDIR)/test-schema-diff -I$(TOP)/interfaces -o$(OBJDIR)/data \
	$(addprefix	$(TOP)/interfaces/, \
		insdc/insdc.vschema \
		insdc/seq.vschema \
		insdc/sra.vschema \
		ncbi/ncbi.vschema \
		ncbi/seq.vschema \
		ncbi/spotname.vschema \
		ncbi/sra.vschema \
		ncbi/stats.vschema \
		ncbi/seq-graph.vschema \
		ncbi/wgs-contig.vschema \
		ncbi/clip.vschema \
		ncbi/varloc.vschema \
		ncbi/pnbrdb.vschema \
		vdb/built-in.vschema \
		vdb/vdb.vschema \
		align/mate-cache.vschema \
		align/refseq.vschema \
		align/seq.vschema \
		align/qstat.vschema \
		align/align.vschema \
		align/pileup-stats.vschema \
		sra/454.vschema \
		sra/ion-torrent.vschema \
		sra/illumina.vschema \
		sra/abi.vschema \
		sra/helicos.vschema \
		sra/nanopore.vschema \
		sra/generic-fastq.vschema \
		sra/pacbio.vschema \
		csra2/stats.vschema \
		csra2/read.vschema \
		csra2/reference.vschema \
		csra2/csra2.vschema \
	) \
	$(TOP)/libs/sra/built-in.vschema
#	$(TOP)/test/loader/sequencewriter.vschema
#	$(TOP)/interfaces/sra/pevents.vschema
	@echo "Schema diff test complete"

.PHONY: scantest parsetest difftestdiff
