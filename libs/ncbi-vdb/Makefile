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
MODULE = libs/ncbi-vdb

EXT_LIBS = \
	libncbi-vdb \
	libncbi-wvdb

ALL_LIBS = \
	$(EXT_LIBS)

include $(TOP)/build/Makefile.env

#-------------------------------------------------------------------------------
# outer targets
#
all std: makedirs
	@ $(MAKE_CMD) $(TARGDIR)/std

$(EXT_LIBS): makedirs
	@ $(MAKE_CMD) $(LIBDIR)/$@

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
clean: stdclean cleanfuzz

.PHONY: clean

#-------------------------------------------------------------------------------
# virtual db library
# made in both static and shared versions
$(LIBDIR)/libncbi-vdb: $(addprefix $(LIBDIR)/libncbi-vdb.,$(LIBX) $(SHLX))

ifeq (64,$(BITS))
  LIBVDB_BLAST = vdb-blast
else
  LIBVDB_BLAST =
endif

ifeq (win,$(OS))
LIBKQ =
else
LIBKQ = kq
endif

ifeq (1,$(HAVE_XML2))
LIBKXML = kxml kxfs
ifneq (,$(XML2_LIBDIR))
LDFLAGS += -L$(XML2_LIBDIR)
endif
endif

VDB_LIB_CMN =    \
	align-access \
	ncbi-bam     \
	vfs          \
	cloud                  \
	$(LIBKXML)   \
	mbedx509     \
	mbedtls      \
	mbedcrypto   \
	kns          \
	kfg          \
	krypto       \
	ksrch        \
	kfs          \
	klib         \
	kproc        \
	$(LIBKQ)     \
	kfc          \
	bz2          \
	z

VDB_LIB_RD =              \
	align-reader          \
	ascp                  \
	$(EXT_SCHEMA_MODULES) \
	srareader             \
	sradb                 \
	$(LIBVDB_BLAST)       \
	vdb                   \
	kdb                   \
	$(VDB_LIB_CMN)        \
	ngs-jni               \
	ngs-py                \
	ngs                   \

VDB_OBJ = \
	$(addprefix $(ILIBDIR)/lib,$(addsuffix .a,$(VDB_LIB_RD)))

VDB_LIB = \
	$(addprefix -s,$(VDB_LIB_RD))

$(LIBDIR)/libncbi-vdb.$(SHLX): $(VDB_OBJ)
	echo $(SRCDIR)
	$(LD) --dlib --vers $(SRCDIR)/libncbi-vdb.vers -o $@ $(VDB_LIB)

$(LIBDIR)/libncbi-vdb.$(LIBX): $(VDB_OBJ)
	$(LD) --slib --vers $(SRCDIR)/libncbi-vdb.vers -o $@ $(VDB_LIB)

#-------------------------------------------------------------------------------
# update version
#
$(LIBDIR)/libncbi-wvdb: $(addprefix $(LIBDIR)/libncbi-wvdb.,$(LIBX) $(SHLX))

VDB_LIB_RDWR =                           \
	align-writer                         \
	$(addprefix w,$(EXT_SCHEMA_MODULES)) \
	wsradb                               \
	wvdb                                 \
	wkdb                                 \
	$(VDB_LIB_CMN)

WVDB_OBJ = \
	$(addprefix $(ILIBDIR)/lib,$(addsuffix .a,$(VDB_LIB_RDWR)))

WVDB_LIB = \
	$(addprefix -s,$(VDB_LIB_RDWR))

$(LIBDIR)/libncbi-wvdb.$(SHLX): $(WVDB_OBJ)
	$(LD) --dlib --vers $(SRCDIR)/libncbi-vdb.vers -o $@ $(WVDB_LIB)

$(LIBDIR)/libncbi-wvdb.$(LIBX): $(WVDB_OBJ)
	$(LD) --slib --vers $(SRCDIR)/libncbi-vdb.vers -o $@ $(WVDB_LIB)

#-------------------------------------------------------------------------------
# fuzz
#
fuzz:
	$(TOP)/build/ld.sh linux x86_64 gcc --build dbg --ldflags "-g  -m64" --objx o --shlx so --libx a --srcdir $(SRCDIR) --bindir $(BINDIR) --slib -o $(FLIBDIR)/libncbi-vdb.$(LIBX) -L$(FLIBDIR) $(VDB_LIB)

cleanfuzz:
	rm -rf $(FLIBDIR)/*