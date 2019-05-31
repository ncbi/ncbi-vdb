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
# run 'make schema' to regenerate the built-in schema
# run 'make schema std' to regenerate the built-in schema and rebuild the libraries
#

default: std

TOP ?= $(abspath ../..)
MODULE = libs/sra

INT_LIBS = \
	libsraschema \
	libwsraschema \
	libsradb \
	libwsradb \
	libsrareader

ALL_LIBS = \
	$(INT_LIBS)

OBJFILES =            \
	$(SRADB_OBJ)      \
	$(WSRADB_OBJ)     \
	$(SRAREADER_OBJ)  \
#	$(SRA_SCHEMA_OBJ) \
	$(WSRA_SCHEMA_OBJ)

include $(TOP)/build/Makefile.env

VPATH += $(OBJDIR)

#-------------------------------------------------------------------------------
# outer targets
#
all std: makedirs compile
	@ $(MAKE_CMD) $(TARGDIR)/std

$(INT_LIBS) schema: makedirs
	@ $(MAKE_CMD) $(ILIBDIR)/$@

.PHONY: all std $(ALL_LIBS) schema

#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: \
	$(addprefix $(ILIBDIR)/,$(INT_LIBS))

.PHONY: $(TARGDIR)/all $(TARGDIR)/std

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean
	@ rm -f $(ILIBDIR)/libsra-schema.* $(ILIBDIR)/libwsra-schema.* $(ILIBDIR)/sra-dflt-schema*
	@ rm -f $(BINDIR)/libsra-schema.* $(BINDIR)/libwsra-schema.*

.PHONY: clean


#-------------------------------------------------------------------------------
# sradb embedded schema
#
$(ILIBDIR)/libsraschema: $(ILIBDIR)/libsraschema.$(LIBX)

$(ILIBDIR)/libwsraschema: $(ILIBDIR)/libwsraschema.$(LIBX)

SRA_DFLT_SCHEMA_LIBS = \
	-skapp \
	-svdb \
	$(ALWAYS_STATIC_SCHEMA_LIBS) \
	-salign-reader \
	-skdb \
	-svfs \
	-skns \
	-sksrch \
	-skrypto \
	-skfg \
	-skfs \
	-sklib \
    -sjudy \
	-smbedx509 \
	-smbedtls \
	-smbedcrypto \
	-skq \
	-skproc \

SRA_DFLT_SCHEMA_OS_LIBS = \
	-ldl \
	-sz \
	-sbz2 \
	-lm

# set workdir for RUN_REMOTELY in rwin mode
RWORKDIR=$(OBJDIR)

$(ILIBDIR)/sra-dflt-schema: sra-dflt-schema.$(OBJX)
	$(LD) --exe -o $@ $^ $(SRA_DFLT_SCHEMA_LIBS) $(SRA_DFLT_SCHEMA_OS_LIBS)

$(ILIBDIR)/schema: $(ILIBDIR)/sra-dflt-schema $(SRCDIR)/built-in.vschema
	$(ILIBDIR)/sra-dflt-schema -o$(SRCDIR)/sra-schema.c -I$(TOP)/interfaces -T$(OBJDIR)/sra-schema.c.d $(SRCDIR)/built-in.vschema

SRA_SCHEMA_SRC = \
	sraschema \
	sra-schema

SRA_SCHEMA_OBJ = \
	$(addsuffix .$(LOBX),$(SRA_SCHEMA_SRC))

WSRA_SCHEMA_OBJ = \
	$(addsuffix .$(LOBX),$(SRA_SCHEMA_SRC))

SRA_SCHEMA_LIB = \
	-dvdb \
	-dklib

WSRA_SCHEMA_LIB = \
	-dwvdb \
	-dklib

SRASCHEMA_SRC = \
	sraschema-stub \

WSRASCHEMA_SRC = \
	wsraschema-stub

SRASCHEMA_SRC += $(SRA_SCHEMA_SRC)
WSRASCHEMA_SRC += $(SRA_SCHEMA_SRC)

SRASCHEMA_OBJ = \
	$(addsuffix .$(LOBX),$(SRASCHEMA_SRC))

WSRASCHEMA_OBJ = \
	$(addsuffix .$(LOBX),$(WSRASCHEMA_SRC))

$(ILIBDIR)/libsraschema.$(LIBX): $(SRASCHEMA_OBJ) $(SRASCHEMA_DEPS)
	$(LD) --slib -o $@ $(SRASCHEMA_OBJ) $(SRA_SCHEMA_LIB)

$(ILIBDIR)/libwsraschema.$(LIBX): $(WSRASCHEMA_OBJ) $(WSRASCHEMA_DEPS)
	$(LD) --slib -o $@ $(WSRASCHEMA_OBJ) $(WSRA_SCHEMA_LIB)


#-------------------------------------------------------------------------------
# sradb library
#
$(ILIBDIR)/libsradb: $(addprefix $(ILIBDIR)/libsradb.,$(ILIBEXT))

SRADBCMN_SRC = \
	sramgr-cmn \
	sratbl-cmn \
	sracol \
	types \
    sracache

SRADB_SRC = \
	$(SRADBCMN_SRC) \
	sramgr \
	sratbl

SRADB_OBJ = \
	$(addsuffix .$(LOBX),$(SRADB_SRC))

SRADB_LIB = \
	-ssraschema \
	-dvdb \
	-dkdb \
	-dvfs \
	-dkfs \
	-lkfg \
	-dkproc \
	-dklib

$(ILIBDIR)/libsradb.$(LIBX): $(SRADB_OBJ)
	$(LD) --slib -o $@ $^ $(SRADB_LIB)


#-------------------------------------------------------------------------------
# update version
#
$(ILIBDIR)/libwsradb: $(addprefix $(ILIBDIR)/libwsradb.,$(ILIBEXT))

WSRADB_SRC = \
	$(SRADBCMN_SRC) \
	wsramgr \
	wsratbl

WSRADB_OBJ = \
	$(addsuffix .$(LOBX),$(WSRADB_SRC))

WSRADB_LIB = \
	-swsraschema \
	-dwvdb \
	-dwkdb \
	-dvfs \
	-dkfs \
	-lkfg \
	-dkproc \
	-dklib

$(ILIBDIR)/libwsradb.$(LIBX): $(WSRADB_OBJ)
	$(LD) --slib -o $@ $^ $(WSRADB_LIB)


#-------------------------------------------------------------------------------
# libsrareader
#
$(ILIBDIR)/libsrareader: $(addprefix $(ILIBDIR)/libsrareader.,$(ILIBEXT))

SRAREADER_SRC = \
	reader-cmn \
	reader-fastq \
	reader-sff \
	reader-illumina \
	reader-abi

SRAREADER_OBJ = \
	$(addsuffix .$(LOBX),$(SRAREADER_SRC))

SRAREADER_LIB = \
	-dsradb \
	-dkdb \
	-dklib

$(ILIBDIR)/libsrareader.$(LIBX): $(SRAREADER_OBJ)
	$(LD) --slib -o $@ $^ $(SRAREADER_LIB)

compile: stdcompile

$(TARGDIR)/compile: $(OBJFILES)

#-------------------------------------------------------------------------------
# fuzz
#
fuzz: std
	cp $(addsuffix .$(LIBX),$(addprefix $(ILIBDIR)/,$(INT_LIBS))) $(FLIBDIR)/
