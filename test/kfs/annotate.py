import sys, os
from collections import namedtuple

KarFile = namedtuple( 'KarFile', 'spos epos name' )

def ingest_kar_list( kar_list ) :
    l = list()
    with open( kar_list ) as f :
        for line in f :
            v = line.strip().split()
            if v[ 0 ] == "-r--r--r--" and v[ 1 ] != '-' :
                size = int( v[ 1 ] )
                pos  = int( v[ 2 ] )
                name = v[ 5 ]
                kf = KarFile( pos, pos + size - 1, name )
                l.append( kf )
    return l

def lookup( l, pos ) :
    res = "?"
    for kf in l :
        if kf.spos <= pos and kf.epos >= pos :
            res = kf.name
            break
    return res
    
def annotate( rec_file, kar_list ) :
    l = ingest_kar_list( kar_list )
    with open( rec_file ) as f :
        for line in f :
            line_s = line.strip()
            v = line_s.split()
            if v[ 0 ] == 'R' :
                print( "%s\t%s"%( line_s, lookup( l, int( v[ 1 ] ) ) ) )
            else :
                print( line_s )

if __name__ == '__main__':
    if len( sys.argv ) > 2 :
        rec_file = sys.argv[ 1 ]
        kar_list = sys.argv[ 2 ]
        found = 0
        if not os.path.isfile( rec_file ) :
            print( "'%s' not found"%( rec_file ) )
        else :
            found += 1
        if not os.path.isfile( kar_list ) :
            print( "'%s' not found"%( kar_list ) )
        else :
            found += 1
        if found == 2 :
            annotate( rec_file, kar_list )

    else :
        print( "no input given" )
        print( "example: python annotate xxx.rec xxx.kar.list" )
