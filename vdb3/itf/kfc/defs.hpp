/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

#ifndef _hpp_vdb3_kfc_defs_
#define _hpp_vdb3_kfc_defs_

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>

namespace vdb3
{

    // raw integer number types
    typedef int8_t I8;
    typedef int16_t I16;
    typedef int32_t I32;
    typedef int64_t I64;

    // raw natural number types
    typedef uint8_t U8;
    typedef uint16_t U16;
    typedef uint32_t U32;
    typedef uint64_t U64;

    // raw real number types
    typedef float F32;
    typedef double F64;

    // raw character types
    typedef char ascii, UTF8;
    typedef U32 UTF32;

    // raw capabilities
    typedef U64 caps_t;
    typedef U64 rcaps_t;

    // an item count
    typedef U64 count_t;

    // an ordinal index
    typedef I64 index_t;

    // size of an item in bits
    class bitsz_t;

    // size of an item in bytes
    class bytes_t;

    // a timeout
    class timeout_t;

    // a data type
    class type_t;

    // a multi-part version number
    class vers_t;

    // even though I generally hate this kind of thing,
    // it will make some portions of the code more readable
    // look for interface declarations and implementations
#undef interface
#define interface struct
#undef implements
#define implements public
}

// turn the value of a pp-symbol into a string
#define stringize( tok ) tok_to_string ( tok )
#define tok_to_string( tok ) # tok

// generate string values for SrcLoc
#if ! defined __mod__ && defined __mod_path__
#define __mod__ stringize ( __mod_path__ )
#endif

#if ! defined __file__ && defined __file_name__
#define __file__ stringize ( __file_name__ )
#endif

#if ! defined __fext__ && defined __file_ext__
#define __fext__ stringize ( __file_ext__ )
#endif

#ifndef _hpp_vdb3_kfc_integer_
#include "integer.hpp"
#endif

#endif // _hpp_vdb3_kfc_defs_
