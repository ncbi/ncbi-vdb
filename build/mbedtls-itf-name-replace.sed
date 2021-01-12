/vdb_mbedtls/!d
s/^.*vdb_\(mbedtls_[A-Za-z0-9_]*\)[^A-Za-z0-9_].*$/s\/\\([^A-Za-z0-9_]\\)\1\\([^A-Za-z0-9_]\\)\/\\1vdb_\1\\2\/g/g
#s/^.*vdb_\(mbedtls_[A-Za-z0-9_]*\)[^A-Za-z0-9_].*$/s\/^\1\\([^A-Za-z0-9_]\\)\/vdb_\1\\1\/g/g

#s\/^mbedtls_aes_cmac_prf_128\\([^A-Za-z0-9_]\\)\/vdb_mbedtls_aes_cmac_prf_128\\1\/g


# s/^mbedtls_aes_cmac_prf_128\([^A-Za-z0-9_]\)/vdb_mbedtls_aes_cmac_prf_128\1/g
# s/\([^A-Za-z0-9_]\)mbedtls_aes_cmac_prf_128\([^A-Za-z0-9_]\)/\1vdb_mbedtls_aes_cmac_prf_128\2/g
# s/^mbedtls_aes_cmac_prf_128$/vdb_mbedtls_aes_cmac_prf_128/g
# s/\([^A-Za-z0-9_]\)mbedtls_aes_cmac_prf_128$/\1vdb_mbedtls_aes_cmac_prf_128/g
