#!/usr/bin/env python

import sys, argparse, vdb

PY3 = sys.version_info[ 0 ] == 3

if PY3 :
    def xrange( *args, **kwargs ) :
        return iter( range( *args, **kwargs ) )

def fastq_from_tbl( args, tbl ) :
    acc = args.accession[ 0 ]
    
    col_names = [ "READ", "(INSDC:quality:text:phred_33)QUALITY", "NAME" ]
    if args.split :
        col_names.append( "READ_START" )
        col_names.append( "READ_LEN" )
    cols = tbl.CreateCursor().OpenColumns( col_names )
    
    c_read = cols[ col_names[ 0 ] ]
    c_qual = cols[ col_names[ 1 ] ]
    c_name = cols[ col_names[ 2 ] ]
    if args.split :
        c_read_start = cols[ col_names[ 3 ] ]
        c_read_len   = cols[ col_names[ 4 ] ]
    
    first, count = c_read.row_range()
    
    if args.first != None :
        first = args.first[ 0 ]
    if args.count != None :
        count = args.count[ 0 ]

    if args.split :
        fastq = '@{0}.{1}.{2} length={3}\n{4}\n+{0}.{1}.{2} length={3}\n{5}'
        for row in xrange( first, first + count ) :
            name     = c_name.Read( row )
            read     = c_read.Read( row )
            qual     = c_qual.Read( row )
            rd_start = c_read_start.Read( row )
            rd_len   = c_read_len.Read( row )
            for x in xrange( 0, len( rd_start ) ) :
                rlen  = rd_len[ x ]
                if rlen > 0 :
                    start = rd_start[ x ]
                    end   = start + rlen
                    print( fastq.format( acc, name, x+1, rlen, read[ start:end ], qual[ start:end ] ) )
    else :
        fastq = '@{0}.{1} length={2}\n{3}\n+{0}.{1} length={2}\n{4}'
        for row in xrange( first, first + count ) :
            read = c_read.Read( row )
            print( fastq.format( acc, c_name.Read( row ), len( read ), read, c_qual.Read( row ) ) )

    
if __name__ == '__main__' :
    parser = argparse.ArgumentParser()
    parser.add_argument( 'accession', nargs='*' )    
    parser.add_argument( '-X', '--first', metavar='row-id', help='first row-id', nargs=1, type=int, dest='first' )
    parser.add_argument( '-N', '--count', metavar='rows', help='how many reads', nargs=1, type=int, dest='count' )
    parser.add_argument( '--split', help='split spot', action='store_true' )
    args = parser.parse_args()
    
    try :
        #open a manager in read-mode ( dflt-mode )
        mgr = vdb.manager()
        
        for acc in args.accession :
            #detect path-type ( database or table or anything-else )
            pt = mgr.PathType( acc )
            if pt == vdb.PathType.Database :
                #object is a database
                fastq_from_tbl( args, mgr.OpenDB( acc ).OpenTable( "SEQUENCE" ) )
            elif pt == vdb.PathType.Table :
                #object is a table
                fastq_from_tbl( args, mgr.OpenTable( acc ) )
            else :
                print( "%s is not an SRA-object"%( acc ) )
    except vdb.vdb_error as e :
        print( e )
    except KeyboardInterrupt :
        print( "^C" )
