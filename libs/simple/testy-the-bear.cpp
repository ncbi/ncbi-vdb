#include <simple/sra.hpp>

#include <iostream>

namespace sra
{
    static
    void dump_read ( const SRASequence & seq )
    {
        std :: cout
            << "  name: '"
            <<  seq . name ()
            << "', technical: "
            << seq . isTechnical ()
            << ", reverse: "
            << seq . isReverse ()
            << ", color-space: "
            << seq . colorSpaceNative ()
            << std :: endl
            << "  "
            << seq . bases ()
            << std :: endl
            << "  "
            << seq . colorSpace ()
            << std :: endl;
    }

    static
    void dump_spot ( const SRASpot & spot )
    {
        std :: cout
            << spot . id ()
            << ". "
            << spot . numReads ()
            << " total reads, "
            << spot . numBioReads ()
            << " are biological:"
            << std :: endl;

        SRASequenceIterator it = spot . reads ();
        while ( it . next () )
            dump_read ( it );
    }

    static
    void dump ( SRASpotIterator & it )
    {
        while ( it . next () )
            dump_spot ( it );
    }

    static
    int run ()
    {
        try
        {
            // open the run
            SRAObject obj ( "SRR000123" );

            // open it - should be a nop
            obj . open ();

            // get a spot iterator on the first 10 spots
            SRASpotIterator it = obj . allSpots ( 1, 10 );

            // dump them
            dump ( it );
        }
        catch ( SRAException &x )
        {
            std :: cerr
                << x . what ()
                << std :: endl;

            return -1;
        }
        return 0;
    }
}

using namespace sra;

int main ()
{
    try
    {
        return run ();
    }
    catch ( ... )
    {
    }
    return -1;
}
