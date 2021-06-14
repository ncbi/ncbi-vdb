# vdb3.cmn.Makefile

default: tests debug release

compile: compile-debug compile-release

MODULE := cmn

TOP ?= $(firstword $(subst /$(MODULE), ,$(CURDIR)))
include $(TOP)/kfc/Makefile.env

# a once-generated data file
$(BINDIR)/gen-base64-tbl: gen-base64-tbl.c
	gcc -o $@ gen-base64-tbl.c

base64-tables.hpp: $(BINDIR)/gen-base64-tbl
	$< > $@

# TESTS
TESTDBGLIBS =      \
	-L$(LIBDIR)    \
	-lkfc-dbg      \
	-lutf8proc-dbg \
	-lgtest-dbg    \
	$(LQUADMATH)   \
	-lpthread      \
	$(LMCHECK)

TESTDBGLIBDEPS =           \
	Makefile               \
	$(LIBDIR)/libkfc-dbg.a \

TESTRELLIBS =      \
	-L$(LIBDIR)    \
	-lkfc-rel      \
	-lutf8proc-rel \
	-lgtest-rel    \
	$(LQUADMATH)   \
	-lpthread

TESTRELLIBDEPS =           \
	Makefile               \
	$(LIBDIR)/libkfc-rel.a \

# buffer
$(TSTDIR)/buffer-tst-dbg: $(OBJDIR)/buffer-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/buffer-tst-dbg.passed: $(TSTDIR)/buffer-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/buffer-tst-rel: $(OBJDIR)/buffer-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/buffer-tst-rel.passed: $(TSTDIR)/buffer-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# text
$(TSTDIR)/text-tst-dbg: $(OBJDIR)/text-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/text-tst-dbg.passed: $(TSTDIR)/text-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/text-tst-rel: $(OBJDIR)/text-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/text-tst-rel.passed: $(TSTDIR)/text-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# reader
$(TSTDIR)/reader-tst-dbg: $(OBJDIR)/reader-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/reader-tst-dbg.passed: $(TSTDIR)/reader-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/reader-tst-rel: $(OBJDIR)/reader-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/reader-tst-rel.passed: $(TSTDIR)/reader-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# writer
$(TSTDIR)/writer-tst-dbg: $(OBJDIR)/writer-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/writer-tst-dbg.passed: $(TSTDIR)/writer-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/writer-tst-rel: $(OBJDIR)/writer-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/writer-tst-rel.passed: $(TSTDIR)/writer-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# txt-writer
$(TSTDIR)/txt-writer-tst-dbg: $(OBJDIR)/txt-writer-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/txt-writer-tst-dbg.passed: $(TSTDIR)/txt-writer-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/txt-writer-tst-rel: $(OBJDIR)/txt-writer-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/txt-writer-tst-rel.passed: $(TSTDIR)/txt-writer-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# serial
$(TSTDIR)/serial-tst-dbg: $(OBJDIR)/serial-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/serial-tst-dbg.passed: $(TSTDIR)/serial-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/serial-tst-rel: $(OBJDIR)/serial-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/serial-tst-rel.passed: $(TSTDIR)/serial-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# dserial
$(TSTDIR)/dserial-tst-dbg: $(OBJDIR)/dserial-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/dserial-tst-dbg.passed: $(TSTDIR)/dserial-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/dserial-tst-rel: $(OBJDIR)/dserial-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/dserial-tst-rel.passed: $(TSTDIR)/dserial-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# string
$(TSTDIR)/string-tst-dbg: $(OBJDIR)/string-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/string-tst-dbg.passed: $(TSTDIR)/string-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/string-tst-rel: $(OBJDIR)/string-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/string-tst-rel.passed: $(TSTDIR)/string-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# str-buffer
$(TSTDIR)/str-buffer-tst-dbg: $(OBJDIR)/str-buffer-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/str-buffer-tst-dbg.passed: $(TSTDIR)/str-buffer-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/str-buffer-tst-rel: $(OBJDIR)/str-buffer-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/str-buffer-tst-rel.passed: $(TSTDIR)/str-buffer-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# nconv-sel
$(TSTDIR)/nconv-sel-tst-dbg: $(OBJDIR)/nconv-sel-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/nconv-sel-tst-dbg.passed: $(TSTDIR)/nconv-sel-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/nconv-sel-tst-rel: $(OBJDIR)/nconv-sel-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/nconv-sel-tst-rel.passed: $(TSTDIR)/nconv-sel-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# nconv
$(TSTDIR)/nconv-tst-dbg: $(OBJDIR)/nconv-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/nconv-tst-dbg.passed: $(TSTDIR)/nconv-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/nconv-tst-rel: $(OBJDIR)/nconv-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/nconv-tst-rel.passed: $(TSTDIR)/nconv-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# fmt
$(TSTDIR)/fmt-tst-dbg: $(OBJDIR)/fmt-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/fmt-tst-dbg.passed: $(TSTDIR)/fmt-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/fmt-tst-rel: $(OBJDIR)/fmt-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/fmt-tst-rel.passed: $(TSTDIR)/fmt-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# kvp
$(TSTDIR)/kvp-tst-dbg: $(OBJDIR)/kvp-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/kvp-tst-dbg.passed: $(TSTDIR)/kvp-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/kvp-tst-rel: $(OBJDIR)/kvp-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/kvp-tst-rel.passed: $(TSTDIR)/kvp-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# buffmt
$(TSTDIR)/buffmt-tst-dbg: $(OBJDIR)/buffmt-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/buffmt-tst-dbg.passed: $(TSTDIR)/buffmt-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/buffmt-tst-rel: $(OBJDIR)/buffmt-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/buffmt-tst-rel.passed: $(TSTDIR)/buffmt-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# trace
$(TSTDIR)/trace-tst-dbg: $(OBJDIR)/trace-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/trace-tst-dbg.passed: $(TSTDIR)/trace-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/trace-tst-rel: $(OBJDIR)/trace-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/trace-tst-rel.passed: $(TSTDIR)/trace-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# ptrace
$(TSTDIR)/ptrace-tst-dbg: $(OBJDIR)/ptrace-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/ptrace-tst-dbg.passed: $(TSTDIR)/ptrace-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/ptrace-tst-rel: $(OBJDIR)/ptrace-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/ptrace-tst-rel.passed: $(TSTDIR)/ptrace-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# rsrc-trace
$(TSTDIR)/rsrc-trace-tst-dbg: $(OBJDIR)/rsrc-trace-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/rsrc-trace-tst-dbg.passed: $(TSTDIR)/rsrc-trace-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/rsrc-trace-tst-rel: $(OBJDIR)/rsrc-trace-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/rsrc-trace-tst-rel.passed: $(TSTDIR)/rsrc-trace-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# log
$(TSTDIR)/log-tst-dbg: $(OBJDIR)/log-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/log-tst-dbg.passed: $(TSTDIR)/log-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/log-tst-rel: $(OBJDIR)/log-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/log-tst-rel.passed: $(TSTDIR)/log-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# plog
$(TSTDIR)/plog-tst-dbg: $(OBJDIR)/plog-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/plog-tst-dbg.passed: $(TSTDIR)/plog-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/plog-tst-rel: $(OBJDIR)/plog-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/plog-tst-rel.passed: $(TSTDIR)/plog-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# rsrc-log
$(TSTDIR)/rsrc-log-tst-dbg: $(OBJDIR)/rsrc-log-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/rsrc-log-tst-dbg.passed: $(TSTDIR)/rsrc-log-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/rsrc-log-tst-rel: $(OBJDIR)/rsrc-log-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/rsrc-log-tst-rel.passed: $(TSTDIR)/rsrc-log-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# stream
$(TSTDIR)/stream-tst-dbg: $(OBJDIR)/stream-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/stream-tst-dbg.passed: $(TSTDIR)/stream-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/stream-tst-rel: $(OBJDIR)/stream-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/stream-tst-rel.passed: $(TSTDIR)/stream-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# obj
$(TSTDIR)/obj-tst-dbg: $(OBJDIR)/obj-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/obj-tst-dbg.passed: $(TSTDIR)/obj-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/obj-tst-rel: $(OBJDIR)/obj-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/obj-tst-rel.passed: $(TSTDIR)/obj-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

# base64
$(TSTDIR)/base64-tst-dbg: $(OBJDIR)/base64-tst.tst.dbg.o $(TESTDBGLIBDEPS)
	$(LD) -g -o $@ $(CFLAGS) $< $(TESTDBGLIBS)
$(TSTDIR)/base64-tst-dbg.passed: $(TSTDIR)/base64-tst-dbg
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

$(TSTDIR)/base64-tst-rel: $(OBJDIR)/base64-tst.tst.rel.o $(TESTRELLIBDEPS)
	$(LD) -O3 -o $@ $(CFLAGS) $< $(TESTRELLIBS)
$(TSTDIR)/base64-tst-rel.passed: $(TSTDIR)/base64-tst-rel
	@ echo -n "running $<... "
	@ $< 1> $<.rslt 2>&1 && ( touch $@ && echo passed ) || ( echo failed && cat $<.rslt && false )

CMNTESTS =         \
	buffer         \
	text           \
	reader         \
	writer         \
	txt-writer     \
	serial         \
	dserial        \
	string         \
	str-buffer     \
	nconv-sel      \
	nconv          \
	fmt            \
	kvp            \
	buffmt         \
	trace          \
	ptrace         \
	rsrc-trace     \
	log            \
	plog           \
	rsrc-log       \
	stream         \
	obj            \
	base64         \

CMNDBGTESTS = \
	$(addprefix $(TSTDIR)/,$(addsuffix -tst-dbg,$(CMNTESTS)))

CMNDBGTESTSPASS = \
	$(addprefix $(TSTDIR)/,$(addsuffix -tst-dbg.passed,$(CMNTESTS)))

CMNRELTESTS = \
	$(addprefix $(TSTDIR)/,$(addsuffix -tst-rel,$(CMNTESTS)))

CMNRELTESTSPASS = \
	$(addprefix $(TSTDIR)/,$(addsuffix -tst-rel.passed,$(CMNTESTS)))

test-objbin: $(OBJDIR) $(TSTDIR) $(CMNDBGTESTS) $(CMNRELTESTS)

TESTTARGS =        \
	$(CMNDBGTESTS) \
	$(CMNRELTESTS)

debug-tests: $(OBJDIR) $(TSTDIR) $(CMNDBGTESTS) $(CMNDBGTESTSPASS)

release-tests: $(OBJDIR) $(TSTDIR) $(CMNRELTESTS) $(CMNRELTESTSPASS)

tests: $(OBJDIR) $(TSTDIR) $(CMNDBGTESTS) $(CMNRELTESTS) $(CMNDBGTESTSPASS) $(CMNRELTESTSPASS)


# library
LIBTARGS =                        \
	$(LIBDIR)/libcmn-dbg.$(LIBX)  \
	$(LIBDIR)/libcmn-rel.$(LIBX)

CMNSRC =                \
	base64              \
	obj                 \
	stream              \
	rsrc-log            \
	log-impl            \
	plogger             \
	rsrc-trace          \
	trace-impl          \
	ptracer             \
	log                 \
	logger              \
	trace               \
	tracer              \
	buffmt              \
	kvp                 \
	fmt                 \
	nconv               \
	str-buffer          \
	str-iter            \
	string              \
	dserial-stream      \
	dserial             \
	serial              \
	txt-writer          \
	writer              \
	reader              \
	text-utf8      	    \
	buffer              \

debug: debug-tests $(OBJDIR) $(LIBDIR) $(LIBDIR)/libcmn-dbg.$(LIBX)

CMNDBGOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .dbg.$(OBJX),$(CMNSRC)))

compile-debug: $(OBJDIR) $(TSTDIR) $(CMNDBGTESTS) $(CMNDBGOBJ)

$(LIBDIR)/libcmn-dbg.$(LIBX): $(CMNDBGOBJ)
	ar -rc $@ $(CMNDBGOBJ)

release: release-tests $(OBJDIR) $(LIBDIR) $(LIBDIR)/libcmn-rel.$(LIBX)

CMNRELOBJ = \
	$(addprefix $(OBJDIR)/,$(addsuffix .rel.$(OBJX),$(CMNSRC)))

compile-release: $(OBJDIR) $(TSTDIR) $(CMNRELTESTS) $(CMNRELOBJ)

$(LIBDIR)/libcmn-rel.$(LIBX): $(CMNRELOBJ)
	ar -rc $@ $(CMNRELOBJ)

$(TESTTARGS) $(LIBTARGS): Makefile

clean:
	rm -rf $(OBJDIR) $(TSTDIR)
	rm -f $(LIBTARGS)
