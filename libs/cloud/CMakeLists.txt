add_library(cloud STATIC
    aws-auth.c
    aws.c
    cloud-cmn.c
    cloud-mgr.c
    cloud.c
    gcp-authentication.c
    gcp.c
)

add_compile_definitions( __mod__="libs/cloud" )