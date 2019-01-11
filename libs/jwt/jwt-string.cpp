/*==============================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any rem_striction on its use or reproduction.
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

#include <jwt/jwt-string.hpp>

#include <klib/text.h>
#include <klib/printf.h>

using namespace std;

namespace ncbi
{

    const size_t JwtString :: npos;

    static String emptyString = { "", 0, 0 };

    static void InitString( String & str, const char * v)
    {
        size_t size;
        uint32_t len = string_measure ( v, & size );
        StringInit( & str, v, size, len );
    }

    JwtString :: JwtString()
    : m_str ( & emptyString )
    {
    }
    JwtString :: JwtString(const char* v)
    {
        if ( v != nullptr )
        {
            String str;
            InitString ( str, v );
            rc_t rc = StringCopy ( & m_str, & str );
            //TODO: throw if rc != 0
        }
        //TODO: else throw
    }

    JwtString :: JwtString(const char* v, size_t sz)
    {
        if ( v != nullptr )
        {
            uint32_t len = string_len ( v, sz );
            String str;
            StringInit( & str, v, sz, len );
            rc_t rc = StringCopy ( & m_str, & str );
            //TODO: throw if rc != 0
        }
        //TODO: else throw
    }

    JwtString :: JwtString(const JwtString& v)
    {
        rc_t rc = StringCopy ( & m_str, v.m_str );
        //TODO: throw if rc != 0
    }

    JwtString :: ~JwtString()
    {
        clear();
    }

    bool
    JwtString :: empty() const
    {
        return m_str->size == 0;
    }

    const char*
    JwtString :: data() const
    {
        return m_str->addr;
    }

    size_t
    JwtString :: size() const
    {
        return m_str->size;
    }

    size_t
    JwtString :: find_first_of (char c, size_t pos) const
    {
        if ( pos >= m_str->size)
        {
            return npos;
        }
        char * chr = string_chr(m_str->addr + pos, m_str->size - pos, c);
        if ( chr == nullptr )
        {
            return npos;
        }
        return chr - m_str->addr;
    }

    size_t
    JwtString :: find_first_of (const char* c, size_t pos) const
    {
        if ( pos >= m_str->size)
        {
            return npos;
        }
        uint32_t clen = strlen ( c );
        size_t i = pos;
        while ( i < m_str->len )
        {
            char ch = m_str->addr [ i ];
            if ( string_chr ( c, clen, ch ) != nullptr )
            {
                return i;
            }
            ++i;
        }
        return npos;
    }

    JwtString
    JwtString :: substr (size_t pos, size_t len) const
    {
        if ( pos >= m_str->size)
        {
            return JwtString();
        }
        if ( len == npos || pos + len > m_str->size )
        {
            len = m_str->size - pos;
        }
        return JwtString ( m_str->addr + pos, len );
    }

    void
    JwtString :: clear ()
    {
        if ( m_str != & emptyString )
        {
            StringWhack ( m_str );
        }
        m_str = & emptyString;
    }

    int
    JwtString :: compare (const char* s) const
    {
        String str;
        InitString ( str, s );
        return StringCompare ( m_str, &str );
    }

    int
    JwtString :: compare (const JwtString& s) const
    {
        return StringCompare ( m_str, s.m_str );
    }

    int
    JwtString :: compare (size_t pos, size_t len, const JwtString& s,
                 size_t subpos, size_t sublen) const
    {
        if ( empty() )
        {
            if ( s.empty() )
            {
                return 0;
            }
            else
            {
                return -1;
            }
        }
        else if ( s.empty() )
        {
            return 1;
        }
        return string_cmp ( m_str->addr + pos, len,
                            s.m_str->addr + subpos, sublen,
                            m_str->size );
    }

    int
    JwtString :: compare (size_t pos, size_t len, const char* s) const
    {
        return compare( pos, len, s, string_size(s) );
    }

    int
    JwtString :: compare (size_t pos, size_t len, const char* s, size_t n) const
    {
        if ( empty() )
        {
            if ( s == nullptr || s[0] == 0 )
            {
                return 0;
            }
            else
            {
                return -1;
            }
        }
        else if ( s == nullptr || s[0] == 0 )
        {
            return 1;
        }
        return string_cmp ( m_str->addr + pos, len, s, n, m_str->size );
    }

    size_t
    JwtString :: find (const char* s, size_t pos) const
    {
        if ( pos >= m_str->size || s == nullptr )
        {
            return npos;
        }
        const char * res = strstr ( m_str->addr + pos, s );
        if ( res == nullptr )
        {
            return npos;
        }
        return res - m_str->addr;
    }

    JwtString&
    JwtString :: operator+= (const JwtString& s)
    {
        if ( empty() )
        {
            *this = s;
        }
        else if ( ! s.empty()  )
        {
            const String* cat;
            rc_t rc = StringConcat( & cat, m_str, s.m_str );
            //TODO throw if rc != 0
            clear();
            m_str = cat;
        }
        return *this;
    }

    JwtString&
    JwtString :: operator+= (char c)
    {
        return *this += JwtString(&c, 1);
    }

    const char&
    JwtString :: operator[] (size_t pos) const
    {
        if ( pos >= m_str->size )
        {
            static char zero = 0;
            return zero; //TODO: throw
        }
        return m_str->addr[pos];
    }

    bool operator<  (const JwtString& lhs, const JwtString& rhs)
    {
        return lhs.compare(rhs) < 0;
    }

    JwtString operator+ (const JwtString& lhs, const char* rhs)
    {
        if ( lhs.empty() )
        {
            return JwtString ( rhs );
        }
        else if ( rhs == nullptr || rhs [ 0 ] == 0 )
        {
            return lhs;
        }
        return lhs + JwtString ( rhs );
    }

    JwtString operator+ (const JwtString& lhs, const JwtString&   rhs)
    {
        if ( lhs.empty() )
        {
            return rhs;
        }
        else if ( rhs.empty() )
        {
            return lhs;
        }
        else
        {
            JwtString ret ( lhs );
            ret += rhs;
            return ret;
        }
    }

    JwtString operator+ (const JwtString& lhs, char rhs)
    {
        return lhs + JwtString(&rhs, 1);
    }

    int stoi (const JwtString& str, size_t* idx, int base)
    {
        char * endptr;
        int ret = (int)strtol( str.data(), & endptr, base );
        if  (idx != nullptr )
        {
            *idx = endptr - str.data();
        }
        return ret;
    }

    long long stoll (const JwtString& str, size_t* idx, int base)
    {
        char * endptr;
        long long ret = strtoll( str.data(), & endptr, base );
        if  (idx != nullptr )
        {
            *idx = endptr - str.data();
        }
        return ret;
    }

    long double stold (const JwtString& str, size_t* idx)
    {
        char * endptr;
        long double ret = strtold( str.data(), & endptr );
        if  (idx != nullptr )
        {
            *idx = endptr - str.data();
        }
        return ret;
    }

    JwtString to_string( unsigned value )
    {
        char buf[32];
        size_t num_writ;
        string_printf(buf, sizeof(buf), &num_writ, "%u", value);
        //TODO: handle rc != 0
        return JwtString(buf, num_writ);
    }

    JwtString to_string( long long value )
    {
        char buf[32];
        size_t num_writ;
        string_printf(buf, sizeof(buf), &num_writ, "%ld", value);
        //TODO: handle rc != 0
        return JwtString(buf, num_writ);
    }
}
