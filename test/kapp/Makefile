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

#TODO: update qfiletest

default: runtests

TOP ?= $(abspath ../..)
MODULE = test/kapp

TEST_TOOLS = \
    test-qfile \
    test-kapp

ALL_TOOLS = \
	env-test-tool \
	sig-core \
	report-kns-agent \
	rc-mod-exitcode \
	append-arg-test

include $(TOP)/build/Makefile.env

$(TEST_TOOLS): makedirs
	@ $(MAKE_CMD) $(TEST_BINDIR)/$@

$(ALL_TOOLS): makedirs
	@ $(MAKE_CMD) $(TEST_BINDIR)/$@

.PHONY: $(ALL_LIBS) $(ALL_TOOLS) $(TEST_TOOLS)

clean: stdclean

std: $(ALL_TOOLS)

#-------------------------------------------------------------------------------
# kapp-test
#
ARGS_TEST_SRC = \
	kapp-test

ARGS_TEST_OBJ = \
	$(addsuffix .$(OBJX),$(ARGS_TEST_SRC))

ARGS_TEST_LIB = \
    -sktst \
	-skapp \
    -sload \
    -sncbi-wvdb \

$(TEST_BINDIR)/test-kapp: $(ARGS_TEST_OBJ)
	$(LP) --exe -o $@ $^ $(ARGS_TEST_LIB)

#-------------------------------------------------------------------------------
# test-qfile
#
QFILETEST_SRC = \
	qfiletest

QFILETEST_OBJ = \
	$(addsuffix .$(OBJX),$(QFILETEST_SRC))

QFILETEST_LIB = \
	-skapp \
    -sload \
    -sncbi-wvdb \


$(TEST_BINDIR)/test-qfile: $(QFILETEST_OBJ)
	$(LD) --exe $(SRCDIR) -o $@ $^ $(QFILETEST_LIB)

#-------------------------------------------------------------------------------
# env-test-tool
#
ENV_TEST_SRC = \
	env-test-tool

ENV_TEST_OBJ = \
	$(addsuffix .$(OBJX),$(ENV_TEST_SRC))

ENV_TEST_LIB = \
	-skapp \
	-sncbi-vdb \
	-sload \
	-lm

$(TEST_BINDIR)/env-test-tool: $(ENV_TEST_OBJ)
	$(LD) --exe -o $@ $^ $(ENV_TEST_LIB)

#-------------------------------------------------------------------------------
# sig-core
#
SIG_CORE_TEST_SRC = \
	sig-core

SIG_CORE_TEST_OBJ = \
	$(addsuffix .$(OBJX),$(SIG_CORE_TEST_SRC))

SIG_CORE_TEST_LIB = \
	-skapp \
	-sncbi-vdb \
	-sload \
	-lm

$(TEST_BINDIR)/sig-core: $(SIG_CORE_TEST_OBJ)
	$(LD) --exe -o $@ $^ $(SIG_CORE_TEST_LIB)

#-------------------------------------------------------------------------------
# report-kns-agent
#
REPORT_KNS_AGENT_TEST_SRC = \
	report-kns-agent

REPORT_KNS_AGENT_TEST_OBJ = \
	$(addsuffix .$(OBJX),$(REPORT_KNS_AGENT_TEST_SRC))

REPORT_KNS_AGENT_TEST_LIB = \
	-skapp \
	-sncbi-vdb

$(TEST_BINDIR)/report-kns-agent: $(REPORT_KNS_AGENT_TEST_OBJ)
	$(LD) --exe -o $@ $^ $(REPORT_KNS_AGENT_TEST_LIB)

#-------------------------------------------------------------------------------
# append-arg-test
#
APPEND_ARG_TEST_SRC = \
	append-arg-test

APPEND_ARG_TEST_OBJ = \
	$(addsuffix .$(OBJX),$(APPEND_ARG_TEST_SRC))

APPEND_ARG_TEST_LIB = \
	-skapp \
	-sncbi-vdb

$(TEST_BINDIR)/append-arg-test: $(APPEND_ARG_TEST_OBJ)
	$(LD) --exe -o $@ $^ $(APPEND_ARG_TEST_LIB)


#-------------------------------------------------------------------------------
# rc-mod-exitcode
#
RC_MOD_EXITCODE_TEST_SRC = \
	rc-mod-exitcode

RC_MOD_EXITCODE_TEST_OBJ = \
	$(addsuffix .$(OBJX),$(RC_MOD_EXITCODE_TEST_SRC))

RC_MOD_EXITCODE_TEST_LIB = \
	-skapp \
	-sncbi-vdb

$(TEST_BINDIR)/rc-mod-exitcode: $(RC_MOD_EXITCODE_TEST_OBJ)
	$(LD) --exe -o $@ $^ $(RC_MOD_EXITCODE_TEST_LIB)

#-------------------------------------------------------------------------------
# scripted tests
#
runtests: kns-agent-tests rc-exitcode-tests append-arg-tests

sig-core-tests: sig-core
	@ echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@ echo "Running $(SRCDIR)/run-sig-core.sh $(TEST_BINDIR)/sig-core"
	@ $(SRCDIR)/run-sig-core.sh $(TEST_BINDIR)/sig-core $(HOST_OS) $(BUILD)

kns-agent-tests: report-kns-agent
	@ echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@ echo "Running kns-agent tests"
	@ # simple tool name
	@ export PATH=./tmp/export:$$PATH && $(SRCDIR)/run-kns-agent.sh $(TEST_BINDIR)/report-kns-agent ./tmp/export/a.123.99 a.1.2.3 a.123.99
	@ export PATH=./tmp/export:$$PATH && $(SRCDIR)/run-kns-agent.sh $(TEST_BINDIR)/report-kns-agent ./tmp/export/abcdefghi abcdefghi.1.2.3 abcdefghi
	@ # tool path where directory path length >= tool name length
	@ $(SRCDIR)/run-kns-agent.sh $(TEST_BINDIR)/report-kns-agent ./tmp/path/tool.99.123 tool.1.2.3
	@ $(SRCDIR)/run-kns-agent.sh $(TEST_BINDIR)/report-kns-agent ./tmp/path/tool tool.1.2.3
	@ $(SRCDIR)/run-kns-agent.sh $(TEST_BINDIR)/report-kns-agent tmp/path/tool tool.1.2.3
	@ $(SRCDIR)/run-kns-agent.sh $(TEST_BINDIR)/report-kns-agent ../kapp/tmp/path/tool tool.1.2.3
	@ $(SRCDIR)/run-kns-agent.sh $(TEST_BINDIR)/report-kns-agent tmp/tool tool.1.2.3
	@ # tool path where directory path length < tool name length
	@ $(SRCDIR)/run-kns-agent.sh $(TEST_BINDIR)/report-kns-agent ./tmp/abcdefghi abcdefghi.1.2.3
	@ $(SRCDIR)/run-kns-agent.sh $(TEST_BINDIR)/report-kns-agent ./tmp/abcdefghi.55.111 abcdefghi.1.2.3
	@ rm -rf ./tmp/

rc-exitcode-tests: rc-mod-exitcode
	@ echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@ echo "Running RC module EXITCODE tests" 
	@ perl $(SRCDIR)/exitcode.pl $(TEST_BINDIR)/rc-mod-exitcode 75

append-arg-tests:   append-arg-test
	@ echo "Running append argument tests"
	@ $(SRCDIR)/run-append-arg.sh $(TEST_BINDIR)/append-arg-test $(DBG)
	@ echo "JOJOBA!!!"
