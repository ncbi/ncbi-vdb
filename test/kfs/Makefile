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

MODULE = test/kfs

TEST_TOOLS = \
	test-kfs \
    test-ramfile \
    test-resolve \
    test-cachetee \
	test-kfs-md5

include $(TOP)/build/Makefile.env

$(TEST_TOOLS): makedirs
	@ $(MAKE_CMD) $(TEST_BINDIR)/$@

.PHONY: $(TEST_TOOLS)

clean: stdclean

#-------------------------------------------------------------------------------
# test-kfs
#
TEST_KFS_SRC = \
	kfstest 

TEST_KFS_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_KFS_SRC))

TEST_KFS_LIB = \
	-lkapp \
    -sncbi-vdb \
	-lktst

ifeq (1,$(HAVE_MAGIC))
	TEST_KFS_LIB += -skff -lmagic
	CPPFLAGS += -DHAVE_KFF
endif

$(TEST_BINDIR)/test-kfs: $(TEST_KFS_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_KFS_LIB)

#-------------------------------------------------------------------------------
# test-kfs-md5
#
TEST_MD5_SRC = \
	md5test 

TEST_MD5_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_MD5_SRC))

TEST_MD5_LIB = \
	-skapp \
    -sncbi-vdb \
	-sktst

$(TEST_BINDIR)/test-kfs-md5: $(TEST_MD5_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_MD5_LIB)


#-------------------------------------------------------------------------------
# test-ramfile
#
TEST_RAMFILE_SRC = \
	ramfiletest 

TEST_RAMFILE_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_RAMFILE_SRC))

TEST_RAMFILE_LIB = \
	-lkapp \
    -sncbi-vdb \
	-lktst

$(TEST_BINDIR)/test-ramfile: $(TEST_RAMFILE_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_RAMFILE_LIB)

#-------------------------------------------------------------------------------
# test-resolve
#
TEST_RESOLVE_SRC = \
	resolvetest 

TEST_RESOLVE_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_RESOLVE_SRC))

TEST_RESOLVE_LIB = \
	-lkapp \
    -sncbi-vdb \
	-lktst

$(TEST_BINDIR)/test-resolve: $(TEST_RESOLVE_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_RESOLVE_LIB)

#-------------------------------------------------------------------------------
# test-cachetee
#
TEST_CACHETEE_SRC = \
	cacheteetest

TEST_CACHETEE_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_CACHETEE_SRC))

TEST_CACHETEE_LIB = \
	-lkapp \
    -sncbi-vdb \
	-lktst

$(TEST_BINDIR)/test-cachetee: $(TEST_CACHETEE_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_CACHETEE_LIB)
