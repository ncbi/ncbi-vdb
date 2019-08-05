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
MODULE = test/cloud

TEST_TOOLS = \
	test-aws \
	test-gcp \
	test-cloud \

include $(TOP)/build/Makefile.env

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

#----------------------------------------------------------------
# test-cloud
#
CLOUD_SRC = \
	test-cloud

CLOUD_OBJ = \
	$(addsuffix .$(OBJX),$(CLOUD_SRC))

CLOUD_LIB = \
	-skapp \
    -sncbi-vdb \
    -sktst \

$(TEST_BINDIR)/test-cloud: $(CLOUD_OBJ)
	$(LP) --exe -o $@ $^ $(CLOUD_LIB)

cloud: test-cloud
	$(TEST_BINDIR)/test-cloud  #-l=all

#----------------------------------------------------------------
# test-aws
#
A_SRC = \
	test-aws

A_OBJ = \
	$(addsuffix .$(OBJX),$(A_SRC))

$(TEST_BINDIR)/test-aws: $(A_OBJ)
	$(LP) --exe -o $@ $^ $(CLOUD_LIB)

#----------------------------------------------------------------
# test-gcp
#
G_SRC = \
	test-gcp TestStream

G_OBJ = \
	$(addsuffix .$(OBJX),$(G_SRC))

$(TEST_BINDIR)/test-gcp: $(G_OBJ)
	$(LP) --exe -o $@ $^ $(CLOUD_LIB)

gcp: test-gcp
	$(TEST_BINDIR)/test-gcp  #-l=all