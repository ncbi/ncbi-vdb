# NCBI External Developer Release:


## NCBI VDB 3.0.8
**September 19, 2023**

  **align, bam-load**: 'circular' can be set from configuration for fasta references  
  **test, vfs**: fixed test crash  


## NCBI VDB 3.0.7
**August 29, 2023**

  **abi-load, fastq-load, helicos-load, illumina-load, pacbio-load, sff-load, sra, srf-load**: fixed loaders to work without predefined schema  
  **align-info, prefetch, vdb, vfs**: stopped printing incorrect 'reference not found' messages; reduced number of calls to resolve WGS references; align-info don't resolve references remotely if found locally  
  **bam-load, kdb, vdb-validate**: index works correctly with only one key  
  **cloud, kns, ngs-tools, sra-tools**: added support of IMDSv2 to allow to use instance identity on new AWS machines  
  **cloud, ngs-tools, sra-tools, vdb-config**: fixed use of AWS credentials  
  **kns, ngs, ngs-tools, sra-tools**: fixed a bug that caused failure during accession resolution while reading HTTP stream  
  **vdb**: added database-contained view aliases to the schema language  


## NCBI VDB 3.0.6
**July 10, 2023**

  **cloud, sra-tools**: fixed a bug in reading chunked HTTP responses in GCP-related code  
  **schema, vdb**: upgraded the schema parser  


## NCBI VDB 3.0.5
**May 9, 2023**

  **blast**: added possibility to return filtered reads  
  **build, ncbi-vdb**: ncbi-vdb upgraded to mbedtls major version 3 to fix crypto errors on some Macs (clang 14), and ncbi-vdb will not use the system mbedtls  
  **build, ncbi-vdb**: downgraded internal mbedtls to 3.2.1 to fix MSVC 2017 build issues  
  **kfg, ngs-tools, sra-tools, vfs**: stopped using old names resolver cgi  


## NCBI VDB 3.0.2
**December 12, 2022**

  **ncbi-vdb**: changed version to match that of _sra-tools_


## NCBI VDB 3.0.1
**November 15, 2022**

  **build**: adapted to support building for Conan
  **build**: added support for generating coverage reports
  **build**: added support for overriding cmake and ctest commands
  **build**: in source builds are no longer supported
  **build**: now supports custom library installation directory
  **build**: will use a system-provided libmbedtls, otherwise the copy included in the source code will be used
  **kff**: now using system-provided libmagic if present
  **kfg**: fixed a bug that caused override of user configuration
  **kfg, ncbi-vdb**: fixed tests that overwrite user configuration
  **kfg, sra-tools**: removed interactive requirement to configure SRA Toolkit
  **kfs**: fixed bug with long paths
  **kns, sra-tools**: allow to use AWS data with compute environment via proxy
  **ncbi-vdb**: fixed crash related to mbedtls
  **ncbi-vdb**: fixed failing tests related to sralite
  **pacbio-load, vdb**: hdf5 support is now a part of sra-tools/pacbio-load; uses system-provided libhdf5
  **sra-pileup, vfs**: fixed crash when working with no-quality run
  **sra-tools, vdb**: retired vdb-get
  **sra-tools, vfs**: fixed infinite loop when processing bad response when resolving accession
  **vdb**: fixed a bug that allowed a null pointer to be used
  **vdb**: kxml and kxfs are combined and moved to sra-tools
  **vdb**: memmove should be used instead of memcpy in this repository. We added an automated check for that. New code containing memcpy will fail this check
  **vdb**: updated zlib version to fix vulnerability
  **vdb**: VTable can copy columns
  **vfs**: now recognizes sralite.vdbcache files


## NCBI VDB 3.0.0
**February 10, 2022**

  **blast**: processes some unusual ILLUMINA runs
  **blast, test**: fixed a missed return
  **build, ncbi-vdb**: see https://github.com/ncbi/ncbi-vdb/wiki/Building-and-Installing-from-Source for updated build instructions
  **build, ncbi-vdb, sra-tools**: added support for building with XCode
  **build, ncbi-vdb, sra-tools**: the build system has been converted to use CMake
  **klib**: removed asserts from RC headers
  **klib**: result codes can be more easily converted to strings
  **kns**: environment variables NCBI_USAGE_PLATFORM_NAME and NCBI_USAGE_PLATFORM_VERSION are sent to NCBI
  **kns, test**: test was improved
  **kfs, sra-tools**: fixed a bug in handling of long URLs on Windows


## NCBI VDB 2.11.2
**October 7, 2021**

  **klib, ngs-tools, sra-tools**: status messages (-v) are printed to stderr rather than stdout
  **kns, ngs-tools, sra-tools**: old verbose messages now happen at verbosity > 1
  **ncbi-vdb, ngs-tools, sra-tools, vdb, vfs**: added  support of SRA Lite files with simplified base quality scores
  **prefetch, vfs**: accept Percent Encoding in source URLs
  **sra-tools, vdb**: environment variable NCBI_TMP_CACHE sets the caching directory, overriding any other setting


## NCBI VDB 2.11.1
**August 17, 2021**

  **align, axf, sra-pileup, vdb, vfs**: resolve reference sequences within output directory
  **cloud, kns, sra-tools**: do not acquire CE more often than necessary
  **kapp, kns**: fixed a crash that occurred when multiple threads update HTTP's User-Agent header
  **kfg**: improved thread safety
  **kns**: fixed test failure on GCP
  **kns**: improved reporting of peer certificate information
  **kns**: improved thread safety when proxy is used
  **kns**: improved timeout management in CacheTeeFile
  **kns**: integrated mutual TLS authentication
  **ncbi-vdb**: added dependency on cmake
  **ncbi-vdb, ngs, ngs-tools, sra-tools**: configure prints the version of compiler
  **prefetch, vdb, vfs**: prefetch with "-O" will now correctly place references in output directory
  **prefetch, vfs**: fixed error message 'multiple response SRR URLs for the same service...' when downloading
  **vdb**: support for Zstd added to the schema
  **vfs**: updated interaction with SRA Data Locator


## NCBI VDB 2.11.0
**March 15, 2021**

  **build, ncbi-vdb, ngs, ngs-tools**: introduced an additional external library, libncbi-ngs
  **kfg, prefetch, vfs**: resolve WGS reference sequences into "Accession Directory"
  **kfg, sra-tools, vfs**: dropped support of protected repositories
  **kns, sra-tools**: fixed formatting of HTTP requests for proxy
  **ktst**: improved thread safety
  **ncbi-vdb, ngs, ngs-tools, sra-tools, vdb**: added support for 64-bit ARM (AArch64, Apple Silicon)
  **prefetch, vfs**: fixed download of protected non-run files
  **prefetch, vfs**: fixed segfault during download of JWT cart
  **prefetch, vfs**: respect requested version when downloading WGS files
  **sra-tools, vfs**: recognize sra file names with version


## NCBI VDB 2.10.9
**December 16, 2020**

  **align**: alignment enumerator structure is now fully initialized
  **align, vdb**: fixed situation where network access could drastically slow down reading references
  **build**: added configure option to produce build in output directory relative to sources
  **cloud**: implemented using OAuth 2.0 to access GCP buckets
  **kfs**: added support of hard links
  **kfs, kns**: improved timeout management in CacheTeeFile
  **kns**: env_token and pay_required flags are now treated independently
  **kns**: memory leak was fixed
  **kns**: removed memory leaks in KNS, VDB, VFS
  **kns, sra-tools, vdb**: added a loop to retry failed connections when fetching SRA files
  **test**: updated test not to fail when the source is not available
  **vdb**: writing a big (>3MB) cell now triggers a page commit
  **vfs**: allow to find local files when remote repository is disabled
  **vfs**: not to call names.cgi but SDL when resolving runs and reference sequences
  **vfs**: support of ETL - BQS runs


## NCBI VDB 2.10.8
**June 29, 2020**

  **kproc, fasterq-dump**: fixed problem with seg-faults caused by too small stack used by threads
  **kdb, vdb, vfs, sra-tools**: fixed bug preventing use of path to directory created by prefetch if it ends with '/'
  **vfs, sra-tools, ngs-tools**: report an error when file was encrypted for a different ngc file
  **vfs, prefetch**: download encrypted phenotype files with encrypted extension
  **vdb, sra-docker**: config can auto-generate LIBS/GUID when in a docker container


## NCBI VDB 2.10.7
**May 21, 2020**

  **kns, ngs-tools, sra-tools**: added new header to HTTP requests to communicate VDB version


## NCBI VDB 2.10.6
**May 15, 2020**

  **align, sra-tools**: fixed fetching of reference sequences from cloud
  **align, sra-tools, vfs**: fixed resolving of hs37d5 reference sequence
  **axf**: fixed invalid soft clips with hard clipped secondary alignments
  **kfg, sra-tools**: ignore configuration with invalid protected user repository having a single 'root' node
  **kns, sra-tools**: added new header to HTTP requests to communicate SRA version
  **kns, sra-tools**: close socket when accessing GCP files
  **kns, sra-tools**: introduced a additional configurable network retry loop
  **kns**: Windows connect() is now asynchronous and supporting timeouts
  **krypto, sra-tools, vfs**: fixed decryption when password contains # symbol
  **sratools, vdb-dump, vfs**: fixed vdb-dump <accession of prefetched run>
  **sra-tools, vdb**: restored possibility to cache WGS references to user repository
  **sra-tools, vfs**: fixed working with runs having WGS reference sequences
  **sraxf**: synthetic qualities are now generated per read
  **sraxf**: synthetic qualities are now generated per read
  **vdb**: fixed segfault in VSchemaDumpToKMDataNode
  **vfs**: added possibility to control the verbosity of negative response of names service


## NCBI VDB 2.10.5
**April 1, 2020**

  **build**: the Windows build now uses MS Visual Studio 2017
  **cloud**: cloud-specific test projects have been ported to Windows
  **cloud**: SRA Tools now filter out cases when internet providers successfully resolve not found DNS entries
  **kfg**: AWS credential files are not opened without user permission
  **kns**: Internet socket connections are now blocking on Windows
  **kns**: KTimeMakeTime() on Windows now returns UTC, not local time
  **kns**: SRA Tools were updated to not hang on HTTP calls when the server doesn't return Content-Length in GET requests without a range
  **kns**: use of root CA certificates is now supported on Windows
  **ncbi-vdb, ngs, ngs-tools, sra-tools**: all Linux builds now use g++ 7.3 (C++11 ABI)
  **vdb**: fixed some POSIX header include issues


## NCBI VDB 2.10.4
**February 26, 2020**

  **kns**: fixed errors when using ngc file


## NCBI VDB 2.10.3
**February 18, 2020**

  **sraxf**: fixed a problem resulting in a segmentation fault


## NCBI VDB 2.10.2
**January 15, 2020**

  **build**: install used to fail on OSX when checking md5
  **kfg**: Users now provide the ngc access token on the command line
  **kfg**: potential memory leak removed
  **kfg, vdb-config, vfs**: new names for caching dbGaP files
  **kfg, vfs**: added new configuration node for resolving protected data
  **klib**: fixed incorrect access to memory in output writers
  **kns**: KNS now provides limited support for attaching files to HTTP POST requests
  **kns, vfs**: added support of ngc files and JWT-carts in SDL requests
  **krypto**: fixed decryption of dbGaP data on the fly
  **vfs**: allow to set dbGaP context by ngc file
  **vfs**: fixed detection of cache location of public runs on cloud
  **vfs**: fixed double-free when processing names service response
  **vfs**: fixed names conflict
  **vfs**: new naming scheme for caching dbGaP files


## NCBI VDB 2.10.1
**December 16, 2019**

  **vfs**: fixed generation of unique file name when caching


## NCBI VDB 2.10.0
**August 19, 2019**

  **cloud**: added support for user-pays access to GCP
  **cloud**: cloud-related code was moved to new module
  **cloud, vfs**: send cloud instance identity just when user allows it
  **kfg, sra-tools**: use trace.ncbi.nlm.nih.gov to call names service
  **kfg, vdb-config**: alternative remote repository URL was added to default configuration
  **kfs**: introduced readahead strategy for cloud storage
  **klib, vdb**: error report is saved to ncbi_error_report.txt
  **kns**: Don't log mdebtls error messages. Added possibility to enable them.
  **kns**: We now use system root CA certs on Unix
  **kns**: increased default value of HTTP read to 5 minutes; made it configurable
  **kns**: introduced configurable controls over network timeouts
  **kns**: special support for handling SDL interaction
  **prefetch, vdb**: adjustments for latest name resolution service
  **prefetch, vfs**: added support of "run accession as directory"
  **prefetch, vfs**: added support of download of reference sequences in "run accession as directory"
  **prefetch, vfs**: fixed regression when prefetch does not download vdbcache
  **sratools**: VDB can get URLs- for local/remote/cache locations of accession from environment
  **tui, vdb-config**: new look and cloud specific options in 'vdb-config -i'
  **vdb**: VDBManagerMakeSRASchema() was deprecated
  **vdb**: make greater use of data returned by latest name resolver
  **vfs**: added possibility to have multiple remote repositories for backward compatibility
  **vfs**: added possibility to set resolver version from configuration
  **vfs**: allow to use SDL as remote service


## NCBI VDB 2.9.6
**March 18, 2019**

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
