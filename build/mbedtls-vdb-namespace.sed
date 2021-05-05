# rename everything on a line starting with "mbedtls_" to have a "vdb_mbedtls_" prefix
s/^\(mbedtls_[A-Za-z0-9_]*\)(/vdb_\1(/g
s/\([ \t*]\)\(mbedtls_[A-Za-z0-9_]*\)(/\1vdb_\2(/g

# also catch extern declarations
s/^\([ \t]*extern.*[ \t]\)\(mbedtls_[A-Za-z0-9_]*\);/\1vdb_\2;/g
s/^\([ \t]*extern.*[ \t]\)\(mbedtls_[A-Za-z0-9_]*\[\]\);/\1vdb_\2;/g

# this is to catch some decorated declarations
s/^\([ \t]*[A-Za-z0-9_][A-Za-z0-9_]*[ \t]\)\(mbedtls_[A-Za-z0-9_]*\);/\1vdb_\2;/g
s/^\([ \t]*[A-Za-z0-9_][A-Za-z0-9_]*[ \t]\)\(mbedtls_[A-Za-z0-9_]*\[\]\);/\1vdb_\2;/g

# this is to put back typedefs that went too far
s/^typedef\(.*\)vdb_mbedtls_/typedef\1mbedtls_/g

# special cases
s/\([^A-Za-z0-9_]\)mbedtls_platform_win32_snprintf\([^A-Za-z0-9_]\)/\1vdb_mbedtls_platform_win32_snprintf\2/g
s/\([^A-Za-z0-9_]\)mbedtls_platform_std_nv_seed_read\([^A-Za-z0-9_]\)/\1vdb_mbedtls_platform_std_nv_seed_read\2/g
s/\([^A-Za-z0-9_]\)mbedtls_platform_std_nv_seed_write\([^A-Za-z0-9_]\)/\1vdb_mbedtls_platform_std_nv_seed_write\2/g
s/\([^A-Za-z0-9_]\)mbedtls_calloc\([^A-Za-z0-9_]\)/\1vdb_mbedtls_calloc\2/g
s/\([^A-Za-z0-9_]\)mbedtls_free\([^A-Za-z0-9_]\)/\1vdb_mbedtls_free\2/g
s/\([^A-Za-z0-9_]\)mbedtls_fprintf\([^A-Za-z0-9_]\)/\1vdb_mbedtls_fprintf\2/g
s/\([^A-Za-z0-9_]\)mbedtls_printf\([^A-Za-z0-9_]\)/\1vdb_mbedtls_printf\2/g
s/\([^A-Za-z0-9_]\)mbedtls_snprintf\([^A-Za-z0-9_]\)/\1vdb_mbedtls_snprintf\2/g
s/\([^A-Za-z0-9_]\)mbedtls_exit\([^A-Za-z0-9_]\)/\1vdb_mbedtls_exit\2/g
s/\([^A-Za-z0-9_]\)mbedtls_nv_seed_read\([^A-Za-z0-9_]\)/\1vdb_mbedtls_nv_seed_read\2/g
s/\([^A-Za-z0-9_]\)mbedtls_nv_seed_write\([^A-Za-z0-9_]\)/\1vdb_mbedtls_nv_seed_write\2/g

s/\([^A-Za-z0-9_]\)mbedtls_platform_win32_snprintf$/\1vdb_mbedtls_platform_win32_snprintf/g
s/\([^A-Za-z0-9_]\)mbedtls_platform_std_nv_seed_read$/\1vdb_mbedtls_platform_std_nv_seed_read/g
s/\([^A-Za-z0-9_]\)mbedtls_platform_std_nv_seed_write$/\1vdb_mbedtls_platform_std_nv_seed_write/g
s/\([^A-Za-z0-9_]\)mbedtls_calloc$/\1vdb_mbedtls_calloc/g
s/\([^A-Za-z0-9_]\)mbedtls_free$/\1vdb_mbedtls_free/g
s/\([^A-Za-z0-9_]\)mbedtls_fprintf$/\1vdb_mbedtls_fprintf/g
s/\([^A-Za-z0-9_]\)mbedtls_printf$/\1vdb_mbedtls_printf/g
s/\([^A-Za-z0-9_]\)mbedtls_snprintf$/\1vdb_mbedtls_snprintf/g
s/\([^A-Za-z0-9_]\)mbedtls_exit$/\1vdb_mbedtls_exit/g
s/\([^A-Za-z0-9_]\)mbedtls_nv_seed_read$/\1vdb_mbedtls_nv_seed_read/g
s/\([^A-Za-z0-9_]\)mbedtls_nv_seed_write$/\1vdb_mbedtls_nv_seed_write/g

s/\([^A-Za-z0-9_]\)mbedtls_ecp_get_type$/\1vdb_mbedtls_ecp_get_type/g

# they put a bad comment into the *.h files to list a function that doesn't exist
# but collides with a typedef, and this causes us to issue a rename that we don't want
s/\([^A-Za-z0-9_]\)vdb_mbedtls_ecp_curve_info\([^A-Za-z0-9_]\)/\1mbedtls_ecp_curve_info\2/g
s/\([^A-Za-z0-9_]\)vdb_mbedtls_ecp_curve_info$/\1mbedtls_ecp_curve_info/g
