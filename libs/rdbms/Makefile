# ==============================================================================
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
MODULE = libs/rdbms

include $(TOP)/build/Makefile.env

INT_LIBS =

EXT_LIBS = \
	librdbms

ALL_LIBS = \
	$(INT_LIBS) \
	$(EXT_LIBS)

MSSQL_INCLUDES ?= /export/home/sybase/clients-mssql/current/include

INCDIRS += $(addprefix -I,$(MSSQL_INCLUDES))

#-------------------------------------------------------------------------------
# outer targets
#
all std: makedirs
	@ $(MAKE_CMD) $(TARGDIR)/std

$(INT_LIBS): makedirs
	@ $(MAKE_CMD) $(ILIBDIR)/$@

$(EXT_LIBS): makedirs
	@ $(MAKE_CMD) $(LIBDIR)/$@

.PHONY: all std $(ALL_LIBS)

#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: \
	$(addprefix $(ILIBDIR)/,$(INT_LIBS)) \
	$(addprefix $(LIBDIR)/,$(EXT_LIBS))

.PHONY: $(TARGDIR)/std

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean

.PHONY: clean

#-------------------------------------------------------------------------------
# OS specific application support
#
$(LIBDIR)/librdbms: $(addprefix $(LIBDIR)/librdbms.,$(LIBEXT))

RDBMS_SRC = \
	rdbms \
	sybase

RDBMS_OBJ = \
	$(addsuffix .$(LOBX),$(RDBMS_SRC))

RDBMS_LIB = \

$(LIBDIR)/librdbms.$(SHLX): $(RDBMS_OBJ)
	$(LD) --dlib --vers $(SRCDIR) -o $@ $^ $(RDBMS_LIB)

$(LIBDIR)/librdbms.$(LIBX): $(RDBMS_OBJ)
	$(LD) --slib --vers $(SRCDIR) -o $@ $^ $(RDBMS_LIB)

