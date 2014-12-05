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
	test-out \
    test-klib \

include $(TOP)/build/Makefile.env

$(TEST_TOOLS): makedirs
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
    
valgrind_asm:
	valgrind --ncbi $(TEST_BINDIR)/test-asm
    
.PHONY: valgrind_asm

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
    
valgrind_md5append:
	valgrind --ncbi $(TEST_BINDIR)/test-md5append
    
.PHONY: valgrind_md5append

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
    
valgrind_printf:
	valgrind --ncbi $(TEST_BINDIR)/test-printf
    
.PHONY: valgrind_printf


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
    
valgrind_out:
	valgrind --ncbi $(TEST_BINDIR)/test-out
    
.PHONY: valgrind_out

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

$(TEST_BINDIR)/test-klib: $(TEST_KLIB_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_KLIB_LIB)
    
valgrind_klib:
	valgrind --ncbi $(TEST_BINDIR)/test-klib
    
.PHONY: valgrind_klib


