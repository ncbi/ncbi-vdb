# vdb3.kfc.Makefile

default: tests debug release

compile: compile-debug compile-release

MODULE := kfc

TOP ?= $(firstword $(subst /$(MODULE), ,$(CURDIR)))
include $(TOP)/kfc/Makefile.env

# rules for generating strerror_r.o in C
# something is messed up in C++ for this function
$(OBJDIR)/%.dbg.$(OBJX): %.c $(TOP)/kfc/Makefile.env
	gcc -g -c $< -o $@ -DDEBUG $(CFLAGS) $(INCDIR) -MD
$(OBJDIR)/%.rel.$(OBJX): %.c $(TOP)/kfc/Makefile.env
	gcc -O3 -c $< -o $@ -DNDEBUG $(CFLAGS) $(INCDIR) -MD

# TESTS
TESTDBGLIBS =                  \
	$(OBJDIR)/strerror_r.dbg.o \
	-L$(LIBDIR)                \
	-lgtest                    \
	$(LQUADMATH)               \
	-lpthread                  \
	-lm                        \
	$(LMCHECK)

TESTDBGLIBDEPS =               \
	Makefile                   \
	$(OBJDIR)/strerror_r.dbg.o \

TESTRELLIBS =                  \
	$(OBJDIR)/strerror_r.rel.o \
	-L$(LIBDIR)                \
	-lgtest                    \
	$(LQUADMATH)               \
	-lpthread                  \
	-lm

TESTRELLIBDEPS =               \
	Makefile                   \
	$(OBJDIR)/strerror_r.rel.o \

# ktrace
$(TSTDIR)/ktrace-tst-dbg: $(OBJDIR)/ktrace-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/ktrace-tst-dbg.passed: $(TSTDIR)/ktrace-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/ktrace-tst-rel: $(OBJDIR)/ktrace-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/ktrace-tst-rel.passed: $(TSTDIR)/ktrace-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# defs
$(TSTDIR)/defs-tst-dbg: $(OBJDIR)/defs-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/defs-tst-dbg.passed: $(TSTDIR)/defs-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/defs-tst-rel: $(OBJDIR)/defs-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/defs-tst-rel.passed: $(TSTDIR)/defs-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# memset_s
$(TSTDIR)/memset_s-tst-dbg: $(OBJDIR)/memset_s-tst.nopt.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/memset_s-tst-dbg.passed: $(TSTDIR)/memset_s-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/memset_s-tst-rel: $(OBJDIR)/memset_s-tst.nopt.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/memset_s-tst-rel.passed: $(TSTDIR)/memset_s-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# Z128, N128
$(TSTDIR)/z128-tst-dbg: $(OBJDIR)/z128-tst.nopt.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/z128-tst-dbg.passed: $(TSTDIR)/z128-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/z128-tst-rel: $(OBJDIR)/z128-tst.nopt.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/z128-tst-rel.passed: $(TSTDIR)/z128-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# ascii
$(TSTDIR)/ascii-tst-dbg: $(OBJDIR)/ascii-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/ascii-tst-dbg.passed: $(TSTDIR)/ascii-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/ascii-tst-rel: $(OBJDIR)/ascii-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/ascii-tst-rel.passed: $(TSTDIR)/ascii-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# fmt-base
$(TSTDIR)/fmt-base-tst-dbg: $(OBJDIR)/fmt-base-tst.nopt.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/fmt-base-tst-dbg.passed: $(TSTDIR)/fmt-base-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/fmt-base-tst-rel: $(OBJDIR)/fmt-base-tst.nopt.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/fmt-base-tst-rel.passed: $(TSTDIR)/fmt-base-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# xp
$(TSTDIR)/xp-tst-dbg: $(OBJDIR)/xp-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/xp-tst-dbg.passed: $(TSTDIR)/xp-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/xp-tst-rel: $(OBJDIR)/xp-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/xp-tst-rel.passed: $(TSTDIR)/xp-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# except
$(TSTDIR)/except-tst-dbg: $(OBJDIR)/except-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/except-tst-dbg.passed: $(TSTDIR)/except-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/except-tst-rel: $(OBJDIR)/except-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/except-tst-rel.passed: $(TSTDIR)/except-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# xbt
$(TSTDIR)/xbt-tst-dbg: $(OBJDIR)/xbt-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/xbt-tst-dbg.passed: $(TSTDIR)/xbt-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/xbt-tst-rel: $(OBJDIR)/xbt-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/xbt-tst-rel.passed: $(TSTDIR)/xbt-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# time
$(TSTDIR)/time-tst-dbg: $(OBJDIR)/time-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/time-tst-dbg.passed: $(TSTDIR)/time-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/time-tst-rel: $(OBJDIR)/time-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/time-tst-rel.passed: $(TSTDIR)/time-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# decflt
$(TSTDIR)/decflt-tst-dbg: $(OBJDIR)/decflt-tst.nopt.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/decflt-tst-dbg.passed: $(TSTDIR)/decflt-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/decflt-tst-rel: $(OBJDIR)/decflt-tst.nopt.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/decflt-tst-rel.passed: $(TSTDIR)/decflt-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# zigzag
$(TSTDIR)/zigzag-tst-dbg: $(OBJDIR)/zigzag-tst.nopt.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/zigzag-tst-dbg.passed: $(TSTDIR)/zigzag-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/zigzag-tst-rel: $(OBJDIR)/zigzag-tst.nopt.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/zigzag-tst-rel.passed: $(TSTDIR)/zigzag-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# ref
$(TSTDIR)/ref-tst-dbg: $(OBJDIR)/ref-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/ref-tst-dbg.passed: $(TSTDIR)/ref-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/ref-tst-rel: $(OBJDIR)/ref-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/ref-tst-rel.passed: $(TSTDIR)/ref-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# ref-na
$(TSTDIR)/ref-na-tst-dbg: $(OBJDIR)/ref-na-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/ref-na-tst-dbg.passed: $(TSTDIR)/ref-na-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/ref-na-tst-rel: $(OBJDIR)/ref-na-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/ref-na-tst-rel.passed: $(TSTDIR)/ref-na-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# tmmgr
$(TSTDIR)/tmmgr-tst-dbg: $(OBJDIR)/tmmgr-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/tmmgr-tst-dbg.passed: $(TSTDIR)/tmmgr-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/tmmgr-tst-rel: $(OBJDIR)/tmmgr-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/tmmgr-tst-rel.passed: $(TSTDIR)/tmmgr-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# rsrc-tm
$(TSTDIR)/rsrc-tm-tst-dbg: $(OBJDIR)/rsrc-tm-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/rsrc-tm-tst-dbg.passed: $(TSTDIR)/rsrc-tm-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/rsrc-tm-tst-rel: $(OBJDIR)/rsrc-tm-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/rsrc-tm-tst-rel.passed: $(TSTDIR)/rsrc-tm-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# rgn
$(TSTDIR)/rgn-tst-dbg: $(OBJDIR)/rgn-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/rgn-tst-dbg.passed: $(TSTDIR)/rgn-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/rgn-tst-rel: $(OBJDIR)/rgn-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/rgn-tst-rel.passed: $(TSTDIR)/rgn-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# rgn-lock
$(TSTDIR)/rgn-lock-tst-dbg: $(OBJDIR)/rgn-lock-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/rgn-lock-tst-dbg.passed: $(TSTDIR)/rgn-lock-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/rgn-lock-tst-rel: $(OBJDIR)/rgn-lock-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/rgn-lock-tst-rel.passed: $(TSTDIR)/rgn-lock-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# mem
$(TSTDIR)/mem-tst-dbg: $(OBJDIR)/mem-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/mem-tst-dbg.passed: $(TSTDIR)/mem-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/mem-tst-rel: $(OBJDIR)/mem-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/mem-tst-rel.passed: $(TSTDIR)/mem-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# mmgr
$(TSTDIR)/mmgr-tst-dbg: $(OBJDIR)/mmgr-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/mmgr-tst-dbg.passed: $(TSTDIR)/mmgr-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/mmgr-tst-rel: $(OBJDIR)/mmgr-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/mmgr-tst-rel.passed: $(TSTDIR)/mmgr-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# busy
$(TSTDIR)/busy-tst-dbg: $(OBJDIR)/busy-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/busy-tst-dbg.passed: $(TSTDIR)/busy-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/busy-tst-rel: $(OBJDIR)/busy-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/busy-tst-rel.passed: $(TSTDIR)/busy-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# pmmgr
$(TSTDIR)/pmmgr-tst-dbg: $(OBJDIR)/pmmgr-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/pmmgr-tst-dbg.passed: $(TSTDIR)/pmmgr-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/pmmgr-tst-rel: $(OBJDIR)/pmmgr-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/pmmgr-tst-rel.passed: $(TSTDIR)/pmmgr-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# rsrc-mem
$(TSTDIR)/rsrc-mem-tst-dbg: $(OBJDIR)/rsrc-mem-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/rsrc-mem-tst-dbg.passed: $(TSTDIR)/rsrc-mem-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/rsrc-mem-tst-rel: $(OBJDIR)/rsrc-mem-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/rsrc-mem-tst-rel.passed: $(TSTDIR)/rsrc-mem-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )


KFCTESTS =         \
	ktrace         \
	defs           \
	memset_s       \
	z128           \
	ascii          \
	fmt-base       \
	xp             \
	except         \
	xbt            \
	time           \
	decflt         \
	zigzag         \
	ref            \
	ref-na         \
	tmmgr          \
	rsrc-tm        \
	rgn            \
	rgn-lock       \
	mem            \
	mmgr           \
	busy           \
	pmmgr          \
	rsrc-mem       \

KFCDBGTESTS = \
	$(addprefix $(TSTDIR)/,$(addsuffix -tst-dbg,$(KFCTESTS)))

KFCDBGTESTSPASS = \
	$(addprefix $(TSTDIR)/,$(addsuffix -tst-dbg.passed,$(KFCTESTS)))

KFCRELTESTS = \
	$(addprefix $(TSTDIR)/,$(addsuffix -tst-rel,$(KFCTESTS)))

KFCRELTESTSPASS = \
	$(addprefix $(TSTDIR)/,$(addsuffix -tst-rel.passed,$(KFCTESTS)))

TESTTARGS =        \
	$(KFCDBGTESTS) \
	$(KFCRELTESTS)

debug-tests: $(OBJDIR) $(TSTDIR) $(KFCDBGTESTS) $(KFCDBGTESTSPASS)

release-tests: $(OBJDIR) $(TSTDIR) $(KFCRELTESTS) $(KFCRELTESTSPASS)

tests: $(OBJDIR) $(TSTDIR) $(KFCDBGTESTS) $(KFCRELTESTS) $(KFCDBGTESTSPASS) $(KFCRELTESTSPASS)


# library
LIBTARGS =                        \
	$(LIBDIR)/libkfc-dbg.$(LIBX)  \
	$(LIBDIR)/libkfc-rel.$(LIBX)

KFCSRC =           \
	rsrc-mem       \
	pmmgr          \
	pmem           \
	pmem-rgn-lock  \
	busy           \
	mmgr           \
	mem            \
	rgn-lock       \
	rgn            \
	rsrc-tm        \
	$(OS)-tmmgr    \
	tmmgr          \
	ref            \
	zigzag         \
	decflt         \
	time           \
	xbt            \
	except         \
	xp             \
	fmt-base       \
	ascii          \
	$(Z128LSRC)    \
	memset_s.nopt  \
	ktrace         \
	strerror_r

debug: debug-tests $(OBJDIR) $(LIBDIR) $(LIBDIR)/libkfc-dbg.$(LIBX)

KFCDBGOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .dbg.$(OBJX),$(KFCSRC)))

compile-debug: $(OBJDIR) $(TSTDIR) $(KFCDBGTESTS) $(KFCDBGOBJ)

$(LIBDIR)/libkfc-dbg.$(LIBX): $(KFCDBGOBJ)
	ar -rc $@ $(KFCDBGOBJ)

release: release-tests $(OBJDIR) $(LIBDIR) $(LIBDIR)/libkfc-rel.$(LIBX)

KFCRELOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .rel.$(OBJX),$(KFCSRC)))

compile-release: $(OBJDIR) $(TSTDIR) $(KFCRELTESTS) $(KFCRELOBJ)

$(LIBDIR)/libkfc-rel.$(LIBX): $(KFCRELOBJ)
	ar -rc $@ $(KFCRELOBJ)

$(TESTTARGS) $(LIBTARGS): Makefile

clean:
	rm -rf $(OBJDIR) $(TSTDIR)
	rm -f $(LIBTARGS)
