################################################################################
sub PACKAGE      { 'ncbi-vdb' }
sub VERSION      { '2.4.2a' }
sub PACKAGE_TYPE { 'L' }
sub PACKAGE_NAME { 'NCBI-VDB' }
sub PACKAGE_NAMW { 'VDB' }
sub DEPENDS      { ( { name => 'hdf5' , },
                     { name => 'magic', Include => '/usr/include'        , },
                     { name => 'xml2' , Include => '/usr/include/libxml2', } ) }
sub CONFIG_OUT   { 'build' }
sub PKG { ( LNG   => 'C',
            OUT   => 'ncbi-outdir',
            PATH  => '/usr/local/ncbi/ncbi-vdb',
            UPATH =>      '$HOME/ncbi/ncbi-vdb', ) }
sub REQ { ( { name    => 'ngs-sdk',
              namew   => 'NGS',
              option  => 'with-ngs-sdk-prefix',
              origin  => 'I',
              type    => 'I',
              srcpath => '../ngs/ngs-sdk',
              pkgpath => '/usr/local/ngs/ngs-sdk',
              usrpath =>      '$HOME/ngs/ngs-sdk',
              bldpath => '$HOME/ncbi-outdir/ngs-sdk',
              include => 'ngs/itf/Refcount.h',
              lib => 'libngs-c++.a',
              ilib => 'libngs-bind-c++.a',
        },
        {     name    => 'hdf5',
              option  => 'with-hdf5-prefix',
              origin  => 'E',
              type    => 'LI',
              pkgpath => '/usr',
              usrpath => '$HOME',
              include => 'hdf5.h',
              lib => 'libhdf5.a',
        } ) }
1
