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

TOP ?= $(abspath ../../..)
MODULE = libs/klib/judy

INT_LIBS = \
	libjudy

ALL_LIBS = \
	$(INT_LIBS)

OBJFILES = \
	$(JUDY_OBJ)

include $(TOP)/build/Makefile.env


#-------------------------------------------------------------------------------
# special rules
#  for some reason these guys use the same source and load it up with
#  ifdefs, so we have general rules to produce two sets of object files
#  from a single set of sources
#
%.n.32.$(OBJX): %.c
	$(CC) -o $@ $(OPT) $(CFLAGS) $<
%.n.64.$(OBJX): %.c
	$(CC) -o $@ $(OPT) $(CFLAGS) $<
%.n.32_64.$(OBJX): %.c
	$(CC) -o $@ $(OPT) $(CFLAGS) $<
%.n.32.$(LOBX): %.c
	$(CC) -o $@ -fPIC $(OPT) $(CFLAGS) $<
%.n.64.$(LOBX): %.c
	$(CC) -o $@ -fPIC $(OPT) $(CFLAGS) $<
%.n.32_64.$(LOBX): %.c
	$(CC) -o $@ $(OPT) $(CFLAGS) $<

%.b.32.$(OBJX): %.c
	$(CC) -o $@ $(OPT) $(CFLAGS) -DJUDY1 $<
%.b.64.$(OBJX): %.c
	$(CC) -o $@ $(OPT) $(CFLAGS) -DJUDY1 $<
%.b.32_64.$(OBJX): %.c
	$(CC) -o $@ $(OPT) $(CFLAGS) -DJUDY1 $<
%.b.32.$(LOBX): %.c
	$(CC) -o $@ -fPIC $(OPT) $(CFLAGS) -DJUDY1 $<
%.b.64.$(LOBX): %.c
	$(CC) -o $@ -fPIC $(OPT) $(CFLAGS) -DJUDY1 $<
%.b.32_64.$(LOBX): %.c
	$(CC) -o $@ -fPIC $(OPT) $(CFLAGS) -DJUDY1 $<

%.w.32.$(OBJX): %.c
	$(CC) -o $@ $(OPT) $(CFLAGS) -DJUDYL $<
%.w.64.$(OBJX): %.c
	$(CC) -o $@ $(OPT) $(CFLAGS) -DJUDYL $<
%.w.32_64.$(OBJX): %.c
	$(CC) -o $@ $(OPT) $(CFLAGS) -DJUDYL $<
%.w.32.$(LOBX): %.c
	$(CC) -o $@ -fPIC $(OPT) $(CFLAGS) -DJUDYL $<
%.w.64.$(LOBX): %.c
	$(CC) -o $@ -fPIC $(OPT) $(CFLAGS) -DJUDYL $<
%.w.32_64.$(LOBX): %.c
	$(CC) -o $@ -fPIC $(OPT) $(CFLAGS) -DJUDYL $<

ifeq (win,$(OS))
#JUDY_WIN = -DJU_WIN
JUDY_WIN = -DJU_WMAIN -I$(TOP)/interfaces/cc/vc++
endif

#ifeq (dbg, $(BUILD))
#DEBUG := -DDEBUG
#endif

CFLAGS = $(DEBUG)               \
	$(NO_ARRAY_BOUNDS_WARNING)  \
	$(DBG) $(CARCH) $(PROF)     \
	$(JUDY_WIN) $(PED)          \
	$(DEFINES) $(ARCHDEFS)      \
	$(MIN_DEPLOY_OS_OPT)        \
	-I$(SRCDIR) -I$(SRCDIR)/..

VPATH += $(OBJDIR)

#-------------------------------------------------------------------------------
# outer targets
#
all std: makedirs
	@ $(MAKE_CMD) $(TARGDIR)/std

$(INT_LIBS): makedirs
	@ $(MAKE_CMD) $(ILIBDIR)/$@

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
# judy tables - whee!
#
JUDY_TABLES_SRC = \
	JudyTablesGen

JUDY_TABLES_BOBJ = \
	$(addsuffix .b.$(BITS).$(OBJX),$(JUDY_TABLES_SRC))

JUDY_TABLES_WOBJ = \
	$(addsuffix .w.$(BITS).$(OBJX),$(JUDY_TABLES_SRC))

$(OBJDIR)/Judy1TablesGen: $(JUDY_TABLES_BOBJ)
	$(LD) --exe -o $@ $^

$(OBJDIR)/JudyLTablesGen: $(JUDY_TABLES_WOBJ)
	$(LD) --exe -o $@ $^

$(OBJDIR)/Judy1Tables.c: $(OBJDIR)/Judy1TablesGen
	$(RUN_REMOTELY) $^

$(OBJDIR)/JudyLTables.c: $(OBJDIR)/JudyLTablesGen
	$(RUN_REMOTELY) $^

$(ILIBDIR)/judy-tables: \
	$(OBJDIR)/JudyLTables.c \
	$(OBJDIR)/Judy1Tables.c

#    
# make judy-tables to manually regenerate Judy tables sources, if there is ever such a need.
#    
judy-tables:
	@ $(MAKE_CMD) $(ILIBDIR)/$@
	cp $(OBJDIR)/Judy1Tables.c $(SRCDIR)/Judy1Tables.$(BITS).c
	cp $(OBJDIR)/JudyLTables.c $(SRCDIR)/JudyLTables.$(BITS).c

.PHONY: judy-tables
    
#-------------------------------------------------------------------------------
# judy whee!
#

$(ILIBDIR)/libjudy: $(addprefix $(ILIBDIR)/libjudy.,$(LIBX))

JUDY_NEUTRAL = \
	JudyMalloc \
	JudySL \
	JudyHS

JUDY_CMN = \
	JudyGet \
	JudyIns \
	JudyInsArray \
	JudyDel \
	JudyCascade \
	JudyCount \
	JudyCreateBranch \
	JudyDecascade \
	JudyFirst \
	JudyFreeArray \
	JudyInsertBranch \
	JudyMallocIF \
	JudyMemActive \
	JudyMemUsed \
	JudyNext \
	JudyNextEmpty \
	JudyPrev \
	JudyPrevEmpty \
	JudyByCountWithDefs \
	j__udyGet

JUDY_WORD = \
	_JudyLTables

JUDY_BIT = \
	_Judy1Tables

JUDY_SRC = \
	$(addsuffix .n,$(JUDY_NEUTRAL)) \
	$(addsuffix .w,$(JUDY_CMN) $(JUDY_WORD)) \
	$(addsuffix .b,$(JUDY_CMN) $(JUDY_BIT))

JUDY_OBJ = \
	$(addsuffix .$(BITS).$(LOBX),$(JUDY_SRC))

$(ILIBDIR)/libjudy.$(LIBX): $(JUDY_OBJ)
	$(LD) --slib -o $@ $^

compile: stdcompile

$(TARGDIR)/compile: $(OBJFILES)
