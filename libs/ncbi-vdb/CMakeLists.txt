add_library( ncbi-vdb STATIC
    $<TARGET_OBJECTS:align-reader>
    $<TARGET_OBJECTS:ascp>
#TODO    $<TARGET_OBJECTS:srareader>
#TODO    $<TARGET_OBJECTS:sradb>
    $<TARGET_OBJECTS:vdb-blast>
    $<TARGET_OBJECTS:vdb>
    $<TARGET_OBJECTS:kdb>
    $<TARGET_OBJECTS:align-access>
    $<TARGET_OBJECTS:ncbi-bam>
#TODO    $<TARGET_OBJECTS:vfs>
	$<TARGET_OBJECTS:cloud>
	$<TARGET_OBJECTS:kxml>
#TODO    $<TARGET_OBJECTS:kxfs>
	$<TARGET_OBJECTS:mbedx509>
	$<TARGET_OBJECTS:mbedtls>
	$<TARGET_OBJECTS:mbedcrypto>
	$<TARGET_OBJECTS:kns>
	$<TARGET_OBJECTS:kfg>
#TODO    $<TARGET_OBJECTS:krypto>
#TODO    $<TARGET_OBJECTS:ksrch>
	$<TARGET_OBJECTS:kfs>
	$<TARGET_OBJECTS:klib>
	$<TARGET_OBJECTS:kproc>
#TODO    $<TARGET_OBJECTS:kq>
	$<TARGET_OBJECTS:kfc>
	$<TARGET_OBJECTS:bz2>
	$<TARGET_OBJECTS:z>
	$<TARGET_OBJECTS:zstd>
)
