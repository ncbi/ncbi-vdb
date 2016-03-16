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

MODULE = test/kfg

TEST_TOOLS = \
	wb-test-kfg \
	test-kfg \
	test-repository \
	test-keystore

include $(TOP)/build/Makefile.env

$(TEST_TOOLS): makedirs
	@ $(MAKE_CMD) $(TEST_BINDIR)/$@

.PHONY: $(TEST_TOOLS)

clean: stdclean

#-------------------------------------------------------------------------------
# ncbi-home-from-env

ENV_SRC = \
	ncbi-home-from-env

ENV_OBJ = \
	$(addsuffix .$(OBJX),$(ENV_SRC))

$(TEST_BINDIR)/ncbi-home-from-env: $(ENV_OBJ)
	$(LP) --exe -o $@ $^ $(ENV_LIB) 

#-------------------------------------------------------------------------------
# white-box test
#
TEST_SRC = \
	wb-test-kfg

TEST_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_SRC))

TEST_LIB = \
	-sktst \
	-skapp \
    -sncbi-vdb

$(TEST_BINDIR)/wb-test-kfg: $(TEST_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_LIB) 

#-------------------------------------------------------------------------------
# test-kfg
#
TEST_KFG_SRC = \
	kfgtest 

TEST_KFG_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_KFG_SRC))

TEST_KFG_LIB = \
	-sktst \
	-skapp \
    -sncbi-vdb

$(TEST_BINDIR)/test-kfg: $(TEST_KFG_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_KFG_LIB)

#-------------------------------------------------------------------------------
# test-repository
#
TEST_REPOSITORY_SRC = \
	repositorytest 

TEST_REPOSITORY_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_REPOSITORY_SRC))

TEST_REPOSITORY_LIB = \
	-sktst \
	-skapp \
    -sncbi-vdb

$(TEST_BINDIR)/test-repository: $(TEST_REPOSITORY_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_REPOSITORY_LIB)

#-------------------------------------------------------------------------------
# test-keystore
#
TEST_KEYSTORE_SRC = \
	keystoretest 

TEST_KEYSTORE_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_KEYSTORE_SRC))

TEST_KEYSTORE_LIB = \
	-sktst \
	-skapp \
    -sncbi-vdb

$(TEST_BINDIR)/test-keystore: $(TEST_KEYSTORE_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_KEYSTORE_LIB)

