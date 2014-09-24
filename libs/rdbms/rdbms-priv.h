#ifndef _h_rdbms_priv_
#define _h_rdbms_priv_

#ifndef _h_klib_rc_
#include <klib/rc.h> /* RC */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * db error codes
 */
enum
{
    dbNoErr,
    dbUnknownErr,
    dbInvalid,
/*  dbUnsupported,*/
    dbMemErr,
    dbConfigErr,
    dbConnectErr,
/*  dbCmdErr,*/
    dbSendErr,
/*  dbBadResponse,*/
    dbFetchErr,
    dbEndData,
    dbNullColumn,
    dbNotFound,
    dbRangeErr,
    dbBuffSpErr,
    dbExists,
    dbNumErrCodes
};

rc_t statusToRC(int status);

#ifdef __cplusplus
}
#endif

#endif /* _h_rdbms_priv_ */
