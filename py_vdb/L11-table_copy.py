#!/usr/bin/env python

import os, sys, argparse, shutil, vdb

schema_txt = '''
version 1;

include 'vdb/vdb.vschema';

table SEQUENCE #1.0
{
    column default limit = 8000000;

    column <ascii> zip_encoding READ;
    column <U8> zip_encoding QUALITY;
    column <U32> zip_encoding READ_LEN;
    column <U32> zip_encoding READ_START;
};

''' 

#a list of name-tuples: (rd-name, wr-name )
column_names = [ ( "(INSDC:dna:text)READ", "READ" ),
                 ( "(INSDC:quality:phred)QUALITY", "QUALITY" ),
                 ( "READ_LEN", "READ_LEN" ),
                 ( "READ_START", "READ_START" ) ]

class progressbar :
    def __init__( self, width : int, max_value : int ) :
        self.width = width
        self.max_value = max_value
        self.cur_value = 0
        self.dashes = 0
        sys.stdout.write( "[{}]".format( " " * width ) ) 
        sys.stdout.flush()
        sys.stdout.write( "\b" * ( width + 1 ) )

    def update( self ) :
        new_dashes = ( self.width * self.cur_value ) // self.max_value
        if new_dashes > self.dashes :
            sys.stdout.write( "-" * ( new_dashes - self.dashes ) )
            sys.stdout.flush()
            self.dashes = new_dashes

    def inc( self, by : int ) :
        if by < 0 : return
        self.cur_value += by
        if self.cur_value > self.max_value : self.cur_value = self.max_value
        self.update()

    def finish( self ) :
        self.cur_value = self.max_value
        self.update()
        print( "" )
        print( "{} rows.".format( self.max_value ) )

def copy_table( rd_tbl, wr_tbl, first : int, count : int, freq : int ) :
    rd_col_names = list()
    wr_col_names = list()
    for rd_name,wr_name in column_names :
        rd_col_names.append( rd_name )
        wr_col_names.append( wr_name )
       
    rd_cur = rd_tbl.CreateCursor()
    rd_cols = rd_cur.OpenColumns( rd_col_names )

    first_row, row_count = rd_cols[ rd_col_names[ 1 ] ].row_range()
    if first != None : first_row = first
    if count != None : row_count = count
    
    wr_cur = wr_tbl.CreateCursor( vdb.OpenMode.Write )
    wr_cols = wr_cur.OpenColumns( wr_col_names )

    columns = list()
    for i in vdb.xrange( len( rd_col_names ) ) :
        rdc = rd_cols[ rd_col_names[ i ] ]
        wrc = wr_cols[ wr_col_names[ i ] ]
        columns.append( ( rdc, wrc ) )
    
    pb = progressbar( 80, row_count )
    for row in vdb.xrange( first_row, first_row + row_count ) :
        if row % freq == 0 : pb.inc( freq )
        wr_cur.OpenRow()                        #this is the meat of the copy-operation
        for rdc, wrc in columns :               #
            wrc.write( rdc.Read( row ) )        #
        wr_cur.CommitRow()                      #
        wr_cur.CloseRow()                       #
    wr_cur.Commit()
    pb.finish()

if __name__ == '__main__' :
    parser = argparse.ArgumentParser()
    parser.add_argument( 'accession' )
    parser.add_argument( '-X', '--first', metavar='row-id', help='first row-id', type=int, dest='first' )
    parser.add_argument( '-N', '--count', metavar='rows', help='how many reads', type=int, dest='count' )
    parser.add_argument( '-R', '--readlib', metavar='path', help='read library', type=str, dest='readlib' )    
    parser.add_argument( '-W', '--writelib', metavar='path', help='write library', type=str, dest='writelib' )
    parser.add_argument( '-O', '--output', metavar='path', help='output', type=str, dest='output', default="tmp" )
    parser.add_argument( '-F', '--freq', metavar='number', help='progress freq', type=int, dest='freq', default=2000 )
    args = parser.parse_args()

    try :
        print( "making a copy of : {}".format( args.accession ) )
        #open 2 managers:
        rd_mgr = vdb.manager( vdb.OpenMode.Read,  args.readlib )
        wr_mgr = vdb.manager( vdb.OpenMode.Write, args.writelib )

        rd_tbl = None
        pt = rd_mgr.PathType( args.accession ) #detect path-type ( database or table or anything-else )
        if pt == vdb.PathType.Database :
            rd_tbl = rd_mgr.OpenDB( args.accession ).OpenTable( "SEQUENCE" ) #object is a database
        elif pt == vdb.PathType.Table :
            rd_tbl = rd_mgr.OpenTable( args.accession ) #object is a table
        else :
            print( "%s is not an SRA-object"%( args.accession ) )

        if rd_tbl != None :
            schema = wr_mgr.MakeSchema( schema_txt )
            wr_tbl = wr_mgr.CreateTable( schema, "SEQUENCE", args.output )
            if wr_tbl != None :
                copy_table( rd_tbl, wr_tbl, args.first, args.count, args.freq )

    except vdb.vdb_error as e :
        print( e )
    except KeyboardInterrupt :
        print( "^C" )
