sub PACKAGE      { "ncbi-vdb" }
sub PACKAGE_NAME { "NCBI-VDB" }
sub PACKAGE_NAMW { "VDB" }
sub PKG { ( LNG => 'C',
            OUT => 'ncbi-outdir',
            PATH => '/usr/local/ncbi/ncbi-vdb', ) }
sub REQ { ( { name    => 'ngs-sdk',
              namw    => 'NGS',
              option  => 'with-ngs-sdk-prefix',
              srcpath => '../ngs/ngs-sdk',
              pkgpath => '/usr/local/ngs/ngs-sdk',
        } ) }
1
