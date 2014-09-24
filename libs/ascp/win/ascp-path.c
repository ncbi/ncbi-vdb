#include "ascp-priv.h" /* ascp_path */
#include <assert.h>

bool ascp_path(const char **cmd, const char **key) {
    static int idx = 0;
    static const char *c[] = {
        "C:\\Program Files (x86)\\Aspera\\Aspera Connect\\bin\\ascp.exe",
        "C:\\Program Files\\Aspera\\Aspera Connect\\bin\\ascp.exe"
    };
    static const char *k[] = {
 "C:\\Program Files (x86)\\Aspera\\Aspera Connect\\etc\\asperaweb_id_dsa.putty",
 "C:\\Program Files\\Aspera\\Aspera Connect\\etc\\asperaweb_id_dsa.putty"
    };
    assert(cmd && key && sizeof c / sizeof c[0] == sizeof k / sizeof k[0]);
    if (idx < sizeof c / sizeof c[0]) {
        *cmd = c[idx];
        *key = k[idx];
        ++idx;
        return true;
    }
    else {
        *cmd =  *key = NULL;
        idx = 0;
        return false;
    }
}
