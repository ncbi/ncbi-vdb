add_compile_definitions( __mod__="test/kproc" )

AddExecutableTest( Test_KPROC_kproctest "kproctest" "${COMMON_LIBS_READ};${ADDITIONAL_LIBS}" )