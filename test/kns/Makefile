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

runtests: run-tests run-test-proxy-with-scheme

TOP ?= $(abspath ../..)
MODULE = test/kns

TEST_TOOLS = \
	test_allow_all_certificates \
	test_dis_allow_all_certificates \
	test-google-proxy \
	test200for-whole-file \
	test-kns \
	test-http \
	test-http-dropconn \
	test-http-request \
	KNSManagerSingletonTest \
	test-connect \
	test-refresh-expired

include $(TOP)/build/Makefile.env

EXT_TOOLS = \
	test-proxy \
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
# all
#
$(TARGDIR)/all: \
	$(addprefix $(BINDIR)/,$(ALL_TOOLS))

.PHONY: $(TARGDIR)/all

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
# test_allow_all_certificates
#
AAA_SRC = \
	test_allow_all_certificates

AAA_OBJ = \
	$(addsuffix .$(OBJX),$(AAA_SRC))

$(TEST_BINDIR)/test_allow_all_certificates: $(AAA_OBJ)
	$(LP) --exe -o $@ $^ $(KNSTEST_LIB)

#----------------------------------------------------------------
# test_dis_allow_all_certificates
#
DAA_SRC = \
	test_dis_allow_all_certificates

DAA_OBJ = \
	$(addsuffix .$(OBJX),$(DAA_SRC))

$(TEST_BINDIR)/test_dis_allow_all_certificates: $(DAA_OBJ)
	$(LP) --exe -o $@ $^ $(KNSTEST_LIB)

#----------------------------------------------------------------
# test-google-proxy
#
P_SRC = \
	test-google-proxy

P_OBJ = \
	$(addsuffix .$(OBJX),$(P_SRC))

$(TEST_BINDIR)/test-google-proxy: $(P_OBJ)
	$(LP) --exe -o $@ $^ $(KNSTEST_LIB)

#----------------------------------------------------------------
# test200for-whole-file
#
K2_SRC = \
	test200for-whole-file \

K2_OBJ = \
	$(addsuffix .$(OBJX),$(K2_SRC))

$(TEST_BINDIR)/test200for-whole-file: $(K2_OBJ)
	$(LP) --exe -o $@ $^ $(KNSTEST_LIB)

#----------------------------------------------------------------
# test-proxy
#
PROXYTEST_SRC = \
	http-client \
	test-proxy \

PROXYTEST_OBJ = \
	$(addsuffix .$(OBJX),$(PROXYTEST_SRC))

$(BINDIR)/test-proxy: $(PROXYTEST_OBJ)
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
	HttpFixture \
	httptest

HTTPTEST_OBJ = \
	$(addsuffix .$(OBJX),$(HTTPTEST_SRC))

$(TEST_BINDIR)/test-http: $(HTTPTEST_OBJ)
	$(LP) --exe -o $@ $^ $(KNSTEST_LIB)

http: test-http
	unset http_proxy ; $(TEST_BINDIR)/test-http  # -l=all

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
# test-http-request
#
HTTP_REQUEST_TEST_SRC = \
	HttpFixture \
	test-http-request

HTTP_REQUEST_TEST_OBJ = \
	$(addsuffix .$(OBJX),$(HTTP_REQUEST_TEST_SRC))

$(TEST_BINDIR)/test-http-request: $(HTTP_REQUEST_TEST_OBJ)
	$(LP) --exe -o $@ $^ $(KNSTEST_LIB)

req: test-http-request
	$(TEST_BINDIR)/test-http-request

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
# test-connect
#
CONNECT_SRC = \
	test-connect \

CONNECT_OBJ = \
	$(addsuffix .$(OBJX),$(CONNECT_SRC))

$(TEST_BINDIR)/test-connect: $(CONNECT_OBJ)
	$(LP) --exe -o $@ $^ $(KNSTEST_LIB)

connect: test-connect
	$(TEST_BINDIR)/test-connect

#----------------------------------------------------------------

#----------------------------------------------------------------
# test-refresh-expired
#
REFRESH_SRC = \
	HttpFixture \
	test-refresh-expired-url \

REFRESH_OBJ = \
	$(addsuffix .$(OBJX),$(REFRESH_SRC))

$(TEST_BINDIR)/test-refresh-expired: $(REFRESH_OBJ)
	$(LP) --exe -o $@ $^ $(KNSTEST_LIB)

refresh: test-refresh-expired
	$(TEST_BINDIR)/test-refresh-expired

#----------------------------------------------------------------

run-tests:
	unset http_proxy ; $(TEST_BINDIR)/test-http
	unset http_proxy ; $(BINDIR)/test-proxy

# test-proxy-with-scheme never does direct internet connection
# ( /http/proxy/only = "true" )
# It can access internet
# just when there is a good proxy specified via configuration or environment

run-test-proxy-with-scheme:
	@ echo test-proxy-with-scheme: BEGIN ...

	# no proxy specified: fail
	@ unset http_proxy ; $(BINDIR)/test-proxy-with-scheme

	# bad proxy in configuration: fail
	@ unset http_proxy ; \
                $(BINDIR)/test-proxy-with-scheme =BAD.proxy.ncbi.nlm.nih.gov

	# good proxy in configuration: success
	@ ping -c1 webproxy > /dev/null \
        && $(BINDIR)/test-proxy-with-scheme =webproxy.ncbi.nlm.nih.gov  SUCCESS \
        || echo skipped

	# bad proxy in environment: fail
	@ http_proxy=BAD.proxy.ncbi.nlm.nih.gov $(BINDIR)/test-proxy-with-scheme

	# good proxy in environment: success
	@ ping -c1 webproxy > /dev/null && all_proxy=webproxy.ncbi.nlm.nih.gov \
		$(BINDIR)/test-proxy-with-scheme                       SUCCESS || echo skipped

	# good proxy with schema in environment: success
	@ ping -c1 webproxy > /dev/null && ALL_PROXY=http://webproxy.ncbi.nlm.nih.gov \
		$(BINDIR)/test-proxy-with-scheme                       SUCCESS || echo skipped

	# good proxy with schema and port in environment: success
	@ ping -c1 webproxy > /dev/null && \
        http_proxy=http://webproxy.ncbi.nlm.nih.gov:3128 \
		$(BINDIR)/test-proxy-with-scheme                       SUCCESS || echo skipped

	# good proxy with any schema in environment: success
	@ ping -c1 webproxy > /dev/null && \
        HTTP_PROXY=anySchemaWillBeIgnored://webproxy.ncbi.nlm.nih.gov \
		$(BINDIR)/test-proxy-with-scheme                       SUCCESS || echo skipped

	# bad proxy port in environment: fail
	@ http_proxy=http://webproxy.ncbi.nlm.nih.gov:3 \
		$(BINDIR)/test-proxy-with-scheme

	# bad proxy specifications in environment: fail
	@ http_proxy=h            $(BINDIR)/test-proxy-with-scheme
	@ http_proxy=:            $(BINDIR)/test-proxy-with-scheme
	@ http_proxy=:a           $(BINDIR)/test-proxy-with-scheme
	@ http_proxy=:2           $(BINDIR)/test-proxy-with-scheme
	@ http_proxy=h:/w.g:3128  $(BINDIR)/test-proxy-with-scheme
	@ http_proxy=http://      $(BINDIR)/test-proxy-with-scheme
	@ http_proxy=http://:3128 $(BINDIR)/test-proxy-with-scheme

	# VDB-3329: http_proxy specification with trailing slash
	@ ping -c1 webproxy > /dev/null && \
	http_proxy=http://webproxy.ncbi.nlm.nih.gov:3128/     \
		$(BINDIR)/test-proxy-with-scheme                       SUCCESS || echo skipped
	@ ping -c1 webproxy > /dev/null && \
	http_proxy=http://webproxy.ncbi.nlm.nih.gov:3128///   \
		$(BINDIR)/test-proxy-with-scheme                       SUCCESS || echo skipped
	@ ping -c1 webproxy > /dev/null && \
	http_proxy=http://webproxy.ncbi.nlm.nih.gov:3128/:    \
		$(BINDIR)/test-proxy-with-scheme                       SUCCESS || echo skipped
	@ ping -c1 webproxy > /dev/null && \
	http_proxy=http://webproxy.ncbi.nlm.nih.gov:3128???XY \
		$(BINDIR)/test-proxy-with-scheme                       SUCCESS || echo skipped
	@ http_proxy=http://webproxy.ncbi.nlm.nih.gov/X         \
		$(BINDIR)/test-proxy-with-scheme
	@ http_proxy=http://webproxy.ncbi.nlm.nih.gov?X         \
		$(BINDIR)/test-proxy-with-scheme

	@ echo ... test-proxy-with-scheme: END
