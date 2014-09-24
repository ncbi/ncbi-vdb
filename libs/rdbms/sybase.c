/*==============================================================================
 *
 * sybase.c
 *
 *   the rdbms interface implemented herein has a Database, which represents
 *   a connection to the server in Sybase and optionally a "use <db>" statement,
 *   a DBResultSet which is really where everything takes place, and the
 *   DBRow which is a relatively simple in-memory representation of a row.
 *
 *   Sybase uses a connection object to represent its ( normally ) TCP link
 *   to the server. the session that it superimposes on the connection is
 *   known as a command.
 *
 *   the command basically gets stuffed with some indication of what action
 *   is to be taken by the server - in our case a SQL "language" string - but
 *   these could be prepared and parameterized forms. once the command has
 *   been built, it is sent to the server for processing.
 *
 *   so far, the interface is fairly clean and simple, albeit ugly and poorly
 *   specified, e.g. taking non-const char* values for strings that are
 *   intended to be treated as const, but with no assurances in the API.
 *
 *   the majority of work rests in processing the server response, which is
 *   to be remembered as a marshaled series of "results" over the connection.
 *   at the outermost level, this unit needs to read ( the header of ) each
 *   result block until no more blocks are available, indicated by the return
 *   status code from "ct_results". it is possible to have more than one
 *   result in your return, which is an unfortunate jumbling of terminology.
 *   each result is associated with a particular return data type. the common
 *   SQL queries of the sort "select blah from table..." will return all
 *   rows in a single result ( block ). a query involving a stored procedure
 *   always returns a status code with zero or more parameters, and apparently
 *   a role. in this case, the status is returned as one result while the
 *   parameters would be another result, because they have differing row
 *   structures. each would have a single row, incidentally.
 *
 *   once a result is obtained, it will either have associated data or not,
 *   and the data we care about are going to be row structured data. these
 *   are obtained via "ct_fetch" and "ct_get_data", where the former
 *   apparently demarshals and the latter copies to our own structures.
 *   the interface is designed such that we need to call "ct_fetch" ad nauseum
 *   or at least while it succeeds, since doing so advances the connection
 *   data pointer to the end of the current result. this is required if
 *   the next call to "ct_results" is going to succeed, or at least so it
 *   would seem.
 *
 *   another way to advance to the next result is to use "ct_cancel" with
 *   a parameter of CS_CANCEL_CURRENT to request that the ct library skip
 *   to the end of the current result. 
 */

#include "rdbms-priv.h"
#include "sybase-priv.h"

#include <klib/log.h>
#include <klib/printf.h>

#include <sysalloc.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#ifndef SRV_MAXERROR
#define SRV_MAXERROR 20000
#endif

#ifndef PACKETSIZE
#define PACKETSIZE ( 512 * 7 )
#endif

#if _DEBUGGING
#define LOG_STATUS_CODES 0
#endif

#if LOG_STATUS_CODES
#define LOG_INIT_CODES 1
#define LOG_CONN_CODES 1
#define LOG_EXEC_CODES 1
#define LOG_RETR_CODES 1
#endif

/*--------------------------------------------------------------------------
 * db error codes
 */

/*--------------------------------------------------------------------------
 * logging support
 */
/*static
const char *ctxlat ( CS_RETCODE retcode )
{
    switch ( retcode )
    {
#define CASE( code ) \
        case code: return # code
        CASE ( CS_SUCCEED );
        CASE ( CS_FAIL );
        CASE ( CS_MEM_ERROR );
        CASE ( CS_PENDING );
        CASE ( CS_QUIET );
        CASE ( CS_BUSY );
        CASE ( CS_INTERRUPT );
        CASE ( CS_BLK_HAS_TEXT );
        CASE ( CS_CONTINUE );
        CASE ( CS_FATAL );
        CASE ( CS_RET_HAFAILOVER );
        CASE ( CS_CANCELED );
        CASE ( CS_ROW_FAIL );
        CASE ( CS_END_DATA );
        CASE ( CS_END_RESULTS );
        CASE ( CS_END_ITEM );
        CASE ( CS_NOMSG );
        CASE ( CS_TIMED_OUT );
#if !defined(SYBDBLIB)
        CASE ( CS_PASSTHRU_EOM );
        CASE ( CS_PASSTHRU_MORE );
        CASE ( CS_TRYING );
#endif
#undef CASE
    }
    return "<UNKNOWN>";
}

static
const char *rtxlat ( CS_INT res_type )
{
    switch ( res_type )
    {
#define CASE( code ) \
        case code: return # code
        CASE ( CS_CMD_DONE );
        CASE ( CS_CMD_FAIL );
        CASE ( CS_CMD_SUCCEED );
        CASE ( CS_COMPUTE_RESULT );
        CASE ( CS_CURSOR_RESULT );
        CASE ( CS_PARAM_RESULT );
        CASE ( CS_ROW_RESULT );
        CASE ( CS_STATUS_RESULT );
        CASE ( CS_COMPUTEFMT_RESULT );
        CASE ( CS_ROWFMT_RESULT );
        CASE ( CS_MSG_RESULT );
        CASE ( CS_DESCRIBE_RESULT );
#undef CASE
    }
    return "<UNKNOWN>";
}*/

#if LOG_STATUS_CODES
static
CS_RETCODE log_status_code ( CS_RETCODE retcode, const char *func )
{
    /*logmsg ( "%s => %s ( %d )\n", func, ctxlat ( retcode ), retcode );*/
    PLOGERR(klogInt,
       (klogInt, RC(rcRDBMS, rcNoTarg, rcExecuting, rcError, rcExists),
        "$(f) => $(reason) ($(code))",
        "f=%s,reason=%s,code=%d", func, ctxlat(retcode), retcode);
    return retcode;
}

static
CS_RETCODE log_status_msgcode ( CS_RETCODE retcode, const char *func, const char *msg )
{
    /*logmsg ( "%s ( %s ) => %s ( %d )\n", func, msg, ctxlat ( retcode ), retcode );*/
    PLOGERR(klogInt,
       (klogInt, RC(rcRDBMS, rcNoTarg, rcExecuting, rcError, rcExists),
        "$(f) ($(msg)) => $(reason) ($(code))",
        "f=%s,msg=%s,reason=%s,code=%d", func, msg, ctxlat(retcode), retcode);
    return retcode;
}
#endif

#if LOG_INIT_CODES
#define LOG_INIT( func, retcode ) \
    log_status_code ( retcode, #func )
#define LOG_INIT_MSG( func, retcode, msg ) \
    log_status_msgcode ( retcode, #func, msg )
#define CALL_INIT( func, args ) \
    log_status_code ( func args, #func )
#define CALL_INIT_MSG( func, args, msg ) \
    log_status_msgcode ( func args, #func, msg )
#else
#define LOG_INIT( func, retcode ) \
    ( ( void ) 0 )
#define LOG_INIT_MSG( func, retcode, msg ) \
    ( ( void ) 0 )
#define CALL_INIT( func, args ) \
    func args
#define CALL_INIT_MSG( func, args, msg ) \
    func args
#endif

#if LOG_CONN_CODES
#define LOG_CONN( func, retcode ) \
    log_status_code ( retcode, #func )
#define LOG_CONN_MSG( func, retcode, msg ) \
    log_status_msgcode ( retcode, #func, msg )
#define CALL_CONN( func, args ) \
    log_status_code ( func args, #func )
#define CALL_CONN_MSG( func, args, msg ) \
    log_status_msgcode ( func args, #func, msg )
#else
#define LOG_CONN( func, retcode ) \
    ( ( void ) 0 )
#define LOG_CONN_MSG( func, retcode, msg ) \
    ( ( void ) 0 )
#define CALL_CONN( func, args ) \
    func args
#define CALL_CONN_MSG( func, args, msg ) \
    func args
#endif

#if LOG_EXEC_CODES
#define LOG_EXEC( func, retcode ) \
    log_status_code ( retcode, #func )
#define LOG_EXEC_MSG( func, retcode, msg ) \
    log_status_msgcode ( retcode, #func, msg )
#define CALL_EXEC( func, args ) \
    log_status_code ( func args, #func )
#define CALL_EXEC_MSG( func, args, msg ) \
    log_status_msgcode ( func args, #func, msg )
#else
#define LOG_EXEC( func, retcode ) \
    ( ( void ) 0 )
#define LOG_EXEC_MSG( func, retcode, msg ) \
    ( ( void ) 0 )
#define CALL_EXEC( func, args ) \
    func args
#define CALL_EXEC_MSG( func, args, msg ) \
    func args
#endif

#if LOG_RETR_CODES
#define LOG_RETR( func, retcode ) \
    log_status_code ( retcode, #func )
#define LOG_RETR_MSG( func, retcode, msg ) \
    log_status_msgcode ( retcode, #func, msg )
#define CALL_RETR( func, args ) \
    log_status_code ( func args, #func )
#define CALL_RETR_MSG( func, args, msg ) \
    log_status_msgcode ( func args, #func, msg )
#else
#define LOG_RETR( func, retcode ) \
    ( ( void ) 0 )
#define LOG_RETR_MSG( func, retcode, msg ) \
    ( ( void ) 0 )
#define CALL_RETR( func, args ) \
    func args
#define CALL_RETR_MSG( func, args, msg ) \
    func args
#endif



/*--------------------------------------------------------------------------
 * SybaseRow
 *  needs to contain an optional status code
 *  and zero or more named columns
 */

/* SybaseRowWhack
 */
static
rc_t SybaseRowWhack ( SybaseRow *self )
{
    if ( self != NULL )
    {
        unsigned int i;
        for ( i = 0; i < self -> num_cols; ++ i )
        {
            SybaseColumn *col;
            if ( self -> cols [ i ] . size > sizeof * col )
            {
                col = ( SybaseColumn* ) self -> cols [ i ] . data . p;
                if ( col != NULL )
                    free ( col );
            }
        }
        free ( self );
    }
    return 0;
}

/* SybaseRowNumColumns
 */
static
rc_t SybaseRowNumColumns ( const SybaseRow *self, unsigned int *num_cols )
{
    if ( num_cols == NULL )
        return statusToRC(dbInvalid);
    * num_cols = self -> num_cols;
    return 0;
}

/* SybaseRowColInfo
 */
static
rc_t SybaseRSColInfo ( const SybaseRS *rs, unsigned int idx,
    int *col_type, const String **name );

static
rc_t SybaseRowColInfo ( const SybaseRow *self, unsigned int idx,
    int *col_type, const String **name )
{
    /* TBD - this may need to override sizing of numeric and decimal type */
    return SybaseRSColInfo ( self -> rs, idx, col_type, name );
}

/* SybaseRowColIdx
 */
static
rc_t SybaseRSColIdx ( const SybaseRS *rs, const String *name, unsigned int *idx );

static
rc_t SybaseRowColIdx ( const SybaseRow *self, const String *name, unsigned int *idx )
{
    return SybaseRSColIdx ( self -> rs, name, idx );
}

/* SybaseRowIsNull
 */
static
bool SybaseRowIsNull ( const SybaseRow *self, unsigned int idx )
{
    if ( idx >= self -> num_cols )
        return false;
    return ( bool ) ( self -> cols [ idx ] . size == 0 );
}

/* SybaseRowGetAs...
 */
static
rc_t SybaseRowGetAsNum ( const SybaseRow *self, unsigned int idx, int64_t *i, double *d, bool *isflt )
{
    size_t size;
    bool is_floating;
    const CS_DATAFMT *fmt;
    const SybaseColumn *col;

    if ( idx >= self -> num_cols )
        return statusToRC(dbInvalid);

    fmt = & self -> rs -> col_info [ idx ];
    size = self -> cols [ idx ] . size;
    col = & self -> cols [ idx ] . data;

    if ( size == 0 )
        return statusToRC(dbNullColumn);

    if ( size > sizeof * col )
        col = col -> p;

    is_floating = false;
    switch ( fmt -> datatype )
    {
    case CS_TINYINT_TYPE:
        * i = col -> cs_tinyint;
        break;
    case CS_SMALLINT_TYPE:
        * i = col -> cs_smallint;
        break;
    case CS_INT_TYPE:
        * i = col -> cs_int;
        break;
    case CS_REAL_TYPE:
        * d = col -> cs_real;
        is_floating = true;
        break;
    case CS_FLOAT_TYPE:
        * d = col -> cs_float;
        is_floating = true;
        break;
    case CS_BIT_TYPE:
        * i = col -> cs_bit != 0;
        break;
    case CS_NUMERIC_TYPE:
    case CS_DECIMAL_TYPE:
        /* require scale, sign, mantissa */
        if ( size < 3 )
            * i = 0;
        else
        {
            size_t n;
            int scale;
            int64_t numeric;

            /* Sybase documentation reads:

                 "'precision' is the maximum number of decimal digits that are represented.
                 At the current time, legal values for precision are from 1 to 77. The
                 default precision is 18. CS_MIN_PREC, CS_MAX_PREC, and CS_DEF_PREC define 
                 the minimum, maximum, and default precision values, respectively."

                 "'array' is a base-256 representation of the numeric value. Byte 0 of the
                 array contains the most significant byte."

               This is what would appear to be what we in the Royal Navy call a lie.
               A numeric with precision of 12 claims a size of 8. When the value is -1,
               the contents of the array are 1,0,0,0,0,1. In other words, array [ 0 ] is
               a boolean for sign inversion, or so it would seem. They go on to state:

                 "The number of bytes used in array is based on the selected precision of
                 the numeric. Mapping is performed based on the precision of the numeric to
                 the length of array that is used."

                 "'scale' is the maximum number of digits to the right of the decimal point.
                 At the current time, legal values for scale are from 0 to 77. The default
                 scale is 0. CS_MIN_SCALE, CS_MAX_SCALE, and CS_DEF_PREC define the minimum,
                 maximum, and default scale values, respectively."

                 "'scale' must be less than or equal to 'precision'."

            */

            /* accumulate mantissa */
            for ( numeric = 0, n = 3; n < size; ++ n )
            {
                numeric <<= 8;
                numeric |= ( uint8_t ) col -> cs_numeric . array [ n - 2 ];
            }

            /* negate if sign set */
            if ( col -> cs_numeric . array [ 0 ] )
                numeric = - numeric;

            /* scale */
            scale = col -> cs_numeric . scale;
            if ( scale )
            {
                double power = 10.0;
                is_floating = true;
                if ( scale > 0 )
                {
                    double divisor = 0.0;
                    while ( 1 )
                    {
                        if ( scale & 1 )
                            divisor += power;
                        if ( ( scale >>= 1 ) == 0 )
                            break;
                        power *= power;
                    }
                    * d = ( double ) numeric / divisor;
                }
                else
                {
                    double factor = 0.0;
                    scale = - scale;
                    while ( 1 )
                    {
                        if ( scale & 1 )
                            factor += power;
                        if ( ( scale >>= 1 ) == 0 )
                            break;
                        power *= power;
                    }
                    * d = ( double ) numeric * factor;
                }
            }

            else
            {
                * i = numeric;
            }
        }
        break;
    case CS_LONG_TYPE:
        * i = col -> cs_long;
        break;
    case CS_USHORT_TYPE:
        * i = col -> cs_ushort;
        break;
#if HAVE_VERS_15
    case CS_BIGINT_TYPE:
        * i = col -> cs_bigint;
        break;
    case CS_USMALLINT_TYPE:
        * i = col -> cs_usmallint;
        break;
    case CS_UINT_TYPE:
        * i = col -> cs_uint;
        break;
    case CS_UBIGINT_TYPE:
        * i = ( int64_t ) col -> cs_ubigint;
        break;
#endif
    default:
        return statusToRC(dbUnknownErr);
    }

    * isflt = is_floating;
    return 0;
}


static
rc_t GetString ( const String **value, const char *text, size_t bytes )
{
    String str;
    unsigned int len = string_len ( text, bytes );
    StringInit ( & str, text, bytes, len );
    return StringCopy ( value, & str );
}

static
rc_t SybaseRowGetAsString ( const SybaseRow *self, unsigned int idx, const String **value )
{
    rc_t rc = 0;
    size_t size;
    const CS_DATAFMT *fmt;
    const SybaseColumn *col;

    if ( value == NULL || idx >= self -> num_cols )
        return statusToRC(dbInvalid);

    fmt = & self -> rs -> col_info [ idx ];
    size = self -> cols [ idx ] . size;
    col = & self -> cols [ idx ] . data;

    if ( size == 0 )
        return statusToRC(dbNullColumn);

    if ( size > sizeof * col )
        col = col -> p;

    switch ( fmt -> datatype )
    {
    case CS_CHAR_TYPE:
    case CS_LONGCHAR_TYPE:
    case CS_TEXT_TYPE:
        return GetString ( value, col -> text, size );

    case CS_VARCHAR_TYPE:
        return GetString ( value, col -> cs_varchar . str, col -> cs_varchar . len );

#if !defined(SYBDBLIB)
    case CS_UNICHAR_TYPE:
        return StringCopyUTF16 ( value, col -> cs_unichar, size );
#endif

    default:
    {
          double d;
          int64_t i;
          bool is_floating;

          rc = SybaseRowGetAsNum ( self, idx, & i, & d, & is_floating );
          if ( rc == 0 )
          {
              char buffer [ 64 ];
              if ( is_floating )
                  rc = string_printf ( buffer, sizeof buffer, & size, "%f", d );
              else
                  rc = string_printf ( buffer, sizeof buffer, & size, "%ld", i );
              if ( rc == 0 )
                  rc = GetString ( value, buffer, size );
          }
    }}

    return rc;
}

static
rc_t SybaseRowGetAsBlob ( const SybaseRow *self, unsigned int idx, const void **value, size_t *bytes )
{
    const CS_DATAFMT *fmt;
    const SybaseColumn *col;

    if ( value == NULL || bytes == NULL || idx >= self -> num_cols )
        return statusToRC(dbInvalid);

    fmt = & self -> rs -> col_info [ idx ];
    * bytes = self -> cols [ idx ] . size;
    col = & self -> cols [ idx ] . data;

    if ( * bytes == 0 )
        return statusToRC(dbNullColumn);

    if ( * bytes > sizeof * col )
        col = col -> p;

    switch ( fmt -> datatype )
    {
    case CS_VARBINARY_TYPE:
        * value = col -> cs_varbinary . array;
        * bytes = col -> cs_varbinary . len;
        break;

    case CS_VARCHAR_TYPE:
        * value = col -> cs_varchar . str;
        * bytes = col -> cs_varchar . len;
        break;
    }

    return 0;
}

static
void WhackBlob ( void *priv )
{
    if ( priv )
        free ( priv );
}

static
rc_t SybaseRowStealBlob ( SybaseRow *self, unsigned int idx, DBBlob *blob )
{
    const CS_DATAFMT *fmt;
    const SybaseColumn *col;

    if ( blob == NULL || idx >= self -> num_cols )
        return statusToRC(dbInvalid);

    blob -> whack = WhackBlob;

    fmt = & self -> rs -> col_info [ idx ];
    blob -> size = self -> cols [ idx ] . size;
    col = & self -> cols [ idx ] . data;

    if ( blob -> size == 0 )
    {
        blob -> addr = blob -> priv = NULL;
        return statusToRC(dbNullColumn);
    }

    if ( blob -> size <= sizeof * col )
    {
        blob -> priv = malloc ( blob -> size );
        if ( blob -> priv == NULL )
        {
            blob -> addr = NULL;
            return statusToRC(dbMemErr);
        }
        memcpy ( blob -> priv, col, blob -> size );
        col = blob -> priv;
    }
    else
    {
        col = col -> p;
        self -> cols [ idx ] . size = 0;
        blob -> priv = ( void* ) col;
    }

    blob -> addr = blob -> priv;

    switch ( fmt -> datatype )
    {
    case CS_VARBINARY_TYPE:
        blob -> addr = ( void* ) col -> cs_varbinary . array;
        blob -> size = col -> cs_varbinary . len;
        break;

    case CS_VARCHAR_TYPE:
        blob -> addr = ( void* ) col -> cs_varchar . str;
        blob -> size = col -> cs_varchar . len;
        break;
    }

    return 0;
}

/* SybaseRowInit
 */
static DBRow_vt sSybaseRow_vt =
{
    ( rc_t ( * ) ( DBRow* ) ) SybaseRowWhack,
    ( rc_t ( * ) ( const DBRow*, unsigned int *num_cols ) ) SybaseRowNumColumns,
    ( rc_t ( * ) ( const DBRow*, unsigned int,
        int *col_type, const String **name ) ) SybaseRowColInfo,
    ( rc_t ( * ) ( const DBRow*, const String *name,
        unsigned int *idx ) ) SybaseRowColIdx,
    ( bool ( * ) ( const DBRow*, unsigned int ) ) SybaseRowIsNull,

    ( rc_t ( * ) ( const DBRow*, unsigned int, int64_t*, double*, bool* ) ) SybaseRowGetAsNum,
    ( rc_t ( * ) ( const DBRow*, unsigned int, const String** ) ) SybaseRowGetAsString,
    ( rc_t ( * ) ( const DBRow*, unsigned int, const void**, size_t* ) ) SybaseRowGetAsBlob,
    ( rc_t ( * ) ( DBRow*, unsigned int, DBBlob* ) ) SybaseRowStealBlob
};

static
void SybaseRowInit ( SybaseRow *row, const SybaseRS *rs )
{
    row -> par . vt = & sSybaseRow_vt;
    row -> rs = rs;
    row -> num_cols = rs -> num_cols;
    memset ( row -> cols, 0, rs -> num_cols * sizeof row -> cols [ 0 ] );
}


/*--------------------------------------------------------------------------
 * SybaseRS
 *  the result set is created with a borrowed reference to the db's
 *  command object. its responsibility upon creation is to read the
 *  header of the first result in preparation for being queried.
 *
 *  this preparation involves calling ct_results until a reasonable
 *  result is obtained - either a row result or a status result - or
 *  until no more results are available.
 *
 *  when queried for "hasMoreRows", it uses the return code status
 *  obtained from preparation to determine whether or not it has more
 *  rows, indicated by a code of CS_SUCCEED.
 *
 *  when queried for "nextRow" it uses the result type state to determine
 *  whether this is a single stored procedure status [ + params ] row or
 *  a possibly multiple normal row. it then creates a row object and
 *  invokes ct_fetch and ct_get_data to build the row.
 */

/* SybaseRSGetColInfo
 *  reads column info on first row
 */
static
int SybaseRSGetColInfo ( SybaseRS *self )
{
    CS_INT i;

    /* retrieve number of columns */
    CS_RETCODE retcode = CALL_RETR ( ct_res_info, ( self -> cmd, CS_NUMDATA,
        & self -> num_cols, sizeof self -> num_cols, 0 ) );
    if ( retcode != CS_SUCCEED )
    {
      /*logmsg ( "ERROR: failed to retrieve number of columns\n" );
        LOGERR(klogInt, -1, "ERROR: failed to retrieve number of columns");*/
        return dbUnknownErr;
    }

    /* create an array of column descriptors */
    self -> col_info = malloc ( self -> num_cols * sizeof self -> col_info [ 0 ] );
    if ( self -> col_info == NULL )
    {
      /*logerr ( errno, "failed to allocate %d rows of column info", self -> num_cols );
        PLOGERR(klogInt, (klogInt, -1,
            "failed to allocate $(n) rows of column info", "n=%s",
            self->num_cols));*/
        return dbMemErr;
    }

    /* retrieve each column's description */
    for ( i = 0; i < self -> num_cols; ++ i )
    {
        CS_DATAFMT *fmt = & self -> col_info [ i ];
        retcode = CALL_RETR ( ct_describe, ( self -> cmd, i + 1, fmt ) );
        if ( retcode != CS_SUCCEED )
            return dbUnknownErr;
        fmt -> name [ fmt -> namelen ] = 0;
    }

    return dbNoErr;
}

/* SybaseRSFetch
 */
static
int SybaseRSFetch ( SybaseRS *self )
{
    /* read off of wire and capture number of rows fetched */
    CS_RETCODE retcode = CALL_RETR ( ct_fetch, ( self -> cmd,
        CS_UNUSED, CS_UNUSED, CS_UNUSED, & self -> num_rows ) );

    /* handle return result */
    switch ( retcode )
    {
    case CS_CANCELED:
        if ( self -> num_rows == 0 )
        {
            self -> have_rows = false;
            return dbEndData;
        }
    case CS_SUCCEED:
        return dbNoErr;

    case CS_END_DATA:
        self -> num_rows = 0;
        self -> have_rows = false;
        return dbEndData;

    case CS_ROW_FAIL:
        CALL_RETR ( ct_cancel, ( 0, self -> cmd, CS_CANCEL_CURRENT ) );
        break;

    case CS_FAIL:
        CALL_RETR ( ct_cancel, ( 0, self -> cmd, CS_CANCEL_ALL ) );
        break;
    }

    self -> num_rows = 0;
    self -> have_rows = false;

    return dbFetchErr;
}

/* SybaseRSFetchStatus
 *  fetch the return code - sometimes used for data
 */
static
int SybaseRSFetchStatus ( SybaseRS *self )
{
    int status;

    /* get the status return name, mostly */
    CS_RETCODE retcode = CALL_RETR ( ct_describe, ( self -> cmd,
        1, & self -> proc_info ) );
    if ( retcode != CS_SUCCEED )
        return dbUnknownErr;

    /* terminate name in paranoia */
    self -> proc_info . name [ self -> proc_info . namelen ] = 0;

    /* read the status data on wire as a row */
    status = SybaseRSFetch ( self );
    if ( status == dbNoErr && self -> num_rows > 0 )
    {
        /* read the status code into our object */
        retcode = CALL_RETR_MSG ( ct_get_data, ( self -> cmd,
            1, & self -> status, sizeof self -> status, NULL ), "status" );
        switch ( retcode )
        {
        case CS_SUCCEED:
        case CS_END_ITEM:
        case CS_END_DATA:
            break;

        default:
            status = dbFetchErr;
        }

        /* induce end of data by fetch of non-existent row */
        CALL_RETR_MSG ( ct_fetch, ( self -> cmd, CS_UNUSED,
            CS_UNUSED, CS_UNUSED, & self -> num_rows ), "find end of result" );

        /* if the status code read went well, we have the status */
        if ( status == dbNoErr )
            self -> have_status = true;
    }

    return status;
 }

/* SybaseRSPrepare
 *  prepares result set after query execution
 */
static
int SybaseRSPrepare ( SybaseRS *self )
{
    int status;

    CS_INT res_type;
    self -> retcode = CALL_RETR ( ct_results, ( self -> cmd, & res_type ) );
    for ( status = 0; self -> retcode == CS_SUCCEED; )
    {
        switch ( res_type )
        {
        case CS_CMD_FAIL:
            if ( status == dbNoErr )
                status = dbUnknownErr;
        case CS_CMD_DONE:
        case CS_CMD_SUCCEED:
            self -> num_rows = 0;
            self -> have_rows = false;
            break;

        case CS_ROW_RESULT:
        case CS_PARAM_RESULT:
            self -> have_rows = true;
            return SybaseRSGetColInfo ( self );

        case CS_STATUS_RESULT:
            status = SybaseRSFetchStatus ( self );
            if ( status != dbNoErr )
                CALL_RETR ( ct_cancel, ( 0, self -> cmd, CS_CANCEL_CURRENT ) );
            break;

        case CS_COMPUTE_RESULT:
        case CS_CURSOR_RESULT:

        case CS_COMPUTEFMT_RESULT:
        case CS_ROWFMT_RESULT:
        case CS_MSG_RESULT:
        case CS_DESCRIBE_RESULT:
          /*logmsg ( "INTERNAL ERROR: not handling result type %s\n", rtxlat ( res_type ) );
            PLOGERR(klogInt, (klogInt, -1,
                "INTERNAL ERROR: not handling result type $(t)", "t=%s",
                rtxlat(res_type)));*/
            CALL_RETR ( ct_cancel, ( 0, self -> cmd, CS_CANCEL_CURRENT ) );
            break;

        default:
   /*       logmsg ( "INTERNAL ERROR: unknown result type code %d\n", res_type );
            PLOGERR(klogInt, (klogInt, -1,
                "INTERNAL ERROR: unknown result type code $(t)", "t=%s",
                res_type));*/
            CALL_RETR ( ct_cancel, ( 0, self -> cmd, CS_CANCEL_CURRENT ) );
        }

        self -> retcode = CALL_RETR ( ct_results, ( self -> cmd, & res_type ) );
    }

    return status;
}

/* SybaseRSWhack
 */
static
rc_t SybaseRSWhack ( SybaseRS *self )
{
    if ( self != NULL )
    {
        if ( self -> retcode != CS_END_RESULTS )
        {
            do
            {
                CS_INT res_type;
                CALL_RETR ( ct_cancel, ( NULL, self -> cmd, CS_CANCEL_ALL ) );
                self -> retcode = CALL_RETR ( ct_results, ( self -> cmd, & res_type ) );
            }
            while ( self -> retcode == CS_SUCCEED );

            if ( self -> retcode != CS_END_RESULTS )
                return statusToRC(dbUnknownErr);
        }

        if ( self -> col_info != NULL )
            free ( self -> col_info );

        free ( self );
    }
    return 0;
}

/* SybaseRSNumColumns
 */
static
rc_t SybaseRSNumColumns ( const SybaseRS *self, unsigned int *num_cols )
{
    if ( num_cols == NULL )
        return statusToRC(dbInvalid);

    * num_cols = self -> num_cols;
    return 0;
}

/* SybaseRSColInfo
 */
static
rc_t SybaseRSColInfo ( const SybaseRS *self, unsigned int idx,
    int *col_type, const String **name )
{
    rc_t rc = 0;
    const CS_DATAFMT *fmt;

    if ( col_type == NULL || idx >= self -> num_cols )
        return dbInvalid;

    fmt = & self -> col_info [ idx ];

    switch ( fmt -> datatype )
    {
    case CS_CHAR_TYPE:
    case CS_TEXT_TYPE:
        * col_type = colText;
        break;
    case CS_TINYINT_TYPE:
        * col_type = colU8;
        break;
    case CS_SMALLINT_TYPE:
        * col_type = colI16;
        break;
    case CS_INT_TYPE:
        * col_type = colI32;
        break;
    case CS_REAL_TYPE:
        * col_type = colF32;
        break;
    case CS_FLOAT_TYPE:
        * col_type = colF64;
        break;
    case CS_BIT_TYPE:
        * col_type = colBool;
        break;
    case CS_NUMERIC_TYPE:
    case CS_DECIMAL_TYPE:
        * col_type =  fmt -> scale ? colF64 : colI64;
        break;
    case CS_LONG_TYPE:
        * col_type = colI32;
        break;
    case CS_USHORT_TYPE:
        * col_type = colU16;
        break;
    case CS_UNICHAR_TYPE:
        * col_type = colText;
        break;

    case CS_IMAGE_TYPE:
        * col_type = colBlob;
        break;

    default:
        * col_type = colUnsupported;
    }

    if ( name != NULL )
        rc = GetString ( name, fmt -> name, fmt -> namelen );
    return rc;
}

/* SybaseRSColIdx
 */
static
rc_t SybaseRSColIdx ( const SybaseRS *self, const String *name, unsigned int *idx )
{
    unsigned int i;

    if ( idx == NULL )
        return statusToRC(dbInvalid);

    for ( i = 0; i < self -> num_cols; ++ i )
    {
        const CS_DATAFMT *fmt = & self -> col_info [ i ];
        if ( fmt -> namelen == name -> len )
        {
            if ( ! strcase_cmp ( name -> addr, name -> size,
                       fmt -> name, fmt -> namelen, name -> len ) )
            {
                * idx = i;
                return 0;
            }
        }
    }
    return statusToRC(dbNotFound);
}

/* SybaseRSGetStatus
 */
static
rc_t SybaseRSGetStatus ( const SybaseRS *self, int32_t *status )
{
    if ( status == NULL )
        return statusToRC(dbInvalid);
    if ( ! self -> have_status )
        return statusToRC(dbNullColumn);
    * status = self -> status;
    return 0;
}

/* SybaseRSNextRow
 */
static
int SybaseRSNextRow_ ( SybaseRS *self, SybaseRow **rowp )
{
    CS_INT idx;
    SybaseRow *row;
    size_t col_size;

    if ( rowp == NULL )
        return dbInvalid;

    if ( ! self -> have_rows )
        return dbEndData;

    /* fetch row */
    if ( self -> num_rows == 0 )
    {
        int status = SybaseRSFetch ( self );
        if ( status != dbNoErr )
            return status;
    }

    /* create row */
    col_size = self -> num_cols * sizeof row -> cols [ 0 ];
    row = malloc ( sizeof * row - sizeof row -> cols + col_size );
    if ( row == NULL )
    {
      /*logerr ( errno, "failed to allocate DBRow" );
        LOGERR(klogInt, -1, "failed to allocate DBRow");*/
        return dbMemErr;
    }

    SybaseRowInit ( row, self );

    /* account for the row */
    -- self -> num_rows;

    /* add column data */
    for ( idx = 0; idx < self -> num_cols; ++ idx )
    {
        CS_RETCODE retcode = 0;
        CS_INT ignore;
        CS_IODESC iodesc;
        CS_INT ct_idx = idx + 1;
        
        /* determine column format and maximum size */
        const CS_DATAFMT *fmt = & self -> col_info [ idx ];
        col_size = fmt -> maxlength;
        
        /* for varchar and blob, fetch actual size */
        switch ( fmt -> datatype )
        {
        case CS_TEXT_TYPE:
        case CS_IMAGE_TYPE:
            retcode = CALL_RETR_MSG ( ct_get_data, ( self -> cmd, ct_idx,
               ( char* ) & iodesc, 0, & ignore ), "text or image iodesc" );
            CALL_RETR_MSG ( ct_data_info, ( self -> cmd, CS_GET,
                ct_idx, & iodesc ), "text or image iodesc" );
            col_size = iodesc . total_txtlen;
            break;
        }
        
        /* store the column data size */
        row -> cols [ idx ] . size = col_size;
        
        /* fetch column data */
        if ( col_size > 0 )
        {
            SybaseColumn *col = & row -> cols [ idx ] . data;
            if ( col_size > sizeof * col )
            {
                /* allocate column */
                col -> p = malloc ( col_size );
                if ( col -> p == NULL )
                {
                  /*logerr ( errno, "failed to allocate %u bytes of column data buffer", col_size );
                    PLOGERR(klogInt, (klogInt, -1,
                        "failed to allocate $(n) bytes of column data buffer",
                        "n=%u", col_size));*/
                    SybaseRowWhack ( row );
                    return dbMemErr;
                }
                col = ( SybaseColumn* ) col -> p;
            }
            
            do
            {
                CS_INT buffer_size;
                CS_RETCODE retcode = CALL_RETR_MSG ( ct_get_data, ( self -> cmd,
                    ct_idx, col -> data, ( CS_INT ) col_size, & buffer_size ), "column data" );
                switch ( retcode )
                {
#ifdef CS_BUSY
                case CS_BUSY:
#endif                
                case CS_PENDING:
                    /* this is dubious - although it's like an EINTR
                       return, we should never see it and it's unclear
                       whether async code should do something different */
                    continue;
                    
                case CS_SUCCEED:
                case CS_END_DATA:
                case CS_END_ITEM:
                    break;
                    
                default:
                  /*logmsg ( "ERROR: failed to retrieve column data - %s\n",
                             ctxlat ( retcode ) );
                    PLOGERR(klogInt, (klogInt, -1,
                        "INTERNAL ERROR: failed to retrieve column data - $(t)",
                        "t=%s", ctxlat(retcode)));*/
                    SybaseRowWhack ( row );
                    return dbUnknownErr;
                }
                
                /* the actual length can be less than max length */
                if ( ( size_t ) buffer_size < col_size )
                {
                    /* place a null-byte in case of text */
                    col -> data  [ buffer_size ] = 0;
                    
                    /* update new size */
                    row -> cols [ idx ] . size = buffer_size;
                    
                    /* if new buffer_size is within data size */
                    if ( ( size_t ) buffer_size <= sizeof * col &&
                         col_size > sizeof * col )
                    {
                        /* have to drop allocation */
                        row -> cols [ idx ] . data = * col;
                        free ( col );
                    }
                }
            }
            while ( false );
        }
    }

    * rowp = row;
    return dbNoErr;
}

static
rc_t SybaseRSNextRow ( SybaseRS *self, SybaseRow **rowp )
{   return (statusToRC(SybaseRSNextRow_(self, rowp))); }

/* SybaseRSInit
 */
static DBResultSet_vt sSybaseRS_vt =
{
    ( rc_t ( * ) ( DBResultSet* ) ) SybaseRSWhack,
    ( rc_t ( * ) ( const DBResultSet*, unsigned int *num_cols ) ) SybaseRSNumColumns,
    ( rc_t ( * ) ( const DBResultSet*, unsigned int, int*, const String** ) ) SybaseRSColInfo,
    ( rc_t ( * ) ( const DBResultSet*, const String*, unsigned int* ) ) SybaseRSColIdx,
    ( rc_t ( * ) ( const DBResultSet*, int32_t* ) ) SybaseRSGetStatus,
    ( rc_t ( * ) ( DBResultSet*, DBRow** ) ) SybaseRSNextRow
};

static
void SybaseRSInit ( SybaseRS *rs, CS_COMMAND *cmd )
{
    rs -> par . vt = & sSybaseRS_vt;

    rs -> cmd = cmd;

    rs -> retcode = CS_END_RESULTS;

    rs -> num_rows = rs -> num_cols = 0;
    rs -> col_info = NULL;

    rs -> have_rows = false;
    rs -> have_status = false;
}


/*--------------------------------------------------------------------------
 * SybaseDB
 */

/* SybaseDBConfig
 *  sets several strings within the connection
 *  makes iomode explicit
 *  sets largest packet size
 */
static
CS_RETCODE SybaseDBConfig ( SybaseDB *db, const char *user, const char *pass )
{
    CS_RETCODE retcode;

    long io_mode;
    CS_INT psize;
    char buffer [ 256 ];

    /* originating host */
    gethostname ( buffer, sizeof buffer );
    retcode = CALL_CONN_MSG ( ct_con_props, ( db -> conn, CS_SET,
        CS_HOSTNAME, buffer, CS_NULLTERM, 0 ), "hostname" );
    if ( retcode != CS_SUCCEED )
    {
      /*logmsg ( "ERROR: failed to set connection hostname\n" );
        LOGERR(klogInt, -1, "ERROR: failed to set connection hostname");*/
        return retcode;
    }

    /* username */
    strncpy ( buffer, user, sizeof buffer );
    buffer [ sizeof buffer - 1 ] = 0;
    retcode = CALL_CONN_MSG ( ct_con_props, ( db -> conn, CS_SET,
        CS_USERNAME, ( CS_VOID* ) buffer, CS_NULLTERM, 0 ), "username" );
    if ( retcode != CS_SUCCEED )
    {
      /*logmsg ( "ERROR: failed to set connection username\n" );
        LOGERR(klogInt, -1, "ERROR: failed to set connection username");*/
        return retcode;
    }
    
    /* password */
    strncpy ( buffer, pass, sizeof buffer );
    buffer [ sizeof buffer - 1 ] = 0;
    retcode = CALL_CONN_MSG ( ct_con_props, ( db -> conn, CS_SET,
        CS_PASSWORD, ( CS_VOID* ) buffer, CS_NULLTERM, 0 ), "password" );
    if ( retcode != CS_SUCCEED )
    {
      /*logmsg ( "ERROR: failed to set connection password\n" );
        LOGERR(klogInt, -1, "ERROR: failed to set connection password");*/
        return retcode;
    }
    
    /* set i/o mode */
    io_mode = CS_SYNC_IO;
    retcode = CALL_CONN_MSG ( ct_con_props, ( db -> conn, CS_SET,
        CS_NETIO, & io_mode, CS_UNUSED, 0 ), "netio" );
    if ( retcode != CS_SUCCEED )
    {
      /*logmsg ( "ERROR: failed to set connection netio to sync mode\n" );
        LOGERR(klogInt, -1,
            "ERROR: failed to set connection netio to sync mode");*/
        return retcode;
    }
    
    /* packet size */
    psize = PACKETSIZE;
    retcode = CALL_CONN_MSG ( ct_con_props, ( db -> conn, CS_SET,
        CS_PACKETSIZE, & psize, CS_UNUSED, 0 ), "packetsize" );
    if ( retcode != CS_SUCCEED ) {
      /*logmsg ( "ERROR: failed to set connection packetsize\n" ); 
        LOGERR(klogInt, -1, "ERROR: failed to set connection packetsize");*/
    }
    return retcode;
}

/* SybaseDBWhack
 */
static
rc_t SybaseDBWhack ( SybaseDB *self )
{
    if ( self != NULL )
    {
        /* whack command */
        CS_RETCODE retcode = CALL_CONN ( ct_cmd_drop, ( self -> cmd ) );
        if ( retcode != CS_SUCCEED )
            return statusToRC(dbUnknownErr);

        retcode = CALL_CONN ( ct_close, ( self -> conn, CS_FORCE_CLOSE ) );
        if ( retcode != CS_SUCCEED )
            return statusToRC(dbUnknownErr);

        retcode = CALL_CONN ( ct_con_drop, ( self -> conn ) );
        if ( retcode != CS_SUCCEED )
            return statusToRC(dbUnknownErr);

        free ( self );
    }

    return 0;
}

/* SybaseSetReturnSize
 */
static
rc_t SybaseSetReturnSize ( SybaseDB *self, size_t max_size )
{
    DBResultSet *rs;
    rc_t rc = DatabaseExecute ( & self -> par, & rs, "set textsize %u", max_size );
    if ( rc == 0 )
        DBResultSetWhack ( rs );
    return rc;
}

/* SybaseDBExecute
 */
static
rc_t SybaseDBExecute ( SybaseDB *self, SybaseRS **rsp, char *sql )
{
    rc_t rc = 0;
    SybaseRS *rs;

    if ( rsp == NULL || sql == NULL || sql [ 0 ] == 0 )
        return statusToRC(dbInvalid);

    rs = malloc ( sizeof * rs );
    if ( rs == NULL )
    {
        rc = statusToRC(dbMemErr);
      /*logerr ( errno, "failed to allocate DBResultSet" );
        LOGERR(klogInt, -1, "failed to allocate DBResultSet");*/
    }
    else
    {
        CS_RETCODE retcode;

        SybaseRSInit ( rs, self -> cmd );

        /* set the command string within cmd object */
        retcode = CALL_EXEC ( ct_command, ( rs -> cmd, CS_LANG_CMD,
            sql, CS_NULLTERM, CS_UNUSED ) );
        if ( retcode != CS_SUCCEED )
        {
            rc = statusToRC(dbUnknownErr);
          /*logmsg ( "ERROR: failed to set command '%s'\n", sql );
            PLOGERR(klogInt, (klogInt, -1,
                "ERROR: failed to set command '$(sql)'", "sql=%s", sql));(*/
        }
        else
        {
            /* send the command to the server */
            retcode = CALL_EXEC ( ct_send, ( rs -> cmd ) );
            if ( retcode != CS_SUCCEED )
            {
                rc = statusToRC(dbSendErr);
/*              logmsg ( "ERROR: failed to deliver command '%s'\n", sql );
                PLOGERR(klogInt, (klogInt, -1,
                 "ERROR: failed to deliver command '$(sql)'", "sql=%s", sql));*/
            }
            else
            {
                /* prepare for the outside world */
                rc = statusToRC(SybaseRSPrepare ( rs ));
                if ( rc == 0 )
                {
                    * rsp = rs;
                    return 0;
                }
            }
        }

        SybaseRSWhack ( rs );
    }

    * rsp = NULL;
    return rc;
}

/* SybaseDBInit
 */
Database_vt gSybaseDB_vt =
{
    ( rc_t ( * ) ( Database* ) ) SybaseDBWhack,
    ( rc_t ( * ) ( Database*, size_t ) ) SybaseSetReturnSize,
    ( rc_t ( * ) ( Database*, DBResultSet**, char* ) ) SybaseDBExecute
};

static
void SybaseDBInit ( SybaseDB *db )
{
    db -> par . vt = & gSybaseDB_vt;
    db -> conn = NULL;
    db -> cmd = NULL;
}


/*--------------------------------------------------------------------------
 * Sybase
 */

/* SybaseWhack
 */
static
rc_t SybaseWhack ( Sybase *self )
{
    rc_t rc = DBManagerUnregister ( & self -> par, "sybase" );
    if ( rc == 0 )
    {
        CS_RETCODE retcode = CALL_INIT ( ct_exit, ( self -> context, CS_FORCE_EXIT ) );
        if ( retcode != CS_SUCCEED )
            return statusToRC(dbUnknownErr);

        retcode = CALL_INIT ( cs_ctx_drop, ( self -> context ) );
        if ( retcode != CS_SUCCEED )
            return statusToRC(dbUnknownErr);

        self -> context = NULL;
    }
    return rc;
}

/* SybaseConnect
 */
static
rc_t SybaseConnect ( const Sybase *self, const char *server,
    const char *dbname, const char *user, const char *pass, SybaseDB **dbp )
{
    rc_t rc = 0;
    SybaseDB *db;
    CS_RETCODE retcode;

    if ( server == NULL || server [ 0 ] == 0 ||
         user == NULL || user [ 0 ] == 0 ||
         pass == NULL || pass [ 0 ] == 0 ||
         dbp == NULL )
        return statusToRC(dbInvalid);

    rc = statusToRC(dbMemErr);
    db = malloc ( sizeof * db );
    if ( db != NULL )
    {
        SybaseDBInit ( db );

        /* allocate the connection - not yet connected, just allocated */
        retcode = CALL_CONN ( ct_con_alloc, ( self -> context, & db -> conn ) );
        if ( retcode == CS_SUCCEED )
        {
            /* configure to know about local hostname, etc. */
            retcode = SybaseDBConfig ( db, user, pass );
            if ( retcode != CS_SUCCEED )
                rc = statusToRC(dbConfigErr);
            else
            {
                /* create non-const string for awful Sybase APIs */
                char buffer [ 64 ];
                strncpy ( buffer, server, sizeof buffer );
                buffer [ sizeof buffer - 1 ] = 0;

                /* TA DA - connect */
                retcode = CALL_CONN ( ct_connect, ( db -> conn, buffer, CS_NULLTERM ) );
                if ( retcode != CS_SUCCEED )
                {
                    rc = statusToRC(dbConnectErr);
                  /*logmsg ( "ERROR: failed to establish connection\n" );
                    LOGERR(klogInt, -1, "ERROR: establish connection");*/
                }
                else
                {
                    /* create a command object - the main communications element */
                    retcode = CALL_CONN ( ct_cmd_alloc, ( db -> conn, & db -> cmd ) );
                    if ( retcode != CS_SUCCEED )
                    {
                        rc = statusToRC(dbMemErr);
                      /*logmsg ( "ERROR: failed to allocated command\n" );
                        LOGERR(klogInt, -1, "ERROR: allocated command");*/
                    }
                    else
                    {
                        DBResultSet *rs;

                        /* the Sybase connection is complete */
                        * dbp = db;
                        if ( dbname == NULL || dbname [ 0 ] == 0 )
                            return 0;

                        /* the old idea of connecting to a database
                           rather than a server is translated to a
                           Sybase "use db" command */
                        rc = DatabaseExecute ( & db -> par, & rs, "use %s", dbname );
                        if ( rc == 0 )
                        {
                            DBResultSetWhack ( rs );
                            return 0;
                        }

                        /* whack command */
                        CALL_CONN ( ct_cmd_drop, ( db -> cmd ) );
                    }
                }
            }

            /* whack connection */
            CALL_CONN ( ct_close, ( db -> conn, CS_FORCE_CLOSE ) );
            CALL_CONN ( ct_con_drop, ( db -> conn ) );
        }

        free ( db );
    }

    * dbp = NULL;
    return rc;
}

/* static data
 */
static Sybase sSybase =
{
    {
        ( rc_t ( * ) ( DBManager* ) ) SybaseWhack,
        ( rc_t ( * ) ( const DBManager*, const char*, const char*,
              const char*, const char*, Database** ) ) SybaseConnect
    },
    NULL
};

/* SybaseCSLibMsg
 */
static
CS_INT CS_PUBLIC SybaseCSLibMsg ( CS_CONTEXT *ctx, CS_CLIENTMSG *msg )
{
    ( ( void ) ctx );
    /*logmsg ( "ERROR: CS library error %d - %s", msg -> msgnumber, msg -> msgstring );*/
    PLOGERR(klogInt,
       (klogInt, RC(rcRDBMS, rcNoTarg, rcExecuting, rcError, rcExists),
        "ERROR: CS library error $(n) - $(str)",
        "n=%d,str=%s",  msg->msgnumber, msg->msgstring));
    return CS_SUCCEED;
}

/* SybaseClientMsg
 */
static
CS_RETCODE CS_PUBLIC SybaseClientMsg ( CS_CONTEXT *ctx, CS_CONNECTION *conn, CS_CLIENTMSG *msg )
{
    ( ( void ) ctx );
    /*logmsg ( "ERROR: Open Client error %d - %s", msg -> msgnumber, msg -> msgstring );*/
    PLOGERR(klogInt,
       (klogInt, RC(rcRDBMS, rcNoTarg, rcExecuting, rcError, rcExists),
        "ERROR: Open Client error $(n) - $(str)",
        "n=%d,str=%s",  msg->msgnumber, msg->msgstring));

	if ( msg -> msgnumber == 16908863 )
    {
		CS_INT login_status;
		if ( ct_con_props ( conn, CS_GET, CS_LOGIN_STATUS,
             & login_status, CS_UNUSED, NULL ) != CS_SUCCEED )
        {
			return CS_FAIL;
		}

		if ( login_status )
        {
			CS_RETCODE retcode = ct_cancel ( conn, NULL, CS_CANCEL_ATTN );
			switch ( retcode )
            {
            case CS_SUCCEED:
				return CS_SUCCEED;
#if !defined(SYBDBLIB)
            case CS_TRYING:
           /*   logmsg ( "ERROR: Open Client error %d - timeout on ct_cancel\n", retcode );
                PLOGERR(klogInt, (klogInt, -1,
                    "ERROR: Open Client error $(n) - timeout on ct_cancel",
                    "n=%d",  retcode);*/
                break;
#endif
			}
		}
		return CS_FAIL;
	}

    if ( msg -> severity == CS_SV_INFORM )
        return CS_SUCCEED;
    return CS_FAIL;
}

/* SybaseServerMsg
 */
static
CS_RETCODE CS_PUBLIC SybaseServerMsg ( CS_CONTEXT *ctx, CS_CONNECTION *conn, CS_SERVERMSG *msg)
{
    ( ( void ) ctx );
    ( ( void ) conn );
    switch ( msg -> msgnumber )
    {
    case 3604:
    case 3605:
    case 5701: /* changed database context... */
    case 5703: /* MSSQL: Changed language setting to us_english */

        /* ignore completely */
        break;

    default:
        /* real error */
        if ( msg -> msgnumber <= SRV_MAXERROR )
        {
            /*logmsg ( "ERROR: SQL server %ld, severity %ld, state %ld, "
                     "line %ld, server %s, proc %s - %s",
                     msg -> msgnumber, msg -> severity,
                     msg -> state, msg -> line,
                     msg -> svrnlen ? msg -> svrname : "<no-server>",
                     msg -> proclen ? msg -> proc : "<noproc>",
                     msg -> text );*/
            PLOGERR(klogInt,
               (klogInt, RC(rcRDBMS, rcNoTarg, rcExecuting, rcError, rcExists),
                "ERROR: SQL server $(num), severity $(sev), state $(st), "
                "line $(l), server $(srv), proc $(pr) - $(txt)",
                "num=%ld,sev=%ld,st=%ld,l=%ld,srv=%s,pr=%s,txt=%s",
                msg -> msgnumber, msg -> severity,
                     msg -> state, msg -> line,
                     msg -> svrnlen ? msg -> svrname : "<no-server>",
                     msg -> proclen ? msg -> proc : "<noproc>",
                     msg -> text));
            if ( msg -> severity > 10 )
            {
                /*logmsg ( "ERROR: Sybase connection will be marked dead\n" );*/
                LOGERR(klogInt,
                    RC(rcRDBMS, rcNoTarg, rcExecuting, rcError, rcExists),
                    "ERROR: Sybase connection will be marked dead");
                return CS_FAIL;
            }
            break;
        }
        /* no break */

    case 0:
    case 7412:
    case 7415:
        /* ignore with a message */
        /*logmsg ( "MESSAGE: SQL server %d - %s", msg -> msgnumber, msg -> text );*/
        PLOGERR(klogInt,
           (klogInt, RC(rcRDBMS, rcNoTarg, rcExecuting, rcError, rcExists),
            "MESSAGE: SQL server $(n) - $(txt)",
            "n=%d,txt=%s", msg -> msgnumber, msg -> text));
        break;
    }

    return CS_SUCCEED;
}


/* SybaseInit
 *  initializes the sybase rdbms
 */
rc_t SybaseInit ( int version )
{
    CS_RETCODE retcode;
    rc_t rc = statusToRC(dbUnknownErr);

    if ( sSybase . context != NULL )
        return 0;

    /* initialize a global Sybase context. presumably
       this is a better approach than using cs_ctx_alloc */
    retcode = CALL_INIT ( cs_ctx_global, ( version, & sSybase . context ) );
    if ( retcode == CS_SUCCEED )
    {
        /* install cs callbacks */
        retcode = CALL_INIT ( cs_config, ( sSybase . context, CS_SET,
            CS_MESSAGE_CB, SybaseCSLibMsg, CS_UNUSED, NULL ) );
        if ( retcode != CS_SUCCEED )
            rc = statusToRC(dbConfigErr);
        else
        {
            /* initialize the ct library */
            retcode = CALL_INIT ( ct_init, ( sSybase . context, version ) );
            if ( retcode == CS_SUCCEED )
            {
                /* install the ct callbacks */
                rc = statusToRC(dbConfigErr);
                retcode = CALL_INIT_MSG ( ct_callback, ( sSybase . context, NULL,
                    CS_SET, CS_SERVERMSG_CB, SybaseServerMsg ), "server msg" );
                if ( retcode == CS_SUCCEED )
                {
                    retcode = CALL_INIT_MSG ( ct_callback, ( sSybase . context, NULL,
                        CS_SET, CS_CLIENTMSG_CB, SybaseClientMsg ), "client msg" );
                    if ( retcode == CS_SUCCEED )
                    {
                        /* register with DBManager */
                        rc = DBManagerRegister (  & sSybase . par, "sybase" );
                        if ( rc == 0 )
                            return 0;
                    }
                }
            }
        }

        /* whack the context, such that another
           invocation will not return success */
        CALL_INIT ( cs_ctx_drop, ( sSybase . context ) );
        sSybase . context = NULL;
    }

    return rc;
}
