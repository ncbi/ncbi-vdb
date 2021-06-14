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
#	Tests for the VDB3 platform library
#
# to build/run all test projects, use target 'test' or 'check'

default: test

TOP ?= $(abspath ../../..)
MODULE = platform/memory/test

TESTS = test-memory

include $(TOP)/platform/Makefile.test

#
# subdirectory-specific part
#

# project memory
SRC = \
	../MemoryBlockItf \
	../MemoryManagerItf \
	../TrackingMemoryManagerItf \
	../FillingMemoryMgr \
	../PinnedMemoryMgr \
	../QuotaMemoryMgr \
	../SharedMemoryMgr \
	../UniqueRawMemoryBlock \
	../PrimordialMemoryMgr \
	../RawMemoryBlock \
	../TypedMemoryBlock \
	test-main \
	test-MemoryManagerItf \
	test-MemoryBlockItf \
	test-TrackingMemoryManager \
	test-PrimordialMemoryMgr \
	test-FillingMemoryMgr \
	test-PinnedMemoryMgr \
	test-QuotaMemoryMgr \
	test-SharedMemoryMgr \
	test-UniqueRawMemoryBlock \
	test-RawMemoryBlock \
	test-TypedMemoryBlock \


OBJ = $(addsuffix .$(OBJX),$(SRC))

$(TEST_BINDIR)/test-memory: $(OBJ)
	$(LP) $(OBJ) -o $@ -lgtest -lpthread

