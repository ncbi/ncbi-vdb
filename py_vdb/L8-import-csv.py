#!/usr/bin/env python

import vdb, csv, os

schematxt = '''
version 1;
table CSVTAB #1.0
{
    column U16 C1;
    column ascii C2;
};
'''

def write_csv( filename, rowcount ) :
    with open( filename, 'w' ) as f :
        for i in range( 1, rowcount ) :
            f.write( "%d, line # %d\n"%( i, i ) )
        f.close()
        
def csv_to_vdb( mgr, csv_file, spec, vdb_table ) :
    with open( csv_file, 'r' ) as f :
        spec = 'CSVTAB'
        tbl_wr = mgr.CreateTable( mgr.MakeSchema( schematxt ), spec, vdb_table )
        cur = tbl_wr.CreateCursor( vdb.OpenMode.Write )
        cols = cur.OpenColumns( [ "C1", "C2" ] )
        for row in csv.reader( f ) :
            cur.OpenRow()
            cols[ "C1" ].write( [ int( row[ 0 ] ) ] )
            cols[ "C2" ].write( row[ 1 ].strip() );
            cur.CommitRow()
            cur.CloseRow()
        cur.Commit()

if __name__ == '__main__':
    csv_file = 'data.txt'
    write_csv( csv_file, 10 )
    try :
        mgr = vdb.manager( vdb.OpenMode.Write, "./2.8.0/libncbi-wvdb.so.2.8.0" )
        spec = 'CSVTAB'
        csv_to_vdb( mgr, csv_file, spec, 'L8' )
        mgr.OpenTable( spec ).print_rows()
    except vdb.vdb_error as e :
        print( e )
    os.remove( csv_file )
