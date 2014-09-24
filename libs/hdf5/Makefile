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

include $(TOP)/build/Makefile.env

#-------------------------------------------------------------------------------
# where to find include/libs
#
ifdef NCBI
ifeq (dbg,$(BUILD))
	DFLT_LIBHDF5_INCLUDES = $(NCBI)/hdf5/Debug/include
	LIBHDF5_LPATH = $(NCBI)/hdf5/Debug/lib
else
	DFLT_LIBHDF5_INCLUDES = $(NCBI)/hdf5/Release/include
	LIBHDF5_LPATH = $(NCBI)/hdf5/Release/lib
endif
endif

DFLT_LIBHDF5_INCLUDES ?= /usr/include/hdf5
LIBHDF5_INCLUDES ?= $(DFLT_LIBHDF5_INCLUDES)
INCDIRS += $(addprefix -I,$(LIBHDF5_INCLUDES))

#-------------------------------------------------------------------------------
# outer targets
#
all std: makedirs
	@ $(MAKE_CMD) LIBHDF5_INCLUDES=$(LIBHDF5_INCLUDES) $(TARGDIR)/std

$(INT_LIBS): makedirs
	@ $(MAKE_CMD) LIBHDF5_INCLUDES=$(LIBHDF5_INCLUDES) $(ILIBDIR)/$@

$(EXT_LIBS): makedirs
	@ $(MAKE_CMD) LIBHDF5_INCLUDES=$(LIBHDF5_INCLUDES) $(LIBDIR)/$@

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

ifdef LIBHDF5_LPATH
KDF5_LIB = \
	-X$(LIBHDF5_LPATH) \
	-shdf5
else
KDF5_LIB = \
	-lhdf5 \
	-lsz
endif

KDF5_LIB += -lz

$(LIBDIR)/libkdf5.$(SHLX): $(KDF5_OBJ)
	$(LD) --dlib --vers $(SRCDIR) -o $@ $^ $(KDF5_LIB)

$(LIBDIR)/libkdf5.$(LIBX): $(KDF5_OBJ)
	$(LD) --slib --vers $(SRCDIR) -o $@ $^ $(KDF5_LIB)
