#include "rdbms-priv.h"
#include <klib/defs.h>
#include <klib/container.h>
#include <rdbms/rdbms.h>

#include <klib/log.h> /* LOGERR */

#include <sysalloc.h>

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>

#if defined(LONG_LONG_MIN)  &&  !defined(LLONG_MIN)
#  define  LLONG_MIN  LONG_LONG_MIN
#  define  LLONG_MAX  LONG_LONG_MAX
#  define ULLONG_MAX ULONG_LONG_MAX
#endif
/*--------------------------------------------------------------------------
 * db error codes
 */

/* logdberr
 *  like logerr, but interprets error code as a db error code
 */
void logdberr ( int err, const char *fmt, ... )
{
    int len;
    char msg [ 256 ];

    va_list args;
    va_start ( args, fmt );
    len = vsnprintf ( msg, sizeof msg, fmt, args );
    va_end ( args );

    if ( len -- < 0 )
        len = sizeof msg - 1;
    if ( msg [ len ] == '\n' )
        msg [ len ] = 0;

    PLOGERR(klogInt,
       (klogInt, RC(rcRDBMS, rcNoTarg, rcExecuting, rcError, rcExists),
        "ERROR: $(msg) - $(err)\n",
        "msg=%s,err=%s", msg, strdberror(err)));
}

rc_t statusToRC(int status)
{
	switch (status) {
		case dbNoErr     : return 0; break;
		case dbUnknownErr:
            return RC(rcRDBMS, rcFunction, rcExecuting, rcError, rcUnknown);
            break;
		case dbInvalid:
            return
                RC(rcRDBMS, rcFunction, rcExecuting, rcFunctParam, rcInvalid);
            break;
		case dbMemErr:
            return RC(rcRDBMS, rcStorage, rcAllocating, rcMemory, rcExhausted);
            break;
		case dbConfigErr:
            return RC(rcRDBMS, rcKFG, rcInitializing, rcKFG, rcFailed);
            break;
		case dbConnectErr:
            return RC(rcRDBMS, rcDatabase, rcOpening, rcConnection, rcFailed);
            break;
		case dbSendErr:
            return RC(rcRDBMS, rcCmd, rcSending, rcCmd, rcFailed);
            break;
		case dbFetchErr:
            return RC(rcRDBMS, rcData, rcRetrieving, rcError, rcUnknown);
            break;
		case dbEndData:
            return RC(rcRDBMS, rcData, rcRetrieving, rcData, rcNotAvailable);
            break;
		case dbNullColumn:
            return RC(rcRDBMS, rcFunction, rcExecuting, rcColumn, rcNull);
            break;
		case dbNotFound:
            return RC(rcRDBMS, rcFunction, rcExecuting, rcItem, rcNotFound);
            break;
		case dbRangeErr:
            return RC(rcRDBMS, rcFunction, rcExecuting, rcParam, rcOutofrange);
            break;
		case dbBuffSpErr:
            return RC(rcRDBMS, rcQuery, rcExecuting, rcBuffer, rcInsufficient);
            break;
		case dbExists:
            return RC(rcRDBMS, rcMgr, rcRegistering, rcName, rcExists);
            break;
        default:
    	    return RC(rcRDBMS, rcNoTarg, rcProcessing, rcNoObj, rcUnknown);
            break;
	}
}

/* strdberror
 *  a version of strerror that operates within db error code space
 *  returned string is owned by library
 */
const char *strdberror ( int dberr )
{
    switch ( dberr )
    {
    case dbNoErr:
        return "Success";
    case dbUnknownErr:
        return "Unknown error";
    case dbInvalid:
        return "Invalid parameter";
    case dbMemErr:
        return "Out of memory";
    case dbConfigErr:
        return "Database connection configuration failure";
    case dbConnectErr:
        return "Database connection could not be established";
/*  case dbCmdErr:
        return "The server command was not accepted";*/
    case dbSendErr:
        return "The server command could not be delivered";
    case dbFetchErr:
        return "Data could not be retrieved from server";
    case dbEndData:
        return "No more data are available";
    case dbNullColumn:
        return "Null column";
    case dbNotFound:
        return "Not found";
    case dbRangeErr:
        return "Parameter out of range";
    case dbBuffSpErr:
        return "Query exceeds buffer space limits";
    case dbExists:
        return "Name already registered";
    }
    return "Invalid error code";
}

/* strdberror_r
 *  a version of strerror_r that operates within db error code space
 *  returns 0 on success, EINVAL if "dberr" is invalid, and
 *  ERANGE if insufficient space was provided to copy the data.
 */
int strdberror_r ( int dberr, char *buf, size_t n )
{
    size_t copied;

    if ( dberr < 0 || dberr >= dbNumErrCodes )
        return EINVAL;

    copied = string_copy_measure ( buf, n, strdberror ( dberr ) );
    if ( copied == n )
        return ERANGE;
    return 0;
}


/*--------------------------------------------------------------------------
 * DBRow
 */

/* DBRowGetAsNum
 */
#define DBRowGetAsNum( row, idx, i, d, is_floating ) \
    ( * ( row ) -> vt -> getAsNum ) ( row, idx, i, d, is_floating )

rc_t DBRowGetAsBool ( const DBRow *self, unsigned int idx, bool *value )
{
    int64_t i; double d; bool is_floating;
    rc_t rc = DBRowGetAsNum ( self, idx, & i, & d, & is_floating );
    if ( rc == 0 )
    {
        if ( is_floating )
            * value = d != 0.0;
        else
            * value = i != 0;
    }
    return rc;
}

rc_t DBRowGetAsI8 ( const DBRow *self, unsigned int idx, int8_t *value )
{
    int64_t i; double d; bool is_floating;
    rc_t rc = DBRowGetAsNum ( self, idx, & i, & d, & is_floating );
    if ( rc == 0 )
    {
        if ( is_floating )
        {
            if ( d < ( double ) SCHAR_MIN || d > ( double ) SCHAR_MAX )
                return statusToRC(dbRangeErr);
            * value = ( int8_t ) d;
        }
        else
        {
            if ( i < SCHAR_MIN || i > SCHAR_MAX )
                return statusToRC(dbRangeErr);
            * value = ( int8_t ) i;
        }
    }
    return rc;
}

rc_t DBRowGetAsU8 ( const DBRow *self, unsigned int idx, uint8_t *value )
{
    int64_t i; double d; bool is_floating;
    rc_t rc = DBRowGetAsNum ( self, idx, & i, & d, & is_floating );
    if ( rc == 0 )
    {
        if ( is_floating )
        {
            if ( d < 0.0 || d > ( double ) UCHAR_MAX )
                return statusToRC(dbRangeErr);
            * value = ( uint8_t ) d;
        }
        else
        {
            if ( i < 0 || i > UCHAR_MAX )
                return statusToRC(dbRangeErr);
            * value = ( uint8_t ) i;
        }
    }
    return rc;
}

rc_t DBRowGetAsI16 ( const DBRow *self, unsigned int idx, int16_t *value )
{
    int64_t i; double d; bool is_floating;
    rc_t rc = DBRowGetAsNum ( self, idx, & i, & d, & is_floating );
    if ( rc == 0 )
    {
        if ( is_floating )
        {
            if ( d < ( double ) SHRT_MIN || d > ( double ) SHRT_MAX )
                return statusToRC(dbRangeErr);
            * value = ( int16_t ) d;
        }
        else
        {
            if ( i < SHRT_MIN || i > SHRT_MAX )
                return statusToRC(dbRangeErr);
            * value = ( int16_t ) i;
        }
    }
    return rc;
}

rc_t DBRowGetAsU16 ( const DBRow *self, unsigned int idx, uint16_t *value )
{
    int64_t i; double d; bool is_floating;
    rc_t rc = DBRowGetAsNum ( self, idx, & i, & d, & is_floating );
    if ( rc == 0 )
    {
        if ( is_floating )
        {
            if ( d < 0.0 || d > ( double ) USHRT_MAX )
                return statusToRC(dbRangeErr);
            * value = ( uint16_t ) d;
        }
        else
        {
            if ( i < 0 || i > USHRT_MAX )
                return statusToRC(dbRangeErr);
            * value = ( uint16_t ) i;
        }
    }
    return rc;
}

rc_t DBRowGetAsI32 ( const DBRow *self, unsigned int idx, int32_t *value )
{
    int64_t i; double d; bool is_floating;
    rc_t rc = DBRowGetAsNum ( self, idx, & i, & d, & is_floating );
    if ( rc == 0 )
    {
        if ( is_floating )
        {
            if ( d < ( double ) INT_MIN || d > ( double ) INT_MAX )
                return statusToRC(dbRangeErr);
            * value = ( int32_t ) d;
        }
        else
        {
            if ( i < INT_MIN || i > INT_MAX )
                return statusToRC(dbRangeErr);
            * value = ( int32_t ) i;
        }
    }
    return rc;
}

rc_t DBRowGetAsU32 ( const DBRow *self, unsigned int idx, uint32_t *value )
{
    int64_t i; double d; bool is_floating;
    rc_t rc = DBRowGetAsNum ( self, idx, & i, & d, & is_floating );
    if ( rc == 0 )
    {
        if ( is_floating )
        {
            if ( d < 0.0 || d > ( double ) UINT_MAX )
                return statusToRC(dbRangeErr);
            * value = ( uint32_t ) d;
        }
        else
        {
            if ( i < 0 || i > UINT_MAX )
                return statusToRC(dbRangeErr);
            * value = ( uint32_t ) i;
        }
    }
    return rc;
}

rc_t DBRowGetAsI64 ( const DBRow *self, unsigned int idx, int64_t *value )
{
    double d; bool is_floating;
    rc_t rc = DBRowGetAsNum ( self, idx, value, & d, & is_floating );
    if ( rc == 0 )
    {
        if ( is_floating )
        {
            if ( d < ( double ) LLONG_MIN || d > ( double ) LLONG_MAX )
                return statusToRC(dbRangeErr);
            * value = ( int64_t ) d;
        }
    }
    return rc;
}

rc_t DBRowGetAsU64 ( const DBRow *self, unsigned int idx, uint64_t *value )
{
    int64_t i; double d; bool is_floating;
    rc_t rc = DBRowGetAsNum ( self, idx, & i, & d, & is_floating );
    if ( rc == 0 )
    {
        if ( is_floating )
        {
            if ( d < 0.0 || d > ( double ) ULLONG_MAX )
                return statusToRC(dbRangeErr);
            * value = ( uint64_t ) d;
        }
        else
        {
            if ( i < 0 )
                return statusToRC(dbRangeErr);
            * value = ( uint64_t ) i;
        }
    }
    return rc;
}

rc_t DBRowGetAsF32 ( const DBRow *self, unsigned int idx, float *value )
{
    int64_t i; double d; bool is_floating;
    rc_t rc = DBRowGetAsNum ( self, idx, & i, & d, & is_floating );
    if ( rc == 0 )
    {
        if ( is_floating )
        {
            * value = ( float ) d;
        }
        else
        {
            * value = ( float ) i;
        }
    }
    return rc;
}

rc_t DBRowGetAsF64 ( const DBRow *self, unsigned int idx, double *value )
{
    int64_t i; bool is_floating;
    rc_t rc = DBRowGetAsNum ( self, idx, & i, value, & is_floating );
    if ( rc == 0 )
    {
        if ( ! is_floating )
        {
            * value = ( double ) i;
        }
    }
    return rc;
}

#undef DBRowGetAsString

static
rc_t DBRowGetAsString ( const DBRow *self, unsigned int idx, const String **value )
{
    return ( * self -> vt -> getAsString ) ( self, idx, value );
}


/*--------------------------------------------------------------------------
 * DBResultSet
 */

/* DBResultSetForEach
 *  iterates across all rows
 */
rc_t DBResultSetForEach ( DBResultSet *rs,
    void ( * f ) ( DBRow *row, void *data ), void * data )
{
    if ( rs == NULL || f == NULL )
        return statusToRC(dbInvalid);

    while ( 1 )
    {
        DBRow *row;
        rc_t rc = DBResultSetNextRow ( rs, & row );
        if ( rc != 0 )
        {
            if ( rc == RC(rcRDBMS,
                    rcData, rcRetrieving, rcData, rcNotAvailable) )
                break;
            return rc;
        }

        ( * f ) ( row, data );

        DBRowWhack ( row );
    }

    return 0;
}

/* DBResultSetDoUntil
 *  iterates across all rows
 *  until the function returns true
 */
rc_t DBResultSetDoUntil ( DBResultSet *rs,
    bool ( * f ) ( DBRow *row, void *data ), void * data )
{
    if ( rs == NULL || f == NULL )
        return statusToRC(dbInvalid);

    while ( 1 )
    {
        bool done;

        DBRow *row;
        rc_t rc = DBResultSetNextRow ( rs, & row );
        if ( rc != 0 )
        {
            if ( rc == RC(rcRDBMS,
                    rcData, rcRetrieving, rcData, rcNotAvailable) )
                break;
            return rc;
        }

        done = ( * f ) ( row, data );

        DBRowWhack ( row );

        if ( done )
            break;
    }

    return 0;
}

/*--------------------------------------------------------------------------
 * Database
 */

/* DatabaseExecute
 *  issue an sql request
 */
static
rc_t DatabaseExecuteV ( Database *self, DBResultSet **rs,
    const char *fmt, va_list args )
{
    rc_t rc = 0;
    char sql [ 1024 ];
    int status = vsnprintf ( sql, sizeof sql, fmt, args );
    if ( status >= 0 )
        rc = ( * self -> vt -> execute ) ( self, rs, sql );
    else
    {
        char *mem = malloc ( 32 * 1024 );
        if ( mem == NULL )
            return statusToRC(dbMemErr);
        status = vsnprintf ( mem, 32 * 1024, fmt, args );
        if ( status < 0 )
        {
            free ( mem );
            mem = malloc ( 128 * 1024 );
            if ( mem == NULL )
                return statusToRC(dbMemErr);
            status = vsnprintf ( mem, 128 * 1024, fmt, args );
            if ( status < 0 )
            {
                free ( mem );
                return statusToRC(dbBuffSpErr);
            }
        }
        rc = ( * self -> vt -> execute ) ( self, rs, mem );
        free ( mem );
    }
    return rc;
}

rc_t DatabaseExecute ( Database *self, DBResultSet **rs, const char *fmt, ... )
{
    rc_t rc = 0;
    va_list args;

    va_start ( args, fmt );
    rc = DatabaseExecuteV ( self, rs, fmt, args );
    va_end ( args );

    return rc;
}


/* DatabaseGetStatus
 *  executes a stored procedure and returns status code
 */
rc_t DatabaseGetStatus ( Database *self, int32_t *val, const char *fmt, ... )
{
    rc_t rc = 0;
    va_list args;
    DBResultSet *rs;

    va_start ( args, fmt );
    rc = DatabaseExecuteV ( self, & rs, fmt, args );
    va_end ( args );

    if ( rc == 0 )
    {
        rc = DBResultSetGetStatus ( rs, val );
        DBResultSetWhack ( rs );
    }

    return rc;
}


/* DatabaseGetColumn
 *  the basic guy
 */
#define CF( func ) \
    ( rc_t ( * ) ( const DBRow*, unsigned int, void* ) ) ( func )

static
rc_t DatabaseGetColumn ( Database *self, unsigned int idx, void *value,
    rc_t ( * get_as ) ( const DBRow*, unsigned int, void* ),
    const char *fmt, va_list args )
{
    DBResultSet *rs;
    rc_t rc = DatabaseExecuteV ( self, & rs, fmt, args );
    if ( rc == 0 )
    {
        DBRow *row;
        rc = DBResultSetNextRow ( rs, & row );
        if ( rc == 0 )
        {
            rc = ( * get_as ) ( row, idx, value );
            DBRowWhack ( row );

            while ( rc == 0 )
            {
                rc = DBResultSetNextRow ( rs, & row );
                if ( rc == 0 )
                    DBRowWhack ( row );
            }
            if ( rc == RC(rcRDBMS,
                    rcData, rcRetrieving, rcData, rcNotAvailable) )
                rc = 0;
        }
        else if ( rc == RC(rcRDBMS,
                    rcData, rcRetrieving, rcData, rcNotAvailable) )
            rc = statusToRC(dbNotFound);

        DBResultSetWhack ( rs );
    }
    return rc;
}

/* DatabaseGetAs...
 *  column access functions on first row of a query
 *  all indices are zero-based
 */
rc_t DatabaseGetAsBool ( Database *self, unsigned int idx,
    bool *value, const char *sql, ... )
{
    rc_t rc = 0;
    va_list args;

    va_start ( args, sql );
    rc = DatabaseGetColumn ( self, idx, value, CF ( DBRowGetAsBool ), sql, args );
    va_end ( args );

    return rc;
}

rc_t DatabaseGetAsI8 ( Database *self, unsigned int idx,
    int8_t *value, const char *sql, ... )
{
    rc_t rc = 0;
    va_list args;

    va_start ( args, sql );
    rc = DatabaseGetColumn ( self, idx, value, CF ( DBRowGetAsI8 ), sql, args );
    va_end ( args );

    return rc;
}

rc_t DatabaseGetAsU8 ( Database *self, unsigned int idx,
    uint8_t *value, const char *sql, ... )
{
    rc_t rc = 0;
    va_list args;

    va_start ( args, sql );
    rc = DatabaseGetColumn ( self, idx, value, CF ( DBRowGetAsU8 ), sql, args );
    va_end ( args );

    return rc;
}

rc_t DatabaseGetAsI16 ( Database *self, unsigned int idx,
    int16_t *value, const char *sql, ... )
{
    rc_t rc = 0;
    va_list args;

    va_start ( args, sql );
    rc = DatabaseGetColumn ( self, idx, value, CF ( DBRowGetAsI16 ), sql, args );
    va_end ( args );

    return rc;
}

rc_t DatabaseGetAsU16 ( Database *self, unsigned int idx,
    uint16_t *value, const char *sql, ... )
{
    rc_t rc = 0;
    va_list args;

    va_start ( args, sql );
    rc = DatabaseGetColumn ( self, idx, value, CF ( DBRowGetAsU16 ), sql, args );
    va_end ( args );

    return rc;
}

rc_t DatabaseGetAsI32 ( Database *self, unsigned int idx,
    int32_t *value, const char *sql, ... )
{
    rc_t rc = 0;
    va_list args;

    va_start ( args, sql );
    rc = DatabaseGetColumn ( self, idx, value, CF ( DBRowGetAsI32 ), sql, args );
    va_end ( args );

    return rc;
}

rc_t DatabaseGetAsU32 ( Database *self, unsigned int idx,
    uint32_t *value, const char *sql, ... )
{
    rc_t rc = 0;
    va_list args;

    va_start ( args, sql );
    rc = DatabaseGetColumn ( self, idx, value, CF ( DBRowGetAsU32 ), sql, args );
    va_end ( args );

    return rc;
}

rc_t DatabaseGetAsI64 ( Database *self, unsigned int idx,
    int64_t *value, const char *sql, ... )
{
    rc_t rc = 0;
    va_list args;

    va_start ( args, sql );
    rc = DatabaseGetColumn ( self, idx, value, CF ( DBRowGetAsI64 ), sql, args );
    va_end ( args );

    return rc;
}

rc_t DatabaseGetAsU64 ( Database *self, unsigned int idx,
    uint64_t *value, const char *sql, ... )
{
    rc_t rc = 0;
    va_list args;

    va_start ( args, sql );
    rc = DatabaseGetColumn ( self, idx, value, CF ( DBRowGetAsU64 ), sql, args );
    va_end ( args );

    return rc;
}

rc_t DatabaseGetAsF32 ( Database *self, unsigned int idx,
    float *value, const char *sql, ... )
{
    rc_t rc = 0;
    va_list args;

    va_start ( args, sql );
    rc = DatabaseGetColumn ( self, idx, value, CF ( DBRowGetAsF32 ), sql, args );
    va_end ( args );

    return rc;
}

rc_t DatabaseGetAsF64 ( Database *self, unsigned int idx,
    double *value, const char *sql, ... )
{
    rc_t rc = 0;
    va_list args;

    va_start ( args, sql );
    rc = DatabaseGetColumn ( self, idx, value, CF ( DBRowGetAsF64 ), sql, args );
    va_end ( args );

    return rc;
}

rc_t DatabaseGetAsString ( Database *self, unsigned int idx,
    const String **value, const char *sql, ... )
{
    rc_t rc = 0;
    va_list args;

    va_start ( args, sql );
    rc = DatabaseGetColumn ( self, idx, value, CF ( DBRowGetAsString ), sql, args );
    va_end ( args );

    return rc;
}


/*--------------------------------------------------------------------------
 * DBManager
 *  something such as Sybase, ORACLE, etc.
 */
typedef struct DBManagerBucket DBManagerBucket;
struct DBManagerBucket
{
    BSTNode n;
    const String *name;
    const DBManager *mgr;
};

static BSTree sDBMgrs;

/* DBManagerBucketCmp
 *  for ordering within tree
 */
static
int DBManagerBucketCmp ( const BSTNode *left, const BSTNode *right )
{
#define lh ( ( const DBManagerBucket* ) left )
#define rh ( ( const DBManagerBucket* ) right )

    return StringCompare ( lh -> name, rh -> name );

#undef lh
#undef rh
}

/* DBManagerBucketFind
 *  for locating a bucket
 */
static
int DBManagerBucketFind ( const void *item, const BSTNode *n )
{
#define lh ( ( const String* ) item )
#define rh ( ( const DBManagerBucket* ) n )

    return StringCompare ( lh, rh -> name );

#undef lh
#undef rh
}

/* DBManagerInit
 *  initialize object reference
 *  returns 0 if manager has been registered
 */
static int DBManagerInit_ ( const DBManager **mgr, const char *name )
{
    String str;
    const DBManagerBucket *b;

    if ( mgr == NULL || name == NULL )
        return dbInvalid;

    /* create a String from name */
    StringInitCString ( & str, name );

    /* see if it's been registered */
    b = ( const DBManagerBucket* ) BSTreeFind ( & sDBMgrs, & str, DBManagerBucketFind );
    if ( b == NULL )
        return dbNotFound;

    /* reach in and grab it */
    * mgr = b -> mgr;
    return 0;
}

rc_t DBManagerInit ( const DBManager **mgr, const char *name )
{   return statusToRC(DBManagerInit_(mgr, name)); }

/* DBManagerRegister
 *  makes an object known to DBManager
 */
rc_t DBManagerRegister ( const DBManager *mgr, const char *name )
{
    String str;
    DBManagerBucket *b;

    if ( mgr == NULL || name == NULL )
        return statusToRC(dbInvalid);

    /* create a String from name */
    StringInitCString ( & str, name );

    /* see if it's been registered */
    b = ( DBManagerBucket* ) BSTreeFind ( & sDBMgrs, & str, DBManagerBucketFind );
    if ( b != NULL )
        return statusToRC(dbExists);

    /* create a new one */
    b = ( DBManagerBucket* ) malloc ( sizeof *b );
    if ( b != NULL )
    {
        rc_t rc = StringCopy ( & b -> name, & str );
        if ( rc == 0 )
        {
            b -> mgr = mgr;
            BSTreeInsert ( & sDBMgrs, & b -> n, DBManagerBucketCmp );
            return 0;
        }
        free ( b );
        return rc;
    }
    return statusToRC(dbMemErr);
}


/* DBManagerUnregister
 *  makes an object unknown to DBManager
 */
rc_t DBManagerUnregister ( const DBManager *mgr, const char *name )
{
    String str;
    DBManagerBucket *b;

    if ( mgr == NULL || name == NULL )
        return statusToRC(dbInvalid);

    /* create a String from name */
    StringInitCString ( & str, name );

    /* see if it's been registered */
    b = ( DBManagerBucket* ) BSTreeFind ( & sDBMgrs, & str, DBManagerBucketFind );
    if ( b == NULL )
        return statusToRC(dbNotFound);

    /* remove node */
    BSTreeUnlink ( & sDBMgrs, & b -> n );

    /* whack node */
    StringWhack ( ( String* ) b -> name );
    free ( b );

    return 0;
}

rc_t DBManagerRelease(const DBManager *mgr)
{   return DBManagerWhack((DBManager*)mgr); }
