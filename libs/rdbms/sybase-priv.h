#ifndef _h_sybase_priv_
#define _h_sybase_priv_

#include <cspublic.h>
#include <ctpublic.h>

#ifdef CS_VERSION_150
#define HAVE_VERS_15 1
#else
#define HAVE_VERS_15 0
#endif

#ifndef _h_db_sybase_
#include <rdbms/sybase.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * SybaseColumn
 *  a multi-typed container for Sybase native types
 *
 *  these are currently allocated, without regard to size
 *  and held in the row as an array of pointers.
 *
 *  this logic should be changed so that the row holds an
 *  array of columns ( not pointers ) so that a large number
 *  of column data may be held directly in the array. for variable
 *  sized data that can be larger than a column, a column can be
 *  allocated and held in the "p" member of the array column.
 *
 *  indirect columns would therefore be detected as having a
 *  data size > sizeof column.
 */
typedef union SybaseColumn SybaseColumn;
union SybaseColumn
{
    CS_CHAR cs_char;                /* CS_CHAR_TYPE       =  0 */
    CS_BINARY cs_binary;            /* CS_BINARY_TYPE     =  1 */
    CS_LONGCHAR cs_longchar;        /* CS_LONGCHAR_TYPE   =  2 */
    CS_LONGBINARY cs_longbinary;    /* CS_LONGBINARY_TYPE =  3 */
    CS_TINYINT cs_tinyint;          /* CS_TINYINT_TYPE    =  6 */
    CS_SMALLINT cs_smallint;        /* CS_SMALLINT_TYPE   =  7 */
    CS_INT cs_int;                  /* CS_INT_TYPE        =  8 */
    CS_REAL cs_real;                /* CS_REAL_TYPE       =  9 */
    CS_FLOAT cs_float;              /* CS_FLOAT_TYPE      = 10 */
    CS_BIT cs_bit;                  /* CS_BIT_TYPE        = 11 */
    CS_DATETIME cs_datetime;        /* CS_DATETIME_TYPE   = 12 */
    CS_DATETIME4 cs_datetime4;      /* CS_DATETIME4_TYPE  = 13 */
    CS_NUMERIC cs_numeric;          /* CS_NUMERIC_TYPE    = 16 */
    CS_DECIMAL cs_decimal;          /* CS_DECIMAL_TYPE    = 17 */
    CS_VARCHAR cs_varchar;          /* CS_VARCHAR_TYPE    = 18 */
    CS_VARBINARY cs_varbinary;      /* CS_VARBINARY_TYPE  = 19 */
    CS_LONG cs_long;                /* CS_LONG_TYPE       = 20 */
    CS_USHORT cs_ushort;            /* CS_USHORT_TYPE     = 24 */
#if !defined(SYBDBLIB)
    CS_UNICHAR cs_unichar [ 1 ];    /* CS_UNICHAR_TYPE    = 25 */
#endif

#if HAVE_VERS_15
    CS_BIGINT cs_bigint;            /* CS_BIGINT_TYPE     = 30 */
    CS_USMALLINT cs_usmallint;      /* CS_USMALLINT_TYPE  = 31 */
    CS_UINT cs_uint;                /* CS_UINT_TYPE       = 32 */
    CS_UBIGINT cs_ubigint;          /* CS_UBIGINT_TYPE    = 33 */
#endif


    uint8_t data [ 1 ];
    int8_t sdata [ 1 ];
    char text [ 1 ];

    const SybaseColumn *p;
};


/*--------------------------------------------------------------------------
 * SybaseRow
 *  built by the result set, it contains no explicit Sybase objects
 */
typedef struct SybaseRS SybaseRS;
typedef struct SybaseRow SybaseRow;
struct SybaseRow
{
    /* polymorphism */
    DBRow par;

    /* borrowed reference to result set
       NOT REFERENCE COUNTED */
    const SybaseRS *rs;

    /* column data */
    unsigned int num_cols;
    struct
    {
        SybaseColumn data;
        size_t size;
    } cols [ 1 ];
};


/*--------------------------------------------------------------------------
 * SybaseRS
 *  by the time this guy is in effect, it is supposed to retrieve
 *  and create rows from the connection.
 *
 *  due to the need to demarshal objects coming from a wire-protocol,
 *  there is some state involved in a result set. the most natural way
 *  for the library to process the protocol is via the "Sybase loop,"
 *  although this requires surrendering control of the thread to the
 *  Sybase client library.
 *
 *  in order to retain control of the thread, the loop must be broken,
 *  and so this object behaves as a state machine.
 *
 *  these are effectively the states of a result set:
 *   1. no data to return
 *   2. has row data to return
 *   3. has only status code to return
 */
struct SybaseRS
{
    /* polymorphism */
    DBResultSet par;

    /* borrowed reference */
    CS_COMMAND *cmd;

    /* last retcode from ct_results */
    CS_RETCODE retcode;

    /* the number of rows that have been fetched into the
       client side buffers within ctlib */
    CS_INT num_rows;

    /* a description of the columns */
    CS_INT num_cols;
    CS_DATAFMT *col_info;

    /* if the query was a stored procedure, its return result */
    CS_INT status;
    CS_DATAFMT proc_info;

    /* operational state */
    bool have_rows;
    bool have_status;
};


/*--------------------------------------------------------------------------
 * SybaseDB
 */
typedef struct SybaseDB SybaseDB;
struct SybaseDB
{
    Database par;
    CS_CONNECTION *conn;
    CS_COMMAND *cmd;
};

extern Database_vt gSybaseDB_vt;


/*--------------------------------------------------------------------------
 * Sybase
 */
typedef struct Sybase Sybase;
struct Sybase
{
    DBManager par;
    CS_CONTEXT *context;
};


#ifdef __cplusplus
}
#endif

#endif /* _h_sybase_priv_ */
