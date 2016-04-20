# NCBI External Developer Release:
## NCBI VDB 2.6.2

  **kdb**: Fixed VTableDropColumn, so that it can drop static columns  
  **kfs, kns**: Fixed thread safety issues for both cache and http files  
  **kproc**: Fixed KQueue to wake waiters when sealed, fixed KSemaphore to wake waiters when canceled  
