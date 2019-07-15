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
MODULE = libs/kfs

INT_LIBS = \
	libkfs \
	libkfs-nommap \
	libkff \

ALL_LIBS = \
	$(INT_LIBS)

OBJFILES =           \
	$(KFS_OBJ)       \
	$(KFS_NOMMAP_OBJ)

include $(TOP)/build/Makefile.env

#-------------------------------------------------------------------------------
# outer targets
#
all std: makedirs compile
	@ $(MAKE_CMD) $(TARGDIR)/std

$(INT_LIBS): makedirs
	@ $(MAKE_CMD) $(ILIBDIR)/$@

.PHONY: all std $(ALL_LIBS)

#-------------------------------------------------------------------------------
# std
#
$(TARGDIR)/std: \
	$(addprefix $(ILIBDIR)/,$(INT_LIBS))

.PHONY: $(TARGDIR)/std

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean

.PHONY: clean

#-------------------------------------------------------------------------------
# mmap.c
#  special rule to generate a disabled mmap
#
%.nommap.$(LOBX): %.c
	$(CC) -o $@ -fPIC $(OPT) -DFORCE_MALLOC_MMAP -D_LIBRARY $<


#-------------------------------------------------------------------------------
# kfs
#
$(ILIBDIR)/libkfs: $(addprefix $(ILIBDIR)/libkfs.,$(ILIBEXT))

$(ILIBDIR)/libkfs-nommap: $(addprefix $(ILIBDIR)/libkfs-nommap.,$(LIBX))

KFS_CMN = \
	directory \
	arrayfile \
	file \
	chunk-reader \
	sysdir \
	sysfile \
	sysmmap \
	syslockfile \
	sysdll \
	gzip \
	bzip \
	md5 \
	crc32 \
	arc \
	toc \
	tocentry \
	tocdir \
	tocfile \
	sra \
	tar \
	teefile \
	buffile \
	buffile-read \
	buffile-write \
	subfile \
	nullfile \
	countfile \
	dir_test \
	pagefile \
	pmem \
	readheadfile \
	ramfile \
	extendable-ramfile \
	quickmount \
	report \
	lockfile \
	remove-file-task \
	syslockfile \
	cacheteefile \
	cacheteefile2 \
	cacheteefile3 \
	rrcachedfile \
	recorder \
	logfile \
	callback_file \
	poolpages \
	lru_cache \
	from_to_namelist \
	limitfile   \
	appendfile

KFS_SRC = \
	mmap \
	$(KFS_CMN)

ifeq (win,$(OS))
	KFS_SRC += directory-path
endif

KFS_NOMMAP_SRC = \
	mmap.nommap \
	$(KFS_CMN)

KFS_OBJ = \
	$(addsuffix .$(LOBX),$(KFS_SRC))

KFS_NOMMAP_OBJ = \
	$(addsuffix .$(LOBX),$(KFS_NOMMAP_SRC))

KFS_LIB = \
	-dkproc \
	-dklib \
	-ddl \
	-dz \
	-dbz2

#	-dsz \

$(ILIBDIR)/libkfs.$(LIBX): $(KFS_OBJ)
	$(LD) --slib -o $@ $^ $(KFS_LIB)

$(ILIBDIR)/libkfs-nommap.$(LIBX): $(KFS_NOMMAP_OBJ)
	$(LD) --slib -o $@ $^ $(KFS_LIB)



#-------------------------------------------------------------------------------
# kfs.2
#
$(ILIBDIR)/libkfs.2: $(addprefix $(ILIBDIR)/libkfs.2.,$(ILIBEXT))

KFS2_SRC = \
	$(KFS_SRC) \
	file-v2 \
	directory-v2 \
	sysfile-v2
#	sysdir-v2

KFS2_OBJ = \
	$(addsuffix .$(LOBX),$(KFS2_SRC))

KFS2_LIB = \
	$(KFS_LIB)

$(ILIBDIR)/libkfs.2.$(LIBX): $(KFS2_OBJ)
	$(LD) --slib -o $@ $^ $(KFS2_LIB)


#-------------------------------------------------------------------------------
# kff
#
ifeq (1,$(HAVE_MAGIC))

$(ILIBDIR)/libkff: $(addprefix $(ILIBDIR)/libkff.,$(ILIBEXT))

OBJFILES += $(KFF_OBJ)

KFF_SRC = \
	fileformat \
	ffext \
	ffkey \
	ffmagic

KFF_OBJ = \
	$(addsuffix .$(LOBX),$(KFF_SRC))

KFF_LIB = \
	-dkfg \
	-dklib

ifneq (win,$(OS))
	KFF_LIB += -lmagic
endif

$(ILIBDIR)/libkff.$(LIBX): $(KFF_OBJ)
	$(LD) --slib -o $@ $^ $(KFF_LIB)

else

$(ILIBDIR)/libkff:
	@ echo "NOTE - internal library $(@F) cannot be built:"         \
	       "It requires 'libmagic' and its development headers."

endif

compile: stdcompile

$(TARGDIR)/compile: $(OBJFILES)


#-------------------------------------------------------------------------------
# fuzz
#
ifneq (1,$(HAVE_MAGIC))
INT_LIBS = \
	libkfs \
	libkfs-nommap
endif

fuzz: std
	cp $(addsuffix .$(LIBX),$(addprefix $(ILIBDIR)/,$(INT_LIBS))) $(FLIBDIR)/
