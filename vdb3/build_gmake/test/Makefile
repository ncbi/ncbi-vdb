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
#	Tests for the VDB3 build system
#

#-------------------------------------------------------------------------------
# default
#
default: test

TOP = $(abspath ../..)

MAKE = make --no-print-directory
WORKDIR = .
ACTUAL = $(WORKDIR)/actual

include $(TOP)/build/Makefile.vers
empty :=
space := $(empty) $(empty)
MAJMIN  = $(subst $(space),.,$(wordlist 1,2,$(subst .,$(space),$(VERSION))))
MAJVERS = $(firstword $(subst .,$(space),$(VERSION)))

config:

build:

clean:
	rm -rf $(ACTUAL)/*

define Setup
    cp Makefile.config.test-default Makefile.config && \
    cp $(TOP)/docs/doxygen.config ./docs/ && \
	rm -rf $(ACTUAL)/outdir docs/html
endef

#
# ExpectPass/ExpectFail
# call like this:
#    $(call ExpectPass, <testId>, ParametersToMakeCommandUnderTest, optionalCommandToVerifyResult)
# set TESTCASE fromcommand line to only run 1 test case, e.g.
#    make TESTCASE=1.1 test
#
RunToPass = ( $(MAKE) $(2) 1>$(ACTUAL)/$(1).stdout 2>$(ACTUAL)/$(1).stderr || \
           ( echo "$(1) failed with $$? ($(MAKE) $(2))" && false ) ) && \
        ( ( eval "$(3)" && echo "$(1) passed" ) || \
           echo "$(1) ($(MAKE) $(2)) failed: $(3) " )
RunToFail = ! $(MAKE) $(2) 1>$(ACTUAL)/$(1).stdout 2>$(ACTUAL)/$(1).stderr && echo "$(1) passed"
ifneq (,$(TESTCASE))
    define ExpectPass
    	if [ "$(TESTCASE)" = "$(1)" ]; then echo "Running $(1)"; $(RunToPass); fi
    endef
    define ExpectFail
    	if [ "$(TESTCASE)" = "$(1)" ]; then echo "Running $(1)"; $(RunToFail); fi
    endef
else
    define ExpectPass
        $(RunToPass)
    endef
    define ExpectFail
        $(RunToFail)
    endef
endif

LIBPLATFORM = libncbi-vdb3-platform.a
LIBDB 		= libncbi-vdb3-db.a

#TODO: make sure tests use real Makefiles (from $(TOP)/, not $(TOP)/build/test/), wherever possible
TEST_PLATFORM 	= -C platform
TEST_DB 		= -C db

ifeq (1,$(BUILD_TESTING))
# prevent recursive execution
test:
else
test:
	@ ln -f -s $(abspath ..) # in order to use the real files from $(TOP)/build
	@ mkdir -p $(ACTUAL)
	@ cp Makefile.config.test-default Makefile.config
	@ #
	@ echo "Testing build system"
	@ #
	@ # 1: platform/sub-project
	@ #
	@ # help
	@ $(call ExpectPass,1.1,-C platform/hello help)
	@ #
	@ # config
	@ $(call ExpectPass,1.2,-C platform/hello config)
	@ #
	@ # build
	@ $(call Setup); \
	  $(call ExpectPass,1.3,-C platform/hello build, \
	  						test -f $(ACTUAL)/outdir/dbg/obj/platform/hello/hello.pic.o)
	@ #
	@ # clean
	@ $(call Setup); \
	  $(call ExpectPass,1.4,-C platform/hello build clean, \
	  						! test -f $(ACTUAL)/outdir/dbg/obj/platform/hello/hello.pic.o)
	@ #
	@ # test (run all unit tests)
	@ $(call Setup); \
	  $(call ExpectPass,1.5,-C platform/hello test, \
	  						[ 2 -eq $$(grep --count Passed $(ACTUAL)/1.5.stdout) ] )
	@ #
	@ # check (same as test)
	@ $(call Setup); \
	  $(call ExpectPass,1.6,-C platform/hello check, \
	  						[ 2 -eq $$(grep --count Passed $(ACTUAL)/1.6.stdout) ] )
	@ #
	@ # 2: platform/sub-project/test
	@ #
	@ # help
	@ $(call ExpectPass,2.1,-C platform/hello/test help)
	@ #
	@ # config
	@ $(call ExpectPass,2.2,-C platform/hello/test config)
	@ #
	@ # build
	@ $(call Setup); \
	  $(call ExpectPass,2.3,-C platform/hello/test build, \
	  						test -f $(ACTUAL)/outdir/dbg/test-bin/test-hello1 )
	@ #
	@ # clean
	@ $(call Setup); \
	  $(call ExpectPass,2.4,-C platform/hello/test build clean, \
	  						! test -f $(ACTUAL)/outdir/dbg/test-bin/test-hello1 )
	@ #
	@ # run a single unit test
	@ $(call Setup); \
	  $(call ExpectPass,2.5,-C platform/hello/test hello1, \
	  						[ 1 -eq $$(grep -i --count Passed $(ACTUAL)/2.5.stdout) ] )
	@ #
	@ #
	@ # 3. the platform library
	@ #
	@ # help/
	@ $(call ExpectPass,3.1, -C platform help)
	@ #
	@ # config
	@ $(call ExpectPass,3.2, -C platform config)
	@ #
	@ # build; $(LIBPLATFORM) created
	@ $(call Setup);\
	  $(call ExpectPass,3.3, $(TEST_PLATFORM) build,\
	  						test -f $(ACTUAL)/outdir/dbg/lib/$(LIBPLATFORM))
	@ #
	@ # clean
	@ $(call Setup); \
	  $(call ExpectPass,3.4, -C platform build clean,\
							! test -f $(ACTUAL)/outdir/dbg/lib/$(LIBPLATFORM))
	@ #
	@ # test; expect 3 unit tests to be run
	@ $(call Setup); \
	  $(call ExpectPass,3.5, -C platform test, \
	  						[ 3 -eq $$(grep -i --count Passed $(ACTUAL)/3.5.stdout) ] )
	@ #
	@ #
	@ # 4. the db library
	@ #
	@ # help
	@ $(call ExpectPass,4.1, -C db help)
	@ #
	@ # config
	@ $(call ExpectPass,4.2, -C db config)
	@ #
	@ # build; $(LIBDB) created
	@ $(call Setup);\
	  $(call ExpectPass,4.3, -C db build,\
	  						test -f $(ACTUAL)/outdir/dbg/lib/$(LIBDB))
	@ #
	@ # clean
	@ $(call Setup); \
	  $(call ExpectPass,4.4, -C db build clean,\
							! test -f $(ACTUAL)/outdir/dbg/lib/$(LIBDB))
	@ #
	@ # test; expect 2 unit tests to be run (build the platform first)
	@
	@ $(call Setup); \
	  $(MAKE) --quiet -C platform; \
	  $(call ExpectPass,4.5, -C db test, \
	  						[ 2 -eq $$(grep --count Passed $(ACTUAL)/4.5.stdout) ] )
	@ #
	@ #
	@ #TODO: 5 services/
	@ #
	@ # help
	@ $(call ExpectPass,5.1, -C services help)
	@ #
	@ # config
	@ $(call ExpectPass,5.2, -C services config)
	@ #
	@ # build; bin/servica and bin/serviceb created (build platform and db first)
	@ $(call Setup); $(MAKE) --quiet -C platform; $(MAKE) --quiet -C db; \
	  $(call ExpectPass,5.3, -C services build,\
	  						test -f $(ACTUAL)/outdir/dbg/bin/servicea && \
	  						test -f $(ACTUAL)/outdir/dbg/bin/serviceb \
							)
	@ #
	@ # clean  (build platform and db first)
	@ $(call Setup); $(MAKE) --quiet -C platform; $(MAKE) --quiet -C db; \
	  $(call ExpectPass,5.4, -C services build clean,\
	  						! test -f $(ACTUAL)/outdir/dbg/bin/servicea && \
							! test -f $(ACTUAL)/outdir/dbg/bin/serviceb)
	@ #
	@ # test; expect 2 unit tests to be run (build the platform first)
	@
	@ $(call Setup); $(MAKE) --quiet -C platform; $(MAKE) --quiet -C db; \
	  $(call ExpectPass,5.5, -C services test, \
	  						[ 2 -eq $$(grep --count Passed $(ACTUAL)/5.5.stdout) ] )
	@ #
	@ #
	@ #TODO: 6 tools/
	@ #
	@ # help
	@ $(call ExpectPass,6.1, -C tools help)
	@ #
	@ # config
	@ $(call ExpectPass,6.2, -C tools config)
	@ #
	@ # build; bin/toola and bin/toolb created (build platform and db first)
	@ $(call Setup); $(MAKE) --quiet -C platform; $(MAKE) --quiet -C db; \
	  $(call ExpectPass,6.3, -C tools build,\
	  						test -f $(ACTUAL)/outdir/dbg/bin/toola && \
	  						test -f $(ACTUAL)/outdir/dbg/bin/toolb \
							)
	@ #
	@ # clean  (build platform and db first)
	@ $(call Setup); $(MAKE) --quiet -C platform; $(MAKE) --quiet -C db; \
	  $(call ExpectPass,6.4, -C tools build clean,\
	  						! test -f $(ACTUAL)/outdir/dbg/bin/toola && \
							! test -f $(ACTUAL)/outdir/dbg/bin/toolb)
	@ #
	@ # test; expect 2 unit tests to be run (build the platform first)
	@
	@ $(call Setup); $(MAKE) --quiet -C platform; $(MAKE) --quiet -C db; \
	  $(call ExpectPass,6.5, -C tools test, \
	  						[ 2 -eq $$(grep --count Passed $(ACTUAL)/6.5.stdout) ] )
	@ #
	@ #
	@ #TODO: 7 tests/
	@ #
	@ # help
	@ $(call ExpectPass,7.1, -C tests help)
	@ #
	@ # config
	@ $(call ExpectPass,7.2, -C tests config)
	@ #
	@ # build; do not expect any binaries here
	@ $(call Setup); \
	  $(call ExpectPass,7.3, -C tests build)
	@ #
	@ # clean
	@ $(call Setup); \
	  $(call ExpectPass,7.4, -C tests clean)
	@ #
	@ # test; expect 2 tests to be run
	@
	@ $(call Setup); \
	  $(call ExpectPass,7.5, -C tests test, \
	  						grep --quiet Suite1 $(ACTUAL)/7.5.stdout && \
							grep --quiet Suite2 $(ACTUAL)/7.5.stdout)
	@ #
	@ #
	@ # 8. Top directory
	@ #
	@ # bad target
	@ $(call ExpectFail,8.1,-f $(TOP)/Makefile h)
	@ #
	@ # help
	@ $(call ExpectPass,8.2,-f $(TOP)/Makefile help)
	@ #
	@ # config
	@ $(call ExpectPass,8.3, -C tests config)
	@ #
	@ # switch from debug to release
	@ $(call Setup); \
	  $(call ExpectPass,8.4,-f $(TOP)/Makefile release,\
							grep BUILD Makefile.config | grep --quiet rel)
	@ #
	@ # switch from release to debug
	@ $(call Setup); \
	  $(call ExpectPass,8.5,-f $(TOP)/Makefile release debug,\
							grep BUILD Makefile.config | grep --quiet dbg)
	@ #
	@ # switch between OUTDIRs
	@ $(call Setup); \
	  $(call ExpectPass,8.6,-f $(TOP)/Makefile out OUTDIR=$(ACTUAL)/other_out,\
							grep OUTDIR Makefile.config | grep --quiet other_out)
	@ #
	@ # build
	@ $(call Setup); \
	  $(call ExpectPass,8.7,-f $(TOP)/Makefile build, \
	  						test -f $(ACTUAL)/outdir/dbg/bin/toola && \
							test -f $(ACTUAL)/outdir/dbg/bin/servicea)
	@ #
	@ # clean
	@ $(call Setup); \
	  $(call ExpectPass,8.8,-f $(TOP)/Makefile clean,\
	  						! test -f $(ACTUAL)/outdir/dbg)
	@ #
	@ # test
	@ $(call Setup); \
	  $(call ExpectPass,8.9,-f $(TOP)/Makefile BUILD_TESTING=1 test, \
	  						grep --quiet Hello $(ACTUAL)/8.9.stdout && \
							grep --quiet BDB $(ACTUAL)/8.9.stdout && \
							grep --quiet ServiceA $(ACTUAL)/8.9.stdout && \
							grep --quiet ToolB $(ACTUAL)/8.9.stdout && \
							grep --quiet Suite1 $(ACTUAL)/8.9.stdout \
		)
	@ #
	@ # missing Makefile.config
	@ $(call Setup); rm Makefile.config; \
	  $(call ExpectPass,8.10,-f $(TOP)/Makefile config)
	@ #
	@ # docs (Doxygen)
	@ $(call Setup); \
	  $(call ExpectPass,8.11,-f $(TOP)/Makefile docs, test -d $(WORKDIR)/docs/html)
	@ #
	@ # install
	@ $(call Setup); \
	  $(call ExpectPass,9.1,-f $(TOP)/Makefile install INSTDIR=$(abspath $(ACTUAL)/install), \
			test -f $(ACTUAL)/install/toola.$(VERSION) && \
			test -L $(ACTUAL)/install/toola.$(MAJMIN) && \
			test -L $(ACTUAL)/install/toola.$(MAJVERS) && \
			test -L $(ACTUAL)/install/toola && \
			test -L $(ACTUAL)/install/toolb && \
			test -L $(ACTUAL)/install/servicea && \
			test -L $(ACTUAL)/install/serviceb \
		)
	@ #
	@ # uninstall
	@ $(call Setup); \
	  $(MAKE) -f $(TOP)/Makefile install INSTDIR=$(abspath $(ACTUAL)/install) >/dev/null; \
	  $(call ExpectPass,10.1,-f $(TOP)/Makefile uninstall, \
			! test -e $(ACTUAL)/install/toola.$(VERSION) && \
			! test -e $(ACTUAL)/install/toola.$(MAJMIN) && \
			! test -e $(ACTUAL)/install/toola.$(MAJVERS) && \
			! test -e $(ACTUAL)/install/toola && \
			! test -e $(ACTUAL)/install/toolb && \
			! test -e $(ACTUAL)/install/servicea && \
			! test -e $(ACTUAL)/install/serviceb \
	  )
	@ #
	@ # package (Docker)
ifeq (0,$(shell id -u))
	@ $(call Setup); \
	$(call ExpectPass,11.1,-f $(TOP)/Makefile PACKAGE=vdb3-test package, docker image inspect vdb3-test >/dev/null)
else
	@ echo "You need to be root to run 'make package'. Test 11.1 is skipped"
endif
	@ #
	@ #TODO: test on iebdev (when googletest is installed)
	@ #TODO: add "this is a test/template" comments throughout the test makefiles
	@ #
	@ rm -rf Makefile.config actual
endif

.phony: config build clean test

