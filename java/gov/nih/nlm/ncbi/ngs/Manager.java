/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

package gov.nih.nlm.ncbi.ngs;

import ngs.ErrorMsg;
import ngs.ReadCollection;


/*==========================================================================
 * Manager
 *  NGS library manager
 */
class Manager
{

    ReadCollection openReadCollection ( String spec )
        throws ErrorMsg
    {
        return ReadCollectionItf . make ( spec );
    }

    Manager ()
        throws ExceptionInInitializerError
    {
        try
        {
            /* Load the DLL for JNI (download it?).

               To do just a plain call to System.LoadLibrary(libname)
                set LibManager.JUST_DO_REGULAR_JAVA_SYSTEM_LOAD_LIBRARY to true
                or set vdb.System.loadLibrary java system property. */

            LibManager m = new LibManager();
            if ( ! m . loadLibrary () ) {
                throw
                    new ExceptionInInitializerError ("Cannot load JNI library");
            }

            // try to initialize the library
            String err = Initialize ();
            if ( err != null )
                throw new ExceptionInInitializerError ( err );

            // install shutdown hook for library cleanup
            Runtime . getRuntime () . addShutdownHook
            (
                new Thread ()
                {
                    public void run ()
                    {
                        Shutdown ();
                    }
                }
            );
        }
        catch ( ExceptionInInitializerError x )
        {
            throw x;
        }
        catch ( Throwable x )
        {
            throw new ExceptionInInitializerError ( x );
        }
    }


    /** This jar's version : should match to JNI DLL's version */
                                //    1.0.  0
    static int version() { return 0x01000000; }

    private native static String Initialize ();
    private native static void Shutdown ();
}
