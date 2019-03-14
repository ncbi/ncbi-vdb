# NCBI External Developer Release:


## NCBI VDB 2.9.6
**March 15, 2019**

  **prefetch, **vfs**: fixed regression that prevented re-download of incomplete files  


## NCBI VDB 2.9.4-1
**March 4, 2019**

  **sra-tools, **vfs**: fixed regression introduced in 2.9.4 release causing delay when starting sra tools  


## NCBI VDB 2.9.4
**January 31, 2019**

  **kns**: added detection of zone where cloud instance runs  
  **kns**: added support of AWS authentication  
  **kns**: allow to access googleapis.com via proxy  
  **kns**: don't add "Accept: */*" request header when it already contains an Accept header  
  **kns**: updated mbedtls to version 2.16  
  **sra-tools, vfs**: added support of realign objects  
  **vdb**: A Json parser is now available in klib  


## NCBI VDB 2.9.3
**October 11, 2018**

  **kns**: added possibility to skip server's certificate validation  
  **kns**: expect to receive HTTP status 200 when sending range-request that includes the whole file  
  **vdb**: fixed a bug in accessing pagemap process request for cursors which do not have pagemap thread running


## NCBI VDB 2.9.2-1
**July 31, 2018**

  **vdb**: restored lost exported attribute on several entries in the private API


## NCBI VDB 2.9.2
**July 23, 2018**

  **kfg, vfs**: Introduced enhanced handling of download-only NGC files that lack read/decrypt permissions


## NCBI VDB 2.9.1-1
**June 25, 2018**

  **ncbi-vdb**: fixed break of binary compatibility introduced in release 2.9.1


## NCBI VDB 2.9.1
**June 15, 2018**

  **build**: 'make install' ignore ROOT environment variable  
  **kfg, kns**: Added API to instruct VDB to silently accept any server certificate as valid.  
  **kfg, vdb-config**: name resolver service now makes use of fcgi  
  **kfg, vfs**: Fixed a bug that prevented decryption of objects encrypted with non-UTF8 text password keys  
  **klib**: added check for NULL format argument to string_printf()  
  **kns**: Randomly select from multiple proxies in configuration  
  **test**: added Windows test projects for the new schema and the latest vdb  


## NCBI VDB 2.9.0
**February 23, 2018**

  **align**: AlignAccessRefSeqEnumeratorNext no longer filters by the index  
  **align**: fixed an order-dependent bug affecting cross-table lookups  
  **build**: Created a script that allows to add a new volume to existing repository  
  **build**: Fixed configure allowing to run it on Perl with version >= v5.26 that has "." removed from @INC  
  **build**: added "smoke tests"  
  **build**: recognize version of libhdf5 that does not allow static linking and do not try to use it  
  **build, doc**: added wiki page: Building-from-source-:--configure-options-explained  
  **build, ncbi-vdb, sra-tools**: the installation script now saves configuration files if they were modified by the user  
  **build, vdb-sql**: modified build to avoid vdb-sql in absence of libxml2  
  **kfg**: added searching of configuration files in ../etc/ncbi/ relative to the binaries  
  **kfg, prefetch**: set limit of Aspera usage to 450m  
  **kfg, prefetch, remote-fuser, vfs**: Updated resolving of cache location of non-accession objects  
  **kfs**: fix to improve on windows  
  **klib**: Reverted KTimeMakeTime to use UTC  
  **kns**: Accept the same http_proxy specifications as wget  
  **kns**: Added possibility to report server's IP address after network error  
  **kns**: Ignore HTTP headers sent multiple times  
  **kns**: Improved reporting of network errors  
  **kns**: fixed generation of invalid error code in response to dropped connection  
  **ncbi-vdb**: add ability to make a cache-tee-file without promtion  
  **ncbi-vdb**: fixed bug of directory not found on mac  
  **ncbi-vdb, ngs-engine**: improved handling of blobs inside the NGS engine   
  **ngs-engine**: improved performance when iterating through partially aligned and unaligned reads  
  **ngs-engine**: optimized filtered access to unaligned runs  
  **sra-tools, vdb**: access to vdb/ngs via SQLite  
  **vdb**: An assert triggered by a rare condition was fixed  
  **vdb**: new api to estimate pileup-workload based on slice  
  **vdb**: new function to open HTTP-file with an arbitrary page-size  
  **vdb**: progressbar can now be created to output on stderr  
  **vfs**: Name resolving service was updated and switched to protocol version 3.0  


## NCBI VDB 2.8.2
**March 6, 2017**

  **blast**: Updated blast library to be able to process runs having empty rows  
  **build**: Added ability to specify ncbi-vdb/configure --with-magic-prefix. Look for libraries in (lib lib64) when running "configure --with-...-prefix"  
  **build**: configure detects location of ngs libraries  
  **build**: configure was fixed to skip options unrecognized by gcc 4.4.7  
  **build**: created sra-toolkit Debian package  
  **build**: fixed a bug in 'configure' when in could not find source files in repository saved with non-standard name  
  **build, kfg**: install updates certs.kfg along with other configuration files  
  **build, ncbi-vdb, sra-tools**: installation will back up old configuration files if they differ from the ones being installed  
  **kfs, vdb**: Fixes errors occuring when configuration is missing  
  **klib**: Fixed logging error reported as "log failure: RC(rcText,rcString,rcConverting,rcBuffer,rcInsufficient)"  
  **kns**: SRA tools respect standard set of environment variables for proxy specification  
  **kns**: rewrote socket code to use proper local error code switches, especially Windows   
  **kns**: updated mbedtls library to version 2.4.1  
  **ncbi-vdb**: configure in detects existence of ngs jar file  
  **ncbi-vdb, ngs, ngs-tools, sra-tools**: eliminated memcpy from sources due to potential for overlap  
  **test**: Improved testing framework  
  **vdb**: increased tenacity in flushing data to disk to accommodate behaviors of file systems such as Lustre.  
  **vdb-blast**: Implemented vdb-blast API for retrieving reference sequence  


## NCBI VDB 2.8.1
**December 22, 2016**

  **kfg**: added possibility to create an empty KConfig object that does not try to load any file  


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
