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

MODULE = test/vfs

TEST_TOOLS = \
	test-redirect-rejected-names-cgi-http-to-https \
	test-resolver-with-log \
	test-sdl-response \
	test-services \
	test-caching \
	test-names-30 \
	test-path \
	test-path-c \
	test-perm \
	test-resolver \
	test-vfsmanager \
	test-json-response \
	test-resolver4 \

include $(TOP)/build/Makefile.env

$(TEST_TOOLS): makedirs
	@ $(MAKE_CMD) $(TEST_BINDIR)/$@

.PHONY: $(TEST_TOOLS)

vg: $(addprefix vg-,$(TEST_TOOLS))

clean: stdclean

#-------------------------------------------------------------------------------
# C test on VPath (aka test-vfs on Windows)
#

TEST_SRC = \
	path-test

TEST_OBJ = \
	$(addsuffix .$(OBJX),$(TEST_SRC))

TEST_LIB = \
	-skapp \
	-sncbi-vdb \

$(TEST_BINDIR)/test-path-c: $(TEST_OBJ)
	$(LD) --exe -o $@ $^ $(TEST_LIB)

#----------------------------------------------------------------
# C++ test on VPath
#

PATHTEST_SRC = \
	pathtest

PATHTEST_OBJ = \
	$(addsuffix .$(OBJX),$(PATHTEST_SRC))

PATHTEST_LIB = \
	-skapp \
	-sktst \
	-sncbi-vdb \

$(TEST_BINDIR)/test-path: $(PATHTEST_OBJ)
	$(LP) --exe -o $@ $^ $(PATHTEST_LIB)

#----------------------------------------------------------------
HTTPS_SRC = \
	redirect-rejected-names-cgi-http-to-https

HTTPS_OBJ = \
	$(addsuffix .$(OBJX),$(HTTPS_SRC))

$(TEST_BINDIR)/test-redirect-rejected-names-cgi-http-to-https: $(HTTPS_OBJ)
	$(LP) --exe -o $@ $^ $(PATHTEST_LIB)

#----------------------------------------------------------------
# resolver4

R4_SRC = \
	test-resolver4

R4_OBJ = \
	$(addsuffix .$(OBJX),$(R4_SRC))

$(TEST_BINDIR)/test-resolver4: $(R4_OBJ)
	$(LP) --exe -o $@ $^ $(PATHTEST_LIB)

#----------------------------------------------------------------
# json-response

J_SRC = \
	test-json-response

J_OBJ = \
	$(addsuffix .$(OBJX),$(J_SRC))

$(TEST_BINDIR)/test-json-response: $(J_OBJ)
	$(LP) --exe -o $@ $^ $(PATHTEST_LIB)

#----------------------------------------------------------------
# sdl-response

S_SRC = \
	test-sdl-response

S_OBJ = \
	$(addsuffix .$(OBJX),$(S_SRC))

$(TEST_BINDIR)/test-sdl-response: $(S_OBJ)
	$(LP) --exe -o $@ $^ $(PATHTEST_LIB)

#----------------------------------------------------------------
# test-caching

CACHING_SRC = \
	test-caching

CACHING_OBJ = \
	$(addsuffix .$(OBJX),$(CACHING_SRC))

$(TEST_BINDIR)/test-caching: $(CACHING_OBJ)
	$(LP) --exe -o $@ $^ $(PATHTEST_LIB)

caching: test-caching
	$(TEST_BINDIR)/test-caching

#----------------------------------------------------------------
# test-names-30

N30_SRC = \
	test-names-30

N30_OBJ = \
	$(addsuffix .$(OBJX),$(N30_SRC))

$(TEST_BINDIR)/test-names-30: $(N30_OBJ)
	$(LP) --exe -o $@ $^ $(PATHTEST_LIB)

#----------------------------------------------------------------
# test-perm

TP_SRC = \
	test-perm

TP_OBJ = \
	$(addsuffix .$(OBJX),$(TP_SRC))

$(TEST_BINDIR)/test-perm: $(TP_OBJ)
	$(LP) --exe -o $@ $^ $(PATHTEST_LIB)

#----------------------------------------------------------------
# C++ test on VFSManager
#

MANAGERTEST_SRC = \
	managertest

MANAGERTEST_OBJ = \
	$(addsuffix .$(OBJX),$(MANAGERTEST_SRC))

MANAGERTEST_LIB = \
	-skapp \
	-sktst \
	-sncbi-vdb \

$(TEST_BINDIR)/test-vfsmanager: $(MANAGERTEST_OBJ)
	$(LP) --exe -o $@ $^ $(MANAGERTEST_LIB)


#----------------------------------------------------------------
# C++ test on VResolver
#

RESOLVERTEST_SRC = \
	resolvertest

RESOLVERTEST_OBJ = \
	$(addsuffix .$(OBJX),$(RESOLVERTEST_SRC))

RESOLVERTEST_LIB = \
	-skapp \
	-sktst \
	-sncbi-vdb \

$(TEST_BINDIR)/test-resolver: $(RESOLVERTEST_OBJ)
	$(LP) --exe -o $@ $^ $(RESOLVERTEST_LIB)

resolver: test-resolver
	$(TEST_BINDIR)/test-resolver

#----------------------------------------------------------------
# Test of names service
#

RESOLVERTESTLOG_SRC = \
	test-resolver-with-log

RESOLVERTESTLOG_OBJ = \
	$(addsuffix .$(OBJX),$(RESOLVERTESTLOG_SRC))

$(TEST_BINDIR)/test-resolver-with-log: $(RESOLVERTESTLOG_OBJ)
	$(LP) --exe -o $@ $^ $(RESOLVERTEST_LIB)

#----------------------------------------------------------------
# Test of names service
#
SERVICES_SRC = \
	test-services

SERVICES_OBJ = \
	$(addsuffix .$(OBJX),$(SERVICES_SRC))

$(TEST_BINDIR)/test-services: $(SERVICES_OBJ)
	$(LP) --exe -o $@ $^ $(RESOLVERTEST_LIB)
