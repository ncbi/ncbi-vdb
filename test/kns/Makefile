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
MODULE = test/kns

TEST_TOOLS = \
	test-kns \
	test-http \
	test-http-dropconn \

include $(TOP)/build/Makefile.env

$(TEST_TOOLS): makedirs
	@ $(MAKE_CMD) $(TEST_BINDIR)/$@

clean: stdclean

INCDIRS += -I$(TOP)/libs/kns

#-------------------------------------------------------------------------------
# white-box test. TODO: combine with the old test/kns/http-test.c, roll into the C++ test-http
#
#
#TEST_SRC = \
#	wb-http-test
#
#TEST_OBJ = \
#	$(addsuffix .$(OBJX),$(TEST_SRC))
#
#TEST_LIB = \
#	-skapp-norsrc \
#	-sncbi-vdb \
#
#$(TEST_BINDIR)/test-http: $(TEST_OBJ)
#	$(LD) --exe -o $@ $^ $(TEST_LIB)

KNSTEST_LIB = \
	-skapp \
    -sncbi-vdb \
    -sktst \

#----------------------------------------------------------------
# test-kns
#
KNSTEST_SRC = \
	knstest

KNSTEST_OBJ = \
	$(addsuffix .$(OBJX),$(KNSTEST_SRC))

$(TEST_BINDIR)/test-kns: $(KNSTEST_OBJ)
	$(LP) --exe -o $@ $^ $(KNSTEST_LIB)

kns: test-kns 
	$(TEST_BINDIR)/test-kns  #-l=all
    
#----------------------------------------------------------------
# test-http
#
HTTPTEST_SRC = \
	httptest

HTTPTEST_OBJ = \
	$(addsuffix .$(OBJX),$(HTTPTEST_SRC))

$(TEST_BINDIR)/test-http: $(HTTPTEST_OBJ)
	$(LP) --exe -o $@ $^ $(KNSTEST_LIB)

http: test-http  
	$(TEST_BINDIR)/test-http  # -l=all
    
#----------------------------------------------------------------
# test-http-dropconn
#
HTTP_DROPCONN_TEST_SRC = \
	http_dropconnection_test

HTTP_DROPCONN_TEST_OBJ = \
	$(addsuffix .$(OBJX),$(HTTP_DROPCONN_TEST_SRC))

$(TEST_BINDIR)/test-http-dropconn: $(HTTP_DROPCONN_TEST_OBJ)
	$(LP) --exe -o $@ $^ $(KNSTEST_LIB)

dropconn: test-http-dropconn  
	$(TEST_BINDIR)/test-http-dropconn  # -l=all
    
