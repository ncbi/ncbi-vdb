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
MODULE = libs/hdf5

INT_LIBS = \

EXT_LIBS = \
	libkdf5

ALL_LIBS = \
	$(INT_LIBS) \
	$(EXT_LIBS)

OBJFILES = \
	$(KDF5_OBJ)

include $(TOP)/build/Makefile.env

ifdef HDF5_INCDIR
	INCDIRS += $(addprefix -I,$(HDF5_INCDIR))
endif

#-------------------------------------------------------------------------------
# outer targets
#
ifeq (1,$(HAVE_HDF5))

all std: makedirs compile
	@ $(MAKE_CMD) $(TARGDIR)/std

$(INT_LIBS): makedirs
	@ $(MAKE_CMD) $(ILIBDIR)/$@

$(EXT_LIBS): makedirs
	@ $(MAKE_CMD) $(LIBDIR)/$@

else

all std: $(ALL_LIBS)

$(INT_LIBS):
	@ echo "NOTE - internal library $(@F) cannot be built:"         \
	       "It requires 'libhdf5' and its development headers."

$(EXT_LIBS):
	@ echo "NOTE - library $(@F) cannot be built:"                  \
	       "It requires 'libhdf5' and its development headers."
endif

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
# hdf5
#
$(LIBDIR)/libkdf5: $(addprefix $(LIBDIR)/libkdf5.,$(LIBEXT))

KDF5_SRC = \
	hdf5dir \
	hdf5file \
	hdf5arrayfile

KDF5_OBJ = \
	$(addsuffix .$(LOBX),$(KDF5_SRC))

KDF5_LIB = \
	-lhdf5 \
	-lz \

ifdef HDF5_LIBDIR
	KDF5_LIB += $(addprefix -L,$(HDF5_LIBDIR))
endif

$(LIBDIR)/libkdf5.$(SHLX): $(KDF5_OBJ)
	$(LD) --dlib --vers $(SRCDIR)/../ncbi-vdb/libncbi-vdb.vers -o $@ $^ $(KDF5_LIB)

$(LIBDIR)/libkdf5.$(LIBX): $(KDF5_OBJ)
	$(LD) --slib --vers $(SRCDIR)/../ncbi-vdb/libncbi-vdb.vers -o $@ $^ $(KDF5_LIB)

compile: stdcompile

$(TARGDIR)/compile: $(OBJFILES)
