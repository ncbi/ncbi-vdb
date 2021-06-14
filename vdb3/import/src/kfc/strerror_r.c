/*

  vdb3.kfc.strerror_r

 */

#include "strerror_r.h"

void string_error_r ( int err, char * buffer, size_t bsize )
{
    ( void ) strerror_r ( err, buffer, bsize );
}


#if 0

    // here's a piece of code that uses constant globals
    // it's all apparently deprecated, but good to hang onto
    int status;
    if ( err >= 0 && err < sys_nerr && sys_errlist [ err ] != nullptr )
        status = :: snprintf ( xm . zmsg, sizeof xm . zmsg, "%s", sys_errlist [ err ] );
    else
        status = :: snprintf ( xm . zmsg, sizeof xm . zmsg, "Unknown error %d", err );

#endif
