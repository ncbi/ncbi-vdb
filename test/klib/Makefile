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

MODULE = test/klib

# WARNING: test-md5append is excluded from TEST_TOOLS since it's supposed
# to be run manually
TEST_TOOLS = \
	test-asm \
	test-printf \
	test-log \
	test-out \
	test-SraReleaseVersion \
	test-time \
	test-hashfile \
	test-hashtable \
	test-klib \
	test-vnamelist \
	test-progressbar \
    test-symtab \

include $(TOP)/build/Makefile.env

std: $(TEST_TOOLS)

test-md5append $(TEST_TOOLS): makedirs
	@ $(MAKE_CMD) $(TEST_BINDIR)/$@

.PHONY: $(TEST_TOOLS)

clean: stdclean

#-------------------------------------------------------------------------------
# test-asm
#
TEST_ASM_SRC = \
	asm-test

TEST_ASM_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_ASM_SRC))

TEST_ASM_LIB = \
	-skapp \
	-sncbi-vdb \
	-sktst \

$(TEST_BINDIR)/test-asm: $(TEST_ASM_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_ASM_LIB)

#-------------------------------------------------------------------------------
# test-md5append
#
TEST_MD5APPEND_SRC = \
	md5appendtest

TEST_MD5APPEND_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_MD5APPEND_SRC))

TEST_MD5APPEND_LIB = \
	-skapp \
	-sncbi-vdb \
	-sktst \

$(TEST_BINDIR)/test-md5append: $(TEST_MD5APPEND_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_MD5APPEND_LIB)

#-------------------------------------------------------------------------------
# test-printf
#
TEST_PRINTF_SRC = \
	printf-test

TEST_PRINTF_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_PRINTF_SRC))

TEST_PRINTF_LIB = \
	-skapp \
	-sncbi-vdb \
	-sktst \

$(TEST_BINDIR)/test-printf: $(TEST_PRINTF_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_PRINTF_LIB)

#-------------------------------------------------------------------------------
# test-log
#
TEST_LOG_SRC = \
	test-log

TEST_LOG_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_LOG_SRC))

TEST_LOG_LIB = \
	-skapp \
	-sncbi-vdb \
	-sktst \

$(TEST_BINDIR)/test-log: $(TEST_LOG_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_LOG_LIB)

#-------------------------------------------------------------------------------
# test-SraReleaseVersion
#
TEST_SRA_RELEASE_VERSION_SRC = \
	test-SraReleaseVersion

TEST_SRA_RELEASE_VERSION_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_SRA_RELEASE_VERSION_SRC))

TEST_SRA_RELEASE_VERSION_LIB = \
	-skapp \
	-sncbi-vdb \
	-sktst \


$(TEST_BINDIR)/test-SraReleaseVersion: $(TEST_SRA_RELEASE_VERSION_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_SRA_RELEASE_VERSION_LIB)

valgrind_sra_release_version:
	valgrind --ncbi $(TEST_BINDIR)/test-SraReleaseVersion

.PHONY: valgrind_SraReleaseVersion


#-------------------------------------------------------------------------------
# test-time
#
TEST_TIME_SRC = \
	test-time

TEST_TIME_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_TIME_SRC))
    
$(TEST_BINDIR)/test-time: $(TEST_TIME_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_SRA_RELEASE_VERSION_LIB)

#-------------------------------------------------------------------------------
# test-out
#
TEST_OUT_SRC = \
	test-out

TEST_OUT_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_OUT_SRC))

TEST_OUT_LIB = \
	-sncbi-vdb \
	-sktst \

$(TEST_BINDIR)/test-out: $(TEST_OUT_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_OUT_LIB)

#-------------------------------------------------------------------------------
# test-klib
#
TEST_KLIB_SRC = \
	test-klib

TEST_KLIB_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_KLIB_SRC))

TEST_KLIB_LIB = \
	-skapp \
	-sncbi-vdb \
	-sktst \
	-sklib

$(TEST_BINDIR)/test-klib: $(TEST_KLIB_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_KLIB_LIB)

#-------------------------------------------------------------------------------
# test-hashfile
#
TEST_HASHFILE_SRC = \
	test-hashfile

TEST_HASHFILE_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_HASHFILE_SRC))

TEST_HASHFILE_LIB = \
	-skapp \
	-sncbi-vdb \
	-sktst \
	-sklib

$(TEST_BINDIR)/test-hashfile: $(TEST_HASHFILE_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_HASHFILE_LIB)


#-------------------------------------------------------------------------------
# test-hashtable
#
TEST_HASHTABLE_SRC = \
	test-hashtable

TEST_HASHTABLE_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_HASHTABLE_SRC))

TEST_HASHTABLE_LIB = \
	-skapp \
	-sncbi-vdb \
	-sktst \
	-sklib

$(TEST_BINDIR)/test-hashtable: $(TEST_HASHTABLE_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_HASHTABLE_LIB)


#-------------------------------------------------------------------------------
# test-vnamelist
#
TEST_VNAMELIST_SRC = \
	test-vnamelist

TEST_VNAMELIST_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_VNAMELIST_SRC))

TEST_VNAMELIST_LIB = \
	-skapp \
	-sncbi-vdb \
	-sktst \

$(TEST_BINDIR)/test-vnamelist: $(TEST_VNAMELIST_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_VNAMELIST_LIB)

VNAMELIST: test-vnamelist
	$(TEST_BINDIR)/test-vnamelist

#-------------------------------------------------------------------------------
# test-progressbar
#
TEST_PROGRESSBAR_SRC = \
	test-progress

TEST_PROGRESSBAR_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_PROGRESSBAR_SRC))

TEST_PROGRESSBAR_LIB = \
	-skapp \
	-sncbi-vdb \
	-sktst \

$(TEST_BINDIR)/test-progressbar: $(TEST_PROGRESSBAR_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_PROGRESSBAR_LIB)

PROGRESSBAR: test-progressbar
	$(TEST_BINDIR)/test-progressbar

#-------------------------------------------------------------------------------
# test-symtab
#
TEST_SYMTAB_SRC = \
	test-symtab

TEST_SYMTAB_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_SYMTAB_SRC))

TEST_SYMTAB_LIB = \
	-skapp \
	-sncbi-vdb \
	-sktst \

$(TEST_BINDIR)/test-symtab: $(TEST_SYMTAB_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_SYMTAB_LIB)

symtab: test-symtab
	$(TEST_BINDIR)/test-symtab
