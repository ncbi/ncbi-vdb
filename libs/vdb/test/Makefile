SYS=mac
CC=clang
ARCH=x86_64
BUILD=dbg
NCBI-VDB-DIR=$(HOME)/ncbi-outdir/ncbi-vdb/$(SYS)/$(CC)/$(ARCH)/$(BUILD)

INCLUDE=\
	-I ../../../interfaces/ \
	-I ../../../interfaces/cc/gcc \
	-I ../../../interfaces/cc/gcc/$(ARCH) \
	-I ../../../interfaces/os \
	-I ../../../interfaces/os/$(SYS) \
	-I ../../../interfaces/os/unix

LIBS=\
	-lkdb \
	-lklib \
	-lkfg \
	-lkfs \
	-lkns \
	-lkrypto \
	-lvfs \
	-lkproc \
    -lz \
    -lm

vdb=\
	../page-map.c \
	../blob-headers.c \
	../blob.c \
	../column-cmn.c \
	../column.c \
	../schema.c \
	../schema-db.c \
	../schema-dump.c \
	../schema-eval.c \
	../schema-expr.c \
	../schema-func.c \
	../schema-parse.c \
	../schema-prod.c \
	../schema-tbl.c \
	../schema-tok.c \
	../schema-type.c \
	../schema-int.c \
	../cursor.c \
	../cursor-cmn.c \
	../table-cmn.c \
	../table-load.c \
	../database-cmn.c \
	../database-load.c \
	../database.c \
	../dbmgr.c \
	../dbmgr-cmn.c \
	../phys-cmn.c \
	../phys-load.c \
	../phys.c \
	../prod-func.c \
	../prod-expr.c \
	../prod.c \
	../linker-cmn.c

SYS_DEFINES=-DUNIX -D_REENTRANT -D_FILE_OFFSET_BITS=64 -D_ARCH_BITS=__SIZEOF_POINTER__*__CHAR_BIT__ -DMAC -DBSD -mmacosx-version-min=10.10
LIB_DEFINES=$(SYS_DEFINES) -DLIBPREFIX=lib -D_LIBRARY -D__mod_name__=bogus -D__file_name__=bogus -D__file_ext__=c -DSHLIBEXT=dylib

.PHONY: clean

all: test-prod-cmn

test-prod-cmn: $(vdb) test-prod-cmn.c
	$(CC) $(CFLAGS) $(INCLUDE) -L$(NCBI-VDB-DIR)/ilib $(LIBS) $^ -o $@

clean:
	@rm -f test-prod-cmn
	@rm -f test-prod-cmn.dSYM

