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

TOP ?= $(abspath ../..)
MODULE = libs/vdb-sqlite

EXT_LIBS = \
	libvdb-sqlite

ALL_LIBS = \
	$(EXT_LIBS)

include $(TOP)/build/Makefile.env

#-------------------------------------------------------------------------------
# outer targets
#
ifeq (1,$(HAVE_XML2))

all std: makedirs
	@ $(MAKE_CMD) $(TARGDIR)/std

$(EXT_LIBS): makedirs
	@ $(MAKE_CMD) $(LIBDIR)/$@

else

all std: $(ALL_LIBS)

$(EXT_LIBS):
	@ echo "NOTE - library $(@F) cannot be built: It requires 'libxml2'."
endif

.PHONY: all std $(ALL_LIBS)

#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: \
	$(addprefix $(LIBDIR)/,$(EXT_LIBS))

.PHONY: $(TARGDIR)/all $(TARGDIR)/std

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean

.PHONY: clean

#-------------------------------------------------------------------------------
# vdb - sqlite3 - bridge - lib
# made in both static and shared versions
$(LIBDIR)/libvdb-sqlite: $(addprefix $(LIBDIR)/libvdb-sqlite.,$(LIBX) $(SHLX))

VDB_SQLITE_SRC = \
	sqlite3 \
	sqlite3vdb

VDB_SQLITE_OBJ = \
	$(addsuffix .$(LOBX),$(VDB_SQLITE_SRC))

VDB_SQLITE_LIB = \
	-sncbi-vdb

VDB_SQLITE_VERS = \
	$(SRCDIR)/../ncbi-vdb/libncbi-vdb.vers

$(LIBDIR)/libvdb-sqlite.$(SHLX): $(VDB_SQLITE_OBJ)
	$(LD) --dlib --vers $(VDB_SQLITE_VERS) -o $@ $(VDB_SQLITE_OBJ) $(VDB_SQLITE_LIB)

$(LIBDIR)/libvdb-sqlite.$(LIBX): $(VDB_SQLITE_OBJ)
	$(LD) --slib --vers $(VDB_SQLITE_VERS) -o $@ $(VDB_SQLITE_OBJ) $(VDB_SQLITE_LIB)
