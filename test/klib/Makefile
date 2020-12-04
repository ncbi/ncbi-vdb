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
	test-data-buffer-print \
	test-log \
	test-out \
	test-SraReleaseVersion \
	test-time \
	test-rcenum \
	test-klib \
	test-KOutMsgOverflow \
	test-vnamelist \
	test-progressbar \
    test-symtab \
	test-json \
	test-base64 \
	test-guid \

ifeq (linux,$(OS))
TEST_TOOLS += test-hashfile
TEST_TOOLS += test-hashtable
endif

include $(TOP)/build/Makefile.env

std: $(TEST_TOOLS)

test-md5append test-json-parser $(TEST_TOOLS): makedirs
	@ $(MAKE_CMD) $(TEST_BINDIR)/$@

.PHONY: $(TEST_TOOLS)

clean: stdclean

TEST_KLIB_LIB = \
	-skapp \
	-sktst \
	-sncbi-vdb \

#-------------------------------------------------------------------------------
# test-asm
#
TEST_ASM_SRC = \
	asm-test

TEST_ASM_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_ASM_SRC))

$(TEST_BINDIR)/test-asm: $(TEST_ASM_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_KLIB_LIB)

#-------------------------------------------------------------------------------
# test-md5append
#
TEST_MD5APPEND_SRC = \
	md5appendtest

TEST_MD5APPEND_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_MD5APPEND_SRC))

$(TEST_BINDIR)/test-md5append: $(TEST_MD5APPEND_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_KLIB_LIB)

#-------------------------------------------------------------------------------
# test-printf
#
TEST_PRINTF_SRC = \
	printf-test

TEST_PRINTF_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_PRINTF_SRC))

$(TEST_BINDIR)/test-printf: $(TEST_PRINTF_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_KLIB_LIB)

#-------------------------------------------------------------------------------
# test-printf
#
TEST_BUF_PRINT_SRC = \
	test-data-buffer-print

TEST_BUF_PRINT_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_BUF_PRINT_SRC))

$(TEST_BINDIR)/test-data-buffer-print: $(TEST_BUF_PRINT_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_KLIB_LIB)

#-------------------------------------------------------------------------------
# test-log
#
TEST_LOG_SRC = \
	test-log

TEST_LOG_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_LOG_SRC))

$(TEST_BINDIR)/test-log: $(TEST_LOG_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_KLIB_LIB)

#-------------------------------------------------------------------------------
# test-SraReleaseVersion
#
TEST_SRA_RELEASE_VERSION_SRC = \
	test-SraReleaseVersion

TEST_SRA_RELEASE_VERSION_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_SRA_RELEASE_VERSION_SRC))

$(TEST_BINDIR)/test-SraReleaseVersion: $(TEST_SRA_RELEASE_VERSION_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_KLIB_LIB)

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
	$(LP) --exe -o $@ $^ $(TEST_KLIB_LIB)

#-------------------------------------------------------------------------------
# test-rcenum
#
TEST_RCENUM_SRC = \
	test-rcenum

TEST_RCENUM_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_RCENUM_SRC))

$(TEST_BINDIR)/test-rcenum: $(TEST_RCENUM_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_KLIB_LIB)

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

$(TEST_BINDIR)/test-klib: $(TEST_KLIB_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_KLIB_LIB)

klib: test-klib
	$(TEST_BINDIR)/test-klib

#-------------------------------------------------------------------------------
# test-KOutMsgOverflow
TEST_OMO_SRC = \
	test-KOutMsgOverflow
TEST_OMO_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_OMO_SRC))
$(TEST_BINDIR)/test-KOutMsgOverflow: $(TEST_OMO_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_KLIB_LIB)
KOutMsgOverflow: test-klib
	$(TEST_BINDIR)/KOutMsgOverflow

#-------------------------------------------------------------------------------
# test-hashfile
#
TEST_HASHFILE_SRC = \
	test-hashfile

TEST_HASHFILE_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_HASHFILE_SRC))

$(TEST_BINDIR)/test-hashfile: $(TEST_HASHFILE_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_KLIB_LIB)


#-------------------------------------------------------------------------------
# test-hashtable
#
TEST_HASHTABLE_SRC = \
	test-hashtable

TEST_HASHTABLE_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_HASHTABLE_SRC))

$(TEST_BINDIR)/test-hashtable: $(TEST_HASHTABLE_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_KLIB_LIB)


#-------------------------------------------------------------------------------
# test-vnamelist
#
TEST_VNAMELIST_SRC = \
	test-vnamelist

TEST_VNAMELIST_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_VNAMELIST_SRC))

$(TEST_BINDIR)/test-vnamelist: $(TEST_VNAMELIST_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_KLIB_LIB)

VNAMELIST: test-vnamelist
	$(TEST_BINDIR)/test-vnamelist

.phony: VNAMELIST

#-------------------------------------------------------------------------------
# test-progressbar
#
TEST_PROGRESSBAR_SRC = \
	test-progress

TEST_PROGRESSBAR_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_PROGRESSBAR_SRC))

$(TEST_BINDIR)/test-progressbar: $(TEST_PROGRESSBAR_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_KLIB_LIB)

PROGRESSBAR: test-progressbar
	$(TEST_BINDIR)/test-progressbar

.phony: PROGRESSBAR

#-------------------------------------------------------------------------------
# test-symtab
#
TEST_SYMTAB_SRC = \
	test-symtab

TEST_SYMTAB_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_SYMTAB_SRC))

$(TEST_BINDIR)/test-symtab: $(TEST_SYMTAB_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_KLIB_LIB)

symtab: test-symtab
	$(TEST_BINDIR)/test-symtab

.phony: symtab

#-------------------------------------------------------------------------------
# test-json
#
TEST_JSON_SRC = \
	test-json

TEST_JSON_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_JSON_SRC))

$(TEST_BINDIR)/test-json: $(TEST_JSON_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_KLIB_LIB)

json: test-json
	$(TEST_BINDIR)/test-json

.phony: json

#-------------------------------------------------------------------------------
# test-json-parser, command line tests
#
TEST_JSON_PARSER_SRC = \
	test-json-parser

TEST_JSON_PARSER_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_JSON_PARSER_SRC))

$(TEST_BINDIR)/test-json-parser: $(TEST_JSON_PARSER_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_KLIB_LIB)

json-parser: test-json-parser json
	@ echo ++++++++++++++++++++++++++++++++++++++++++++++++++++++
	@ echo Run $(TEST_BINDIR)/test-json-parser
	@ $(TEST_BINDIR)/test-json-parser $(shell find $(SRCDIR)/test-json-parser/ -name \*.json)

runtests: json-parser

.phony: json-parser

#-------------------------------------------------------------------------------
# fuzz testing json parser
#
FUZZ_JSON_SRC = \
	fuzz-json

FUZZ_JSON_OBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .fuzz.$(OBJX),$(FUZZ_JSON_SRC)))

$(TEST_BINDIR)/fuzz-json: $(FUZZ_JSON_OBJ) $(FLIBDIR)/libncbi-vdb.a
	clang++ -o $@ $^ $(FUZZ_LIB) $(FUZZ_OPT)

fuzz-json: $(TEST_BINDIR)/fuzz-json

FUZZ_SEEDS ?= test-json-parser
FUZZ_DICT ?= -dict=fuzz-dict
RUNS ?= -1

runfuzz $(FUZZ_PROF_RAW): makedirs $(TEST_BINDIR)/fuzz-json
	 mkdir -p fuzz-corpus
	 export LLVM_PROFILE_FILE=$(FUZZ_PROF_RAW); $(TEST_BINDIR)/fuzz-json $(FUZZ_DICT) -runs=$(RUNS) fuzz-corpus $(FUZZ_SEEDS)

.phony: runfuzz

#-------------------------------------------------------------------------------
# test-base64
#
TEST_BASE64_SRC = \
	test-base64

TEST_BASE64_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_BASE64_SRC))

$(TEST_BINDIR)/test-base64: $(TEST_BASE64_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_KLIB_LIB)

base64: test-base64
	$(TEST_BINDIR)/test-base64

.phony: base64

#-------------------------------------------------------------------------------
# test-guid
#
TEST_GUID_SRC = \
	test-guid

TEST_GUID_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_GUID_SRC))

$(TEST_BINDIR)/test-guid: $(TEST_GUID_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_KLIB_LIB)

guid: test-guid
	$(TEST_BINDIR)/test-guid

.phony: guid
