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
MODULE = libs/ext/libmagic

INT_LIBS = \
	libmagic

ALL_LIBS = \
	$(INT_LIBS) \

include $(TOP)/build/Makefile.env

#-------------------------------------------------------------------------------
# outer targets
#
std all: $(INT_LIBS) 

$(INT_LIBS): makedirs 
	@ $(MAKE_CMD) $(ILIBDIR)/$@

.PHONY: all std $(ALL_LIBS)

#-------------------------------------------------------------------------------
# clean
#
clean: stdclean

.PHONY: clean

#-------------------------------------------------------------------------------
# libmagic
#
$(ILIBDIR)/libmagic: $(addprefix $(ILIBDIR)/libmagic.,$(LIBX))

#CFLAGS += -DHAVE_SYS_PARAM 
CFLAGS += -DHAVE_CONFIG_H -DHAVE_STDDEF_H

# 5.04
#LIBMAGIC_SRC = \
#	magic \
#	apprentice \
#	softmagic \
#	ascmagic \
#	encoding \
#	compress \
#	is_tar \
#	readelf \
#	print \
#	fsmagic \
#	funcs \
#	apptype \
#	cdf \
#	cdf_time \
#	readcdf

# 5.20
LIBMAGIC_SRC =  \
    magic       \
    apprentice  \
    softmagic   \
    ascmagic    \
	encoding    \
    compress    \
    is_tar      \
    readelf     \
    print       \
    fsmagic     \
	funcs       \
    apptype     \
	cdf         \
    cdf_time    \
    readcdf     \
    strlcpy     \
    strlcat     \
    fmtcheck    \
    
LIBMAGIC_OBJ = \
	$(addsuffix .$(LOBX),$(LIBMAGIC_SRC))

$(ILIBDIR)/libmagic.$(LIBX): $(LIBMAGIC_OBJ)
	$(LD) --slib --vers $(SRCDIR) -o $@ $^
