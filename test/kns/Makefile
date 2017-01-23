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

runtests: run-test-proxy-with-scheme

TOP ?= $(abspath ../..)
MODULE = test/kns

TEST_TOOLS = \
	test-kns \
	test-proxy \
	test-http \
	test-http-dropconn \
	KNSManagerSingletonTest \

include $(TOP)/build/Makefile.env

EXT_TOOLS = \
	test-proxy-with-env \
	test-proxy-with-scheme \

ALL_TOOLS = \
	$(INT_TOOLS) \
	$(EXT_TOOLS)

#-------------------------------------------------------------------------------
# outer targets
#
all std: makedirs
	@ $(MAKE_CMD) $(TARGDIR)/$@

$(ALL_TOOLS): makedirs
	@ $(MAKE_CMD) $(BINDIR)/$@

.PHONY: all std $(ALL_TOOLS)

#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: \
	$(addprefix $(BINDIR)/,$(EXT_TOOLS))

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
# KNSManagerSingletonTest
#
SINGLETON_SRC = \
	KNSManagerSingletonTest
SINGLETON_OBJ = \
	$(addsuffix .$(OBJX),$(SINGLETON_SRC))
$(TEST_BINDIR)/KNSManagerSingletonTest: $(SINGLETON_OBJ)
	$(LP) --exe -o $@ $^ $(KNSTEST_LIB)

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
# test-proxy
#
PROXYTEST_SRC = \
	http-client \
	test-proxy \

PROXYTEST_OBJ = \
	$(addsuffix .$(OBJX),$(PROXYTEST_SRC))

$(TEST_BINDIR)/test-proxy: $(PROXYTEST_OBJ)
	$(LP) --exe -o $@ $^ $(KNSTEST_LIB)
    

#----------------------------------------------------------------
# test-proxy-with-scheme
#
PROXYSCH_SRC = \
	test-proxy-with-scheme \

PROXYSCH_OBJ = \
	$(addsuffix .$(OBJX),$(PROXYSCH_SRC))

$(BINDIR)/test-proxy-with-scheme: $(PROXYSCH_OBJ)
	$(LP) --exe -o $@ $^ $(KNSTEST_LIB)


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
    
#----------------------------------------------------------------
# test-proxy-with-env
# 
TOOL_SRC = \
	http-client \
	test-proxy-with-env \

TOOL_OBJ = \
	$(addsuffix .$(OBJX),$(TOOL_SRC))

$(BINDIR)/test-proxy-with-env: $(TOOL_OBJ)
	$(LP) --exe -o $@ $^ -L$(VDB_LIBDIR) -L$(VDB_LIBDIR)/../ilib $(KNSTEST_LIB)


#----------------------------------------------------------------


# test-proxy-with-scheme never does direct intermet connection
# ( /http/proxy/only = "true" )
# It can access internet
# just when there is a good proxy specified via configuration or environment

run-test-proxy-with-scheme:
	# no proxy specified: fail
	$(BINDIR)/test-proxy-with-scheme

	# bad proxy in configuration: fail
	$(BINDIR)/test-proxy-with-scheme =BAD.proxy.ncbi.nlm.nih.gov

	# good proxy in configuration: success
	$(BINDIR)/test-proxy-with-scheme =webproxy.ncbi.nlm.nih.gov  SUCCESS

	# bad proxy in environment: fail
	all_proxy=BAD.proxy.ncbi.nlm.nih.gov $(BINDIR)/test-proxy-with-scheme

	# good proxy in environment: success
	all_proxy=webproxy.ncbi.nlm.nih.gov \
		$(BINDIR)/test-proxy-with-scheme                         SUCCESS

	# good proxy with schema in environment: success
	ALL_PROXY=http://webproxy.ncbi.nlm.nih.gov \
		$(BINDIR)/test-proxy-with-scheme                         SUCCESS

	# good proxy with schema and port in environment: success
	http_proxy=http://webproxy.ncbi.nlm.nih.gov:3128 \
		$(BINDIR)/test-proxy-with-scheme                         SUCCESS

	# good proxy with any schema in environment: success
	HTTP_PROXY=anySchemaWillBeIgnored://webproxy.ncbi.nlm.nih.gov \
		$(BINDIR)/test-proxy-with-scheme                         SUCCESS

	# bad proxy port in environment: fail
	all_proxy=http://webproxy.ncbi.nlm.nih.gov:3 \
		$(BINDIR)/test-proxy-with-scheme

	# bad proxy specifications in environment: fail
	all_proxy=h            $(BINDIR)/test-proxy-with-scheme
	all_proxy=:            $(BINDIR)/test-proxy-with-scheme
	all_proxy=:a           $(BINDIR)/test-proxy-with-scheme
	all_proxy=:2           $(BINDIR)/test-proxy-with-scheme
	all_proxy=h:/w.g:3128  $(BINDIR)/test-proxy-with-scheme
	all_proxy=http://      $(BINDIR)/test-proxy-with-scheme
	all_proxy=http://:3128 $(BINDIR)/test-proxy-with-scheme
