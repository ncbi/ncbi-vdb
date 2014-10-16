################################################################################
sub PACKAGE      { 'ncbi-vdb' }
sub VERSION      { '2.4.2-1' }
sub PACKAGE_TYPE { 'L' }
sub PACKAGE_NAME { 'NCBI-VDB' }
sub PACKAGE_NAMW { 'VDB' }
sub DEPENDS      { qw(hdf5 xml2 magic) }
sub CONFIG_OUT   { 'build' }
sub PKG { ( LNG   => 'C',
            OUT   => 'ncbi-outdir',
            PATH  => '/usr/local/ncbi/ncbi-vdb',
            UPATH =>      '$HOME/ncbi/ncbi-vdb', ) }
sub REQ { ( { name    => 'ngs-sdk',
              namew   => 'NGS',
              option  => 'with-ngs-sdk-prefix',
              type    => 'I',
              srcpath => '../ngs/ngs-sdk',
              pkgpath => '/usr/local/ngs/ngs-sdk',
              usrpath =>      '$HOME/ngs/ngs-sdk',
              bldpath => '$HOME/ncbi-outdir/ngs-sdk/$OS',
              include => 'ngs/itf/Refcount.h',
              lib => 'libngs-sdk.so',
              ilib => 'libngs-bind-c++.a',
        } ) }
1
