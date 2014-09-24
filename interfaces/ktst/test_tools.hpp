#ifndef _h_ktst_test_tools_
#define _h_ktst_test_tools_

////////////////////////////////////////////////////////////////////////////////
// these macros are available outside of test cases' code

#define LOG(log_level, msg) \
    (log_level >= ncbi::NK::TestEnv::verbosity ? (std::cerr << msg) : std::cerr)

#define TEST_MESSAGE(M) \
{ ncbi_NK_saveLocation(__FILE__,__LINE__); \
    LOG(ncbi::NK::LogLevel::e_message, M << std::endl); }

#define TEST_CHECKPOINT(M) TEST_MESSAGE(M)

// report failure from a fixture
#define FAIL( msg_ )     \
      _REPORT_CRITICAL_ERROR_( (msg_), __FILE__, __LINE__, true )

#define GET_TEST_SUITE ncbi::NK::GetTestSuite

#endif// _h_ktst_test_tools_
