# NCBI External Developer Release:

## NCBI VDB 2.8.0
**October 7, 2016**

### HTTPS-ENABLED RELEASE

  **kfg, kns**: Use environment variables to determine the proxy for a http protocol  
  **kfg, vdb-config**: vdb-config-GUI has now option to pick up proxy-setting from environment  
  **kns**: All tools and libraries now support https  
  **kns**: replaced all direct uses of sleep() within code to enforce standardization upon mS.  
  **kproc, ncbi-vdb**: Fixed KCondition to generate timeout error on Windows when timeout exhausted  
  **ngs-engine**: improved diagnostic messages  
  **test**: updated tests to not fail outside of NCBI  
  **test-sra**: test-sra prints network information  
  **test-sra**: test-sra prints version of ncbi-vdb or ngs-sdk dynamic library  
  **vdb**: improved parameter checking on VDatabaseOpenTableRead()  
  **vdb**: new function: "VDBManagerDeleteCacheOlderThan()"  
  **vdb**: problem with buffer-overrun when compressing random data fixed  
  **vdb**: remote/aux nodes have been removed from configuration  


## NCBI VDB 2.7.0
**July 12, 2016**

  **blast, kfg, ncbi-vdb, sra-tools, vfs**: restored possibility to disable local caching  
  **doc, ncbi-vdb**: created a Wiki page illustrating how to use the API to set up logging  
  **kdb, kfs, kns**: Added blob validation for data being fetched remotely  
  **kfg**: When loading configuration files on Windows USERPROFILE environment variable is used before HOME  
  **kfg**: modified auxiliary remote access configuration to use load-balanced servers  
  **kns**: Fixed a bug when KHttpRequestPOST generated an incorrect Content-Length after retry  
  **ngs, search, sra-search**: sra-search was modified to support multiple threads.  
  **ngs-engine, ngs-tools, sra-tools, vfs**: The "auxiliary" nodes in configuration are now ignored  
  **ngs-engine**: Added support for blob-by-blob access to SEQUENCE table  
  **ngs-engine**: removed a potential memory leak in NGS_CursorMake()  
  **search**: now supports multi-threaded search  
  **vdb**: fixed a bug in VCursorFindNextRowIdDirect where it returned a wrong rowId  
  **vdb**: fixed a bug in the code used to iterate over blobs where rowmap expansion cache would reset iteration to initial row instead of respecting sequence  
  **vfs**: environment variable VDB_PWFILE is no longer used  


## NCBI VDB 2.6.3
**May 25, 2016**

  **vdb**: Fixed a bound on memory cache that would never flush under certain access modes  

## NCBI VDB 2.6.2
**April 20, 2016**

  **kdb**: Fixed VTableDropColumn, so that it can drop static columns  
  **kfs, kns**: Fixed thread safety issues for both cache and http files  
  **kproc**: Fixed KQueue to wake waiters when sealed, fixed KSemaphore to wake waiters when canceled  
