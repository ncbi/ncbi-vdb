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
MODULE = libs/kapp

include $(TOP)/build/Makefile.env

INT_LIBS = \
	libkapp \
	libkapp-norsrc \
    libload

ALL_LIBS = \
	$(INT_LIBS)

OBJFILES = \
	$(KAPP_OBJ) \
	$(KAPPR_OBJ) \
	$(LOADLIB_OBJ)

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
# OS specific application support
#
$(ILIBDIR)/libkapp: $(addprefix $(ILIBDIR)/libkapp.,$(LIBX))

KAPP_CMN = \
	args \
	args-conv \
	args-conv-os \
	tokenizer \
	sysmain \
	ram \
	log-xml

KAPP_SRC = \
	main \
	$(KAPP_CMN)

KAPP_OBJ = \
	$(addsuffix .$(OBJX),$(KAPP_SRC))

KAPP_LIB = \
	-dvfs \
	-dklib

$(ILIBDIR)/libkapp.$(LIBX): $(KAPP_OBJ)
	$(LD) --slib -o $@ $^ $(KAPP_LIB)


#-------------------------------------------------------------------------------
# KMane without KRsrc support
#
$(ILIBDIR)/libkapp-norsrc: $(addprefix $(ILIBDIR)/libkapp-norsrc.,$(LIBX))

KAPPR_SRC = \
	main-norsrc \
	$(KAPP_CMN)

main-norsrc.$(OBJX): main.c
	$(CC) -o $@ $(OPT) -DNO_KRSRC=1 $< $(LOC_INFO) -D__file_ext__=c

KAPPR_OBJ = \
	$(addsuffix .$(OBJX),$(KAPPR_SRC))

KAPPR_LIB = \

$(ILIBDIR)/libkapp-norsrc.$(LIBX): $(KAPPR_OBJ)
	$(LD) --slib -o $@ $^ $(KAPPR_LIB)


#-------------------------------------------------------------------------------
# loaders common libarary
#
$(ILIBDIR)/libload: $(addprefix $(ILIBDIR)/libload.,$(LIBX))

LOADLIB_SRC = \
	progressbar \
	loader-file \
	loader-meta

ifneq (win,$(OS))
	LOADLIB_SRC += queue-file
endif

LOADLIB_OBJ = \
	$(addsuffix .$(LOBX),$(LOADLIB_SRC))

LOADLIB_LIB = \
	-dkfs \
	-dklib

ifneq (win,$(OS))
	LOADLIB_LIB += -dkq
endif

$(ILIBDIR)/libload.$(LIBX): $(LOADLIB_OBJ)
	$(LD) --slib -o $@ $^ $(LOADLIB_LIB)

compile: stdcompile

$(TARGDIR)/compile: $(OBJFILES)
