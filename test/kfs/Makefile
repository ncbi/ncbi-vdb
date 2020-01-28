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
	kdf \
	test-kfs \
	test-ramfile \
	test-resolve \
	test-cachetee \
	test-cachetee2 \
	test-cachetee3 \
	test-lru-cache \
	test-kfs-md5    \
	test-appendfile

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
# df
#
DU_SRC = \
	kdf

DU_OBJ = \
	$(addsuffix .$(OBJX),$(DU_SRC))

$(TEST_BINDIR)/kdf: $(DU_OBJ)
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

ramfile: test-ramfile
	$(TEST_BINDIR)/test-ramfile

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
	rm *.o *.d

cachetee: test-cachetee
	$(TEST_BINDIR)/test-cachetee

#-------------------------------------------------------------------------------
# test-cachetee2
#
TEST_CACHETEE2_SRC = \
	cachetee2test

TEST_CACHETEE2_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_CACHETEE2_SRC))

TEST_CACHETEE2_LIB = \
	-lkapp \
    -sncbi-vdb \
	-lktst

$(TEST_BINDIR)/test-cachetee2: $(TEST_CACHETEE2_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_CACHETEE2_LIB)
	rm *.o *.d

cachetee2: test-cachetee2
	$(TEST_BINDIR)/test-cachetee2

#-------------------------------------------------------------------------------
# test-cachetee3
#
TEST_CACHETEE3_SRC = \
	cachetee3test

TEST_CACHETEE3_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_CACHETEE3_SRC))

TEST_CACHETEE3_LIB = \
	-lkapp \
    -sncbi-vdb \
	-lktst

$(TEST_BINDIR)/test-cachetee3: $(TEST_CACHETEE3_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_CACHETEE3_LIB)
#	rm *.o *.d

cachetee3: test-cachetee3
	rm -f cache.dat.cache org.dat
	$(TEST_BINDIR)/test-cachetee3
# $(TEST_BINDIR)/test-cachetee3 -vvv

#-------------------------------------------------------------------------------
# test-lru-cache
#
TEST_LRU_CACHE_SRC = \
	lru_cache_test

TEST_LRU_CACHE_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_LRU_CACHE_SRC))

TEST_LRU_CACHE_LIB = \
	-lkapp \
    -sncbi-vdb \
	-lktst

$(TEST_BINDIR)/test-lru-cache: $(TEST_LRU_CACHE_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_LRU_CACHE_LIB)
	rm *.o *.d

testlrucache: test-lru-cache
	$(TEST_BINDIR)/test-lru-cache

#-------------------------------------------------------------------------------
# fuse-proxy ( needed for the cachetee_out_of_space - test
#
FUSE_PROXY_SRC = \
	fuse_proxy

FUSE_PROXY_OBJ = \
	$(addsuffix .$(OBJX),$(FUSE_PROXY_SRC))

FUSE_PROXY_LIB = \
	-lfuse

$(TEST_BINDIR)/fuse-proxy: $(FUSE_PROXY_OBJ)
	$(LP) --exe -o $@ $^ $(FUSE_PROXY_LIB)
	rm *.o *.d

#-------------------------------------------------------------------------------
# cachetee_out_of_space
#
CTOFS_SRC = \
	cachetee-out-of-space

CTOFS_OBJ = \
	$(addsuffix .$(OBJX),$(CTOFS_SRC))

CTOFS_LIB = \
	-lkapp \
    -sncbi-vdb \
	-lktst

$(TEST_BINDIR)/test-cachetee-out-of-space: $(CTOFS_OBJ)
	$(LP) --exe -o $@ $^ $(CTOFS_LIB)
	rm *.o *.d

#-------------------------------------------------------------------------------
# simple_q_test
#
SIMPLE_Q_TEST_SRC = \
	simple_q_test

SIMPLE_Q_TEST_OBJ = \
	$(addsuffix .$(OBJX),$(SIMPLE_Q_TEST_SRC))

SIMPLE_Q_TEST_LIB = \
	-lkapp \
    -sncbi-vdb \
	-lktst

$(TEST_BINDIR)/simple_q: $(SIMPLE_Q_TEST_OBJ)
	$(LP) --exe -o $@ $^ $(SIMPLE_Q_TEST_LIB)
	rm *.o *.d

simple_q: simple_q
	$(TEST_BINDIR)/simple_q

ifeq (linux,$(OS))
ifeq (1,$(HAVE_FUSE))
#-------------------------------------------------------------------------------
# scripted tests
#
runtests: cachetee-out-of-space-test

cachetee-out-of-space-test: $(TEST_BINDIR)/fuse-proxy $(TEST_BINDIR)/test-cachetee-out-of-space run-cachetee-out-of-space-test.sh
	@ echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@ echo "Running $(SRCDIR)/run-cachetee-out-of-space-test.sh"
	@ $(SRCDIR)/run-cachetee-out-of-space-test.sh $(SRCDIR) $(TEST_BINDIR)
endif
endif

#-------------------------------------------------------------------------------
# test-appendfile
#
TEST_APPENDFILE_SRC = \
	appendfiletest

TEST_APPENDFILE_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_APPENDFILE_SRC))

TEST_APPENDFILE_LIB = \
	-lkapp \
    -sncbi-vdb \
	-lktst

$(TEST_BINDIR)/test-appendfile: $(TEST_APPENDFILE_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_APPENDFILE_LIB)

