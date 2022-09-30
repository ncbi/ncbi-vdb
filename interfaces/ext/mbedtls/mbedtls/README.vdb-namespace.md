# The NCBI SRA ( Sequence Read Archive )

### Contact:
email: sra-tools@ncbi.nlm.nih.gov

### LICENSE INFORMATION
Due to the fact that libncbi-vdb includes mbedtls as a sub-component, the
symbols for non-static functions and global data have been modified to
avoid the possibility of naming conflicts when linking against libncbi-vdb
and any other copy of mbedtls.

The namespace prefix "mbedtls_" has been modified to "vdb_mbedtls_". No
ownership or copyright is implied by this modification.
