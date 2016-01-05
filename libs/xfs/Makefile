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

default: all

TOP ?= $(abspath ../..)

MODULE = libs/xfs

include $(TOP)/build/Makefile.env
RWORKDIR=$(TOP)/$(MODULE)

ifeq (mac,$(OS))
TOP = $(shell cd ../.. && pwd )
endif


INT_LIBS = \
	libxfs

ALL_LIBS = \
	$(INT_LIBS)


#-------------------------------------------------------------------------------
# outer targets
#
all std: vers-includes
	@ $(MAKE_CMD) $(TARGDIR)/std

$(INT_LIBS): vers-includes
	@ $(MAKE_CMD) $(ILIBDIR)/$@

.PHONY: all std $(ALL_LIBS)

#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: \
	$(addprefix $(ILIBDIR)/,$(INT_LIBS))

.PHONY: $(TARGDIR)/std

#-------------------------------------------------------------------------------
# vers-includes
#
$(TARGDIR)/vers-includes: $(addsuffix .vers.h,$(EXT_LIBS))

.PHONY: $(TARGDIR)/vers-includes

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean

.PHONY: clean

ifeq (mac,$(OS))
INCDIRS += -I/usr/local/include/osxfuse/fuse
endif

#-------------------------------------------------------------------------------
# xfs
#
$(ILIBDIR)/libxfs: $(addprefix $(ILIBDIR)/libxfs.,$(ILIBEXT))

XFS_SRC = \
    xfs \
    xlog    \
    platform \
    operations  \
    security    \
    model    \
    tree    \
    node    \
    path    \
    perm    \
    editors \
    handle  \
    teleport    \
    foo \
    ncon  \
    contnode    \
    docnode \
    simcon  \
    kfs   \
    readme  \
    bogus     \
    owp \
    doc \
    xhttp    \
    http    \
    xtar     \
    tar     \
    xencro     \
    encro     \
    xencws     \
    encws     \
    spen    \
    xgap    \
    xgapf    \
    xgapk   \
    gapk    \
    gapks   \
    gapka   \
    gapc   \
    gap     \
    gapf    \
    lockdpt \
    lreader \
    schwarzschraube \
    zehr    \
    mehr

XFS_OBJ = \
	$(addsuffix .$(LOBX),$(XFS_SRC))

$(ILIBDIR)/libxfs.$(LIBX): $(XFS_OBJ)
	$(LD) --slib -o $@ $^ $(XFS_LIB)
