/*

  vdb3.kfc.strerror_r

 */

#pragma once

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
    
    void string_error_r ( int err, char * buffer, size_t bsize );
        
#ifdef __cplusplus
}
#endif
