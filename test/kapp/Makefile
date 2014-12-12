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
    args-test
    
#	TEST_TOOLS += qfiletest

SLOW_TEST_TOOLS = args-test

include $(TOP)/build/Makefile.env

$(TEST_TOOLS): makedirs
	@ $(MAKE_CMD) $(TEST_BINDIR)/$@

.PHONY: $(ALL_LIBS) $(TEST_TOOLS)

clean: stdclean

#-------------------------------------------------------------------------------
# args-test
#
ARGS_TEST_SRC = \
	args-test

ARGS_TEST_OBJ = \
	$(addsuffix .$(OBJX),$(ARGS_TEST_SRC))

ARGS_TEST_LIB = \
	-lkapp \
    -lktst \
    -lload \
    -lncbi-wvdb \

$(TEST_BINDIR)/args-test: $(ARGS_TEST_OBJ)
	$(LP) --exe -o $@ $^ $(ARGS_TEST_LIB)

#-------------------------------------------------------------------------------
# test-byteorder
#
BYTEORDER_SRC = \
	test-byteorder

BYTEORDER_OBJ = \
	$(addsuffix .$(OBJX),$(BYTEORDER_SRC))

BYTEORDER_LIB = \

$(BINDIR)/test-byteorder: $(addprefix $(SRCDIR)/,$(BYTEORDER_COL))

.PHONY: test-byteorder

test-byteorder: $(BINDIR)/test-byteorder

$(BINDIR)/test-byteorder: $(BYTEORDER_OBJ)
	$(LD) --exe -o $@ $^ $(BYTEORDER_LIB)

#-------------------------------------------------------------------------------
# qfiletest
#
QFILETEST_SRC = \
	qfiletest

QFILETEST_OBJ = \
	$(addsuffix .$(OBJX),$(QFILETEST_SRC))

QFILETEST_LIB = \
	-lkapp \
    -lload \
    -lncbi-wvdb \


$(TEST_BINDIR)/qfiletest: $(QFILETEST_OBJ)
	$(LD) --exe $(SRCDIR) -o $@ $^ $(QFILETEST_LIB)
