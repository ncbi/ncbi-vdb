#ifndef _h_rdbms_sybase_
#define _h_rdbms_sybase_

#ifndef _h_rdbms_rdbms_
#include <rdbms/rdbms.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * Sybase
 */

/* constants
 */
#ifndef CS_VERSION_100
#define CS_VERSION_100 112
#endif

#ifndef CS_VERSION_110
#define CS_VERSION_110 1100
#endif

#ifndef CS_VERSION_125
#define CS_VERSION_125 12500
#endif

#ifndef CS_VERSION_150
#define CS_VERSION_150 15001
#endif

#ifndef MIN_CS_VERSION
#define MIN_CS_VERSION CS_VERSION_125
#endif

#ifndef OS_CS_VERSION
#define OS_CS_VERSION CS_VERSION_125
#endif

/* SybaseInit
 *  initializes the sybase rdbms
 */
rc_t SybaseInit ( int version );


#ifdef __cplusplus
}
#endif

#endif /* _h_rdbms_sybase_ */
