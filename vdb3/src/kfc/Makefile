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


default: std

TOP ?= $(abspath ../../..)
MODULE = vdb3/src/kfc


INT_LIBS = libvdb3

ALL_LIBS = $(INT_LIBS)

OBJFILES = \
	$(VDB3_OBJ)

include $(TOP)/build/Makefile.env
#EXTLIBS =
INCDIRS += -I. -I$(TOP)/vdb3/itf

all std: makedirs compile
#	@ $(MAKE_CMD) $(TARGDIR)/std

#$(INT_LIBS): makedirs
#	@ $(MAKE_CMD) $(ILIBDIR)/$@

.PHONY: default all std $(ALL_LIBS) $(TARGDIR)/all $(TARGDIR)/std

$(TARGDIR)/std: $(addprefix $(ILIBDIR)/,$(INT_LIBS))

$(ILIBDIR)/libvdb3: $(addprefix $(ILIBDIR)/livdb3.,$(LIBEXT))

#TARGETS = \
#	$(INTLIBS) \
#	$(EXTLIBS)

# kernel-like facilities
VDB3_SRC =  \
	rsrc     \
	pfdmgr   \
	fdmgr    \
	fd       \
	stream   \
	plogger  \
	log      \
	ptimemgr \
	pmemmgr  \
	syserr   \
	string   \
	timemgr  \
	time     \
	ptr      \
	array    \
	memmgr   \
	memory   \
	ref      \
	refcount \
	integer  \
	except   \
	callstk

# object files
VDB3_OBJ = $(addprefix $(OBJDIR)/,$(addsuffix .$(LOBX),$(VDB3_SRC)))

#$(INT_LIBS): makedirs
#	@ $(MAKE_CMD) $(ILIBDIR)/$@

#$(TARGDIR)/std:
#	$(addprefix $(ILIBDIR)/,$(INT_LIBS))
#	echo "MV: std"
#	echo "MV: $@"

#$(ILIBDIR)/libvdb3: $(addprefix $(ILIBDIR)/libvdb3.,$(ILIBEXT))

$(ILIBDIR)/libvdb3.$(LIBX): $(VDB3_OBJ)
	$(LD) --slib -o $@ $^ $(VDB3_LIB)

clean:
	rm -rf $(ILIBDIR)/$(LPFX)ngs-disp* $(OBJDIR)


libvdb3: $(ILIBDIR) $(OBJDIR) $(ILIBDIR)/$(LPFX)libvdb3.$(LIBX)

# rule to produce the static library
$(ILIBDIR)/$(LPFX)libvdb3.$(LIBX): $(VDB3_OBJ)
	$(AR) $@ $^

compile: stdcompile

$(TARGDIR)/compile: $(OBJFILES)

