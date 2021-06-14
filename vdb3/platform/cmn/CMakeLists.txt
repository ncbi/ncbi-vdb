# # a once-generated data file
# $(BINDIR)/gen-base64-tbl: gen-base64-tbl.c
# 	gcc -o $@ gen-base64-tbl.c

# base64-tables.hpp: $(BINDIR)/gen-base64-tbl
# 	$< > $@

add_library(cmn STATIC
    base64.cpp
    buffer.cpp
    buffmt.cpp
    dserial-stream.cpp
    dserial.cpp
    fmt.cpp
    kvp.cpp
    log-impl.cpp
    log.cpp
    logger.cpp
    nconv.cpp
    obj.cpp
    plogger.cpp
    ptracer.cpp
    reader.cpp
    rsrc.cpp
    rsrc-log.cpp
    rsrc-trace.cpp
    serial.cpp
    str-buffer.cpp
    str-iter.cpp
    stream.cpp
    string.cpp
    text-utf8.cpp
    trace-impl.cpp
    trace.cpp
    tracer.cpp
    txt-writer.cpp
    writer.cpp
)

add_subdirectory(test)