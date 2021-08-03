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

#
#	Test for the VDB3 build system, unit test project for a subdirectory of platform/
#
# to build/run all test projects, use target 'test' or 'check'

default: test

TOP ?= $(abspath ../../..)
MODULE = platform/hello/test

TESTS = test-hello1 test-hello2

include $(TOP)/platform/Makefile.test

#
# subdirectory-specific part
#

# project hello1
SRC_1 = test-hello1

OBJ_1 = $(addsuffix .$(OBJX),$(SRC_1))

$(TEST_BINDIR)/test-hello1: $(OBJ_1)
	$(LP) $(OBJ_1) -o $@

# build/run a single project
hello1: test-hello1
	$(TEST_BINDIR)/test-hello1

# project hello2
SRC_2 = test-hello2

OBJ_2 = $(addsuffix .$(OBJX),$(SRC_2))

$(TEST_BINDIR)/test-hello2: $(OBJ_2)
	$(LP) $(OBJ_2) -o $@

# build/run a single project
hello2: test-hello2
	$(TEST_BINDIR)/test-hello2 2
