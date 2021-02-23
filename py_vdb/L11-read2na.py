#!/usr/bin/env python

import os, argparse, shutil, vdb

schema_txt = '''
version 1;

include 'vdb/vdb.vschema';

table SEQUENCE #1.0
{
    column <ascii> zip_encoding READ;
    column <U8> zip_encoding QUALITY;
};

''' 

rd_read_col_name = "(INSDC:dna:text)READ"
rd_qual_col_name = "(INSDC:quality:phred)QUALITY"

wr_read_col_name = "READ"
wr_qual_col_name = "QUALITY"

tmp_name = "tmp"

#"(INSDC:2na:packed)READ"
#"(INSDC:2na:bin)READ"
#"(INSDC:quality:text:phred_33)QUALITY"

def force_remove( dir ) :
    try :
        #if dir[ -1 ] == os.sep: dir = dir[ :-1 ]
        for file in os.listdir( dir ) :
            if file == '.' or file == '..': continue
            path = dir + os.sep + file
            if os.path.isdir( path ):
                force_remove( path )
            else:
                os.unlink( path )
        os.rmdir( dir )
    except :
        pass

def copy_table( rd_tbl, wr_tbl, first, count ) :
    rd_cur = rd_tbl.CreateCursor()
    rd_cols = rd_cur.OpenColumns( [ rd_read_col_name, rd_qual_col_name ] )
    rd_read_col = rd_cols[ rd_read_col_name ]
    rd_qual_col = rd_cols[ rd_qual_col_name ]
    
    first_row, row_count = rd_read_col.row_range()
    
    if first != None : first_row = first
    if count != None : row_count = count

    wr_cur = wr_tbl.CreateCursor( vdb.OpenMode.Write )
    wr_cols = wr_cur.OpenColumns( [ wr_read_col_name, wr_qual_col_name ] )
    wr_read_col = wr_cols[ wr_read_col_name ]
    wr_qual_col = wr_cols[ wr_qual_col_name ]

    for row in vdb.xrange( first_row, first_row + row_count ) :
        if row % 1000 == 0 : print( "row #{}".format( row ) )
        wr_cur.OpenRow()
        wr_read_col.write( rd_read_col.Read( row ) )
        wr_qual_col.write( rd_qual_col.Read( row ) )
        wr_cur.CommitRow()
        wr_cur.CloseRow()
    wr_cur.Commit()
     
if __name__ == '__main__' :
    parser = argparse.ArgumentParser()
    parser.add_argument( 'accession', nargs='*' )    
    parser.add_argument( '-X', '--first', metavar='row-id', help='first row-id', type=int, dest='first' )
    parser.add_argument( '-N', '--count', metavar='rows', help='how many reads', type=int, dest='count' )
    parser.add_argument( '-R', '--readlib', metavar='path', help='read library', type=str, dest='readlib' )    
    parser.add_argument( '-W', '--writelib', metavar='path', help='write library', type=str, dest='writelib' )
    parser.add_argument( '-O', '--output', metavar='path', help='output', type=str, dest='output', default="out" )    
    args = parser.parse_args()

    try :
        #open 2 managers:
        rd_mgr = vdb.manager( vdb.OpenMode.Read,  args.readlib )
        wr_mgr = vdb.manager( vdb.OpenMode.Write, args.writelib )
        nr = 1

        for acc in args.accession :
            rd_tbl = None
            pt = rd_mgr.PathType( acc ) #detect path-type ( database or table or anything-else )
            if pt == vdb.PathType.Database :
                rd_tbl = rd_mgr.OpenDB( acc ).OpenTable( "SEQUENCE" ) #object is a database
            elif pt == vdb.PathType.Table :
                rd_tbl = rd_mgr.OpenTable( acc ) #object is a table
            else :
                print( "%s is not an SRA-object"%( acc ) )

            if rd_tbl != None :
                force_remove( tmp_name )
                schema = wr_mgr.MakeSchema( schema_txt )
                wr_tbl = wr_mgr.CreateTable( schema, "SEQUENCE", tmp_name )
                
                if wr_tbl != None :
                    copy_table( rd_tbl, wr_tbl, args.first, args.count )

                    output_name = "{}.{}".format( args.output, nr )
                    nr += 1
                    try :
                        os.remove( output_name )
                    except :
                        pass
                    os.system( "kar -c {} -d {}".format( output_name, tmp_name ) )

    except vdb.vdb_error as e :
        print( e )
    except KeyboardInterrupt :
        print( "^C" )
    force_remove( tmp_name )
