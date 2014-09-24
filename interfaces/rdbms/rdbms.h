#ifndef _h_rdbms_rdbms_
#define _h_rdbms_rdbms_

#ifndef _h_klib_text_
#include <klib/text.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* logdberr
 *  like logerr, but interprets error code as a db error code
 */
void logdberr ( int err, const char *fmt, ... );

/* strdberror
 *  a version of strerror that operates within db error code space
 *  returned string is owned by library
 */
const char *strdberror ( int dberr );

/* strdberror_r
 *  a version of strerror_r that operates within db error code space
 *  returns 0 on success, EINVAL if "dberr" is invalid, and
 *  ERANGE if insufficient space was provided to copy the data.
 */
int strdberror_r ( int dberr, char *buf, size_t n );

/*--------------------------------------------------------------------------
 * DBColumnType
 *  for discovering column type at runtime
 */
enum
{
    colUnsupported,
    colBool,
    colI8,
    colU8,
    colI16,
    colU16,
    colI32,
    colU32,
    colI64,
    colU64,
    colF32,
    colF64,
    colText,
    colBlob
};


/*--------------------------------------------------------------------------
 * DBRow
 */
typedef struct DBBlob DBBlob;
struct DBBlob
{
    /* the blob is in here */
    void *addr;
    size_t size;

    /* when finished with the blob, whack it thus */
    void *priv;
    void ( * whack ) ( void *priv );
};

/* DBBlobWhack
 */
#define DBBlobWhack( blob ) \
    ( ( blob ) -> priv != NULL ? \
      ( * ( blob ) -> whack ) ( ( blob ) -> priv ) : ( void ) 0 )

typedef struct DBRow DBRow;
typedef struct DBRow_vt DBRow_vt;
struct DBRow_vt
{
    rc_t ( * whack ) ( DBRow *self );
    rc_t ( * numColumns ) ( const DBRow *self, unsigned int *num_cols );
    rc_t ( * colInfo ) ( const DBRow *self, unsigned int idx,
        int *col_type, const String **name );
    rc_t ( * colIdx ) ( const DBRow *self, const String *name,
        unsigned int *idx );
    bool ( * isNull ) ( const DBRow *self, unsigned int idx );

    rc_t ( * getAsNum ) ( const DBRow *self, unsigned int idx, int64_t *i, double *d, bool *is_floating );
    rc_t ( * getAsString ) ( const DBRow *self, unsigned int idx,
        const String **value );
    rc_t ( * getAsBlob ) ( const DBRow *self, unsigned int idx,
        const void **value, size_t *bytes );
    rc_t ( * stealBlob ) ( DBRow *self, unsigned int idx, DBBlob *blob );
};
struct DBRow
{
    const DBRow_vt *vt;
};

/* DBRowNumColumns
 *  read the number of columns in row
 */
#define DBRowNumColumns( row, num_cols ) \
    ( * ( row ) -> vt -> numColumns ) ( row, num_cols )

/* DBRowColumnInfo
 *  read the type for indexed column as DBColumnType
 *  index is zero-based
 *  optionally reads column name if "name" is not NULL
 */
#define DBRowColumnInfo( row, idx, col_type, name ) \
    ( * ( row ) -> vt -> colInfo ) ( row, idx, col_type, name )

/* DBRowColumnIdx
 *  determine zero-based index for named column
 */
#define DBRowColumnIdx( row, name, idx ) \
    ( * ( row ) -> vt -> colIdx ) ( row, name, idx )

/* DBRowIsNull
 *  determine whether an indexed column value is NULL
 *  index is zero-based
 *  returns true only if row and column are valid,
 *  and column is NULL, obviously
 */
#define DBRowIsNull( row, idx ) \
    ( * ( row ) -> vt -> isNull ) ( row, idx )

/* DBRowGetAs...
 *  column access functions
 *  all indices are zero-based
 */
rc_t DBRowGetAsBool ( const DBRow *row, unsigned int idx, bool *value );
rc_t DBRowGetAsI8 ( const DBRow *row, unsigned int idx, int8_t *value );
rc_t DBRowGetAsU8 ( const DBRow *row, unsigned int idx, uint8_t *value );
rc_t DBRowGetAsI16 ( const DBRow *row, unsigned int idx, int16_t *value );
rc_t DBRowGetAsU16 ( const DBRow *row, unsigned int idx, uint16_t *value );
rc_t DBRowGetAsI32 ( const DBRow *row, unsigned int idx, int32_t *value );
rc_t DBRowGetAsU32 ( const DBRow *row, unsigned int idx, uint32_t *value );
rc_t DBRowGetAsI64 ( const DBRow *row, unsigned int idx, int64_t *value );
rc_t DBRowGetAsU64 ( const DBRow *row, unsigned int idx, uint64_t *value );
rc_t DBRowGetAsF32 ( const DBRow *row, unsigned int idx, float *value );
rc_t DBRowGetAsF64 ( const DBRow *row, unsigned int idx, double *value );
#define DBRowGetAsString( row, idx, value ) \
    ( * ( row ) -> vt -> getAsString ) ( row, idx, value )

/* DBRowGetAsBlob
 *  special access that returns direct pointer to internal data
 *  only guaranteed to live while row exists
 *  ( here's where reference counting saves the day... )
 */
#define DBRowGetAsBlob( row, idx, value, bytes ) \
    ( * ( row ) -> vt -> getAsBlob ) ( row, idx, value, bytes )

/* DBRowStealBlob
 *  even more special access that steals internal data from row
 *  or at least returns an allocation that is no longer associated
 *  with the column.
 *
 *  intended to invalidate the column and cause it to be NULL,
 *  but this is implementation dependent.
 */
#define DBRowStealBlob( row, idx, blob ) \
    ( * ( row ) -> vt -> stealBlob ) ( row, idx, blob )

/* DBRowWhack
 *  whacks the row
 */
#define DBRowWhack( row ) \
    ( ( row ) == NULL ? 0 : ( * ( row ) -> vt -> whack ) ( row ) )


/*--------------------------------------------------------------------------
 * DBResultSet
 */
typedef struct DBResultSet DBResultSet;
typedef struct DBResultSet_vt DBResultSet_vt;
struct DBResultSet_vt
{
    rc_t ( * whack ) ( DBResultSet *self );
    rc_t ( * numColumns ) ( const DBResultSet *self, unsigned int *num_cols );
    rc_t ( * colInfo ) ( const DBResultSet *self, unsigned int idx,
        int *col_type, const String **name );
    rc_t ( * colIdx ) ( const DBResultSet *self, const String *name,
        unsigned int *idx );

    rc_t ( * getStatus ) ( const DBResultSet *self, int32_t *status );
    rc_t ( * nextRow ) ( DBResultSet *self, DBRow **row );
};
struct DBResultSet
{
    const DBResultSet_vt *vt;
};

/* DBResultSetNumColumns
 *  read the number of columns in result
 */
#define DBResultSetNumColumns( rs, num_cols ) \
    ( * ( rs ) -> vt -> numColumns ) ( rs, num_cols )

/* DBResultSetColumnInfo
 *  read the type for indexed column as DBColumnType
 *  index is zero-based
 *  optionally reads column name if "name" is not NULL
 */
#define DBResultSetColumnInfo( rs, idx, col_type, name ) \
    ( * ( rs ) -> vt -> colInfo ) ( rs, idx, col_type, name )

/* DBResultSetColumnIdx
 *  determine zero-based index for named column
 */
#define DBResultSetColumnIdx( rs, name, idx ) \
    ( * ( rs ) -> vt -> colIdx ) ( rs, name, idx )

/* DBResultSetGetStatus
 */
#define DBResultSetGetStatus( rs, status ) \
    ( * ( rs ) -> vt -> getStatus ) ( rs, status )

/* DBResultSetNextRow
 *  returns dbNoErr if row was created
 *  returns dbEndData if no more rows were available
 *  returns something more sinister if an error occurred
 */
#define DBResultSetNextRow( rs, row ) \
    ( * ( rs ) -> vt -> nextRow ) ( rs, row )

/* DBResultSetForEach
 *  iterates across all rows
 */
rc_t DBResultSetForEach ( DBResultSet *rs,
    void ( * f ) ( DBRow *row, void *data ), void * data );

/* DBResultSetDoUntil
 *  iterates across all rows
 *  until the function returns true
 */
rc_t DBResultSetDoUntil ( DBResultSet *rs,
    bool ( * f ) ( DBRow *row, void *data ), void * data );

/* DBResultSetWhack
 *  whacks the result set
 */
#define DBResultSetWhack( rs ) \
    ( ( rs ) == NULL ? 0 : ( * ( rs ) -> vt -> whack ) ( rs ) )


/*--------------------------------------------------------------------------
 * Database
 */
typedef struct Database Database;
typedef struct Database_vt Database_vt;
struct Database_vt
{
    rc_t ( * whack ) ( Database *self );
    rc_t ( * setReturnSize ) ( Database *self, size_t max_size );
    rc_t ( * execute ) ( Database *self, DBResultSet **rs, char *sql );
};
struct Database
{
    const Database_vt *vt;
};

/* DatabaseSetReturnSize
 *  sets maximum size for returns
 */
#define DatabaseSetReturnSize( db, max_size ) \
    ( * ( db ) -> vt -> setReturnSize ) ( db, max_size )

/* DatabaseExecute
 *  issue an sql request
 */
rc_t DatabaseExecute ( Database *db, DBResultSet **rs, const char *sql, ... );

/* DatabaseGetStatus
 *  executes a stored procedure and returns status code
 */
rc_t DatabaseGetStatus ( Database *db, int32_t *status, const char *sql, ... );

/* DatabaseGetAs...
 *  column access functions on first row of a query
 *  all indices are zero-based
 */
rc_t DatabaseGetAsBool ( Database *db, unsigned int idx,
    bool *value, const char *sql, ... );
rc_t DatabaseGetAsI8 ( Database *db, unsigned int idx,
    int8_t *value, const char *sql, ... );
rc_t DatabaseGetAsU8 ( Database *db, unsigned int idx,
    uint8_t *value, const char *sql, ... );
rc_t DatabaseGetAsI16 ( Database *db, unsigned int idx,
    int16_t *value, const char *sql, ... );
rc_t DatabaseGetAsU16 ( Database *db, unsigned int idx,
    uint16_t *value, const char *sql, ... );
rc_t DatabaseGetAsI32 ( Database *db, unsigned int idx,
    int32_t *value, const char *sql, ... );
rc_t DatabaseGetAsU32 ( Database *db, unsigned int idx,
    uint32_t *value, const char *sql, ... );
rc_t DatabaseGetAsI64 ( Database *db, unsigned int idx,
    int64_t *value, const char *sql, ... );
rc_t DatabaseGetAsU64 ( Database *db, unsigned int idx,
    uint64_t *value, const char *sql, ... );
rc_t DatabaseGetAsF32 ( Database *db, unsigned int idx,
    float *value, const char *sql, ... );
rc_t DatabaseGetAsF64 ( Database *db, unsigned int idx,
    double *value, const char *sql, ... );
rc_t DatabaseGetAsString ( Database *db, unsigned int idx,
    const String **value, const char *sql, ... );

/* DatabaseWhack
 *  tear down a connection
 */
#define DatabaseWhack( db ) \
    ( ( db ) == NULL ? 0 : ( * ( db ) -> vt -> whack ) ( db ) )


/*--------------------------------------------------------------------------
 * DBManager
 *  something such as Sybase, ORACLE, etc.
 */
typedef struct DBManager DBManager;
struct DBManager
{
    rc_t ( * whack ) ( DBManager *self );
    rc_t ( * connect ) ( const DBManager *self, const char *server,
        const char *dbname, const char *user, const char *pass, Database **db );
};

/* DBManagerInit
 *  initialize object reference
 *  returns 0 if manager has been registered
 */
rc_t DBManagerInit ( const DBManager **mgr, const char *name );


/* DBManagerRegister
 *  makes an object known to DBManager
 */
rc_t DBManagerRegister ( const DBManager *mgr, const char *name );


/* DBManagerUnregister
 *  makes an object unknown to DBManager
 */
rc_t DBManagerUnregister ( const DBManager *mgr, const char *name );


/* DBManagerConnect
 *  establish a connection
 */
#define DBManagerConnect( mgr, server, dbname, user, pass, db ) \
    ( * ( mgr ) -> connect ) ( mgr, server, dbname, user, pass, db )

/* DBManagerWhack
 */
#define DBManagerWhack( mgr ) \
    ( ( mgr ) == NULL ? 0 : ( * ( mgr ) -> whack ) ( mgr ) )

rc_t DBManagerRelease(const DBManager *mgr);

#ifdef __cplusplus
}
#endif

#endif /* _h_rdbms_rdbms_ */
