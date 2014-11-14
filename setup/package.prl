################################################################################
sub PACKAGE      { 'ncbi-vdb' }
sub VERSION      { '2.4.2-4' }
sub PACKAGE_TYPE { 'L' }
sub PACKAGE_NAME { 'NCBI-VDB' }
sub PACKAGE_NAMW { 'VDB' }
sub CONFIG_OUT   { 'build' }
sub PKG { ( LNG   => 'C',
            OUT   => 'ncbi-outdir',
            PATH  => '/usr/local/ncbi/ncbi-vdb',
            UPATH =>      '$HOME/ncbi/ncbi-vdb', ) }
sub DEPENDS { ( { name => 'hdf5' , Include => '/usr/include'        , },
                { name => 'magic', Include => '/usr/include'        , },
                { name => 'xml2' , Include => '/usr/include/libxml2', } ) }
sub REQ { ( { name    => 'ngs-sdk',
              aname   => 'NGS',
              option  => 'with-ngs-sdk-prefix',
              origin  => 'I',
              type    => 'Q',
              srcpath => '../ngs/ngs-sdk',
              pkgpath => '/usr/local/ngs/ngs-sdk',
              usrpath =>      '$HOME/ngs/ngs-sdk',
              bldpath => '$HOME/ncbi-outdir/ngs-sdk',
              include => 'ngs/itf/Refcount.h',
              lib     => 'libngs-c++.a',
            },
            { name    => 'hdf5',
              option  => 'with-hdf5-prefix',
              origin  => 'E',
              type    => 'LIO',
              pkgpath => '/usr',
              usrpath => '$HOME',
              include => 'hdf5.h',
              lib     => 'libhdf5.a',
            },
            { name    => 'xml2',
              option  => 'with-xml2-prefix',
              origin  => 'E',
              type    => 'LIO',
              pkgpath => '/usr',
              usrpath => '$HOME',
              include => 'libxml2/xmlreader.h',
              lib     => 'libxml2.a',
        } ) }
1
