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

# repairs to Makefile.config
INCDIRS = -I. -I$(TOP)/vdb3/itf

OBJFILES = \
	$(KFC_OBJ)

INTLIBS = \
	vdb3-kfc

EXTLIBS =   \

TARGETS =      \
	$(INTLIBS) \
	$(EXTLIBS)

# kernel-like facilities
KFC_SRC =    \
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
KFC_OBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .$(LOBX),$(KFC_SRC)))

all std: $(TARGETS)

compile: $(OBJDIR) $(OBJFILES)

clean:
	rm -rf $(ILIBDIR)/$(LPFX)vdb3* $(OBJDIR)

.PHONY: default all std $(TARGETS)

# rule to produce the static library
vdb3-kfc: $(ILIBDIR) $(OBJDIR) $(ILIBDIR)/$(LPFX)vdb3-kfc.$(LIBX)
$(ILIBDIR)/$(LPFX)vdb3-kfc.$(LIBX): $(KFC_OBJ)
	$(AR) $@ $^

libvdb3: $(ILIBDIR) $(OBJDIR) $(ILIBDIR)/$(LPFX)vdb3.$(LIBX)
$(ILIBDIR)/$(LPFX)vdb3.$(LIBX): $(VDB3_OBJ)
	$(AR) $@ $^

compile: stdcompile

$(TARGDIR)/compile: $(OBJFILES) libvdb3

