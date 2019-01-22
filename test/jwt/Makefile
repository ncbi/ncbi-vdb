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

MODULE = test/jwt

TEST_TOOLS = \
	test-jwt-string \
	test-jwt-json \
	test-jwt \
	test-jwt-c \

include $(TOP)/build/Makefile.env

$(TEST_TOOLS): makedirs
	@ $(MAKE_CMD) $(TEST_BINDIR)/$@

.PHONY: $(TEST_TOOLS)

clean: stdclean

TEST_LIB = \
	-sktst \
	-skapp \
    -sncbi-vdb

#-------------------------------------------------------------------------------
# test-jwt-string

JWTSTRING_SRC = \
	test-jwt-string

JWTSTRING_OBJ = \
	$(addsuffix .$(OBJX),$(JWTSTRING_SRC))

$(TEST_BINDIR)/test-jwt-string: $(JWTSTRING_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_LIB)

str: test-jwt-string
	$(TEST_BINDIR)/$^

#-------------------------------------------------------------------------------
# test-json

JSON_SRC = \
	test-jwt-json

JSON_OBJ = \
	$(addsuffix .$(OBJX),$(JSON_SRC))

$(TEST_BINDIR)/test-jwt-json: $(JSON_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_LIB)

json: test-jwt-json
	$(TEST_BINDIR)/$^

#-------------------------------------------------------------------------------
# test-jwt

JWT_SRC = \
	test-jwt

JWT_OBJ = \
	$(addsuffix .$(OBJX),$(JWT_SRC))

$(TEST_BINDIR)/test-jwt: $(JWT_OBJ)
	$(LP) --exe -o $@ $^ $(TEST_LIB)

#-------------------------------------------------------------------------------
# test-jwt-c
# make sure we can use Jwt and link without stdc++

JWTC_SRC = \
	test-jwt-c

JWTC_OBJ = \
	$(addsuffix .$(OBJX),$(JWTC_SRC))

$(TEST_BINDIR)/test-jwt-c: $(JWTC_OBJ)
	#$(LD) --exe -o $@ $^ $(TEST_LIB)
	# this is temporary, until all libc++ references have been removed:
	$(LP) --exe -o $@ $^ $(TEST_LIB)

c: test-jwt-c
	$(TEST_BINDIR)/$^

