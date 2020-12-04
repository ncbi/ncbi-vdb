import sys, os
from collections import OrderedDict
from operator import itemgetter

def inspect( filename ) :
    if os.path.isfile( filename ) :
        print( "inspecting: %s"%( filename ) )

        max = 0
        min = 0
        sum = 0
        cnt = 0
        
        d = {}
        
        with open( filename ) as f :
            for line in f :
                v = line.strip().split()
                if v[ 0 ] == "R" and len( v ) > 3:

                    req_pos = int( v[ 1 ] )
                    if req_pos in d.keys() :
                        d[ req_pos ] = d[ req_pos ] + 1
                    else :
                        d[ req_pos ] = 1
                        
                    req_len = int( v[ 3 ] )                        
                    sum += req_len
                    cnt += 1
                    if req_len > max :
                        max = req_len;
                    if min == 0 :
                        min = req_len
                    elif req_len < min :
                        min = req_len

        print( "\treq\t{:,d}".format( cnt ) )
        print( "\tmin\t{:,d}".format( min ) )
        print( "\tmax\t{:,d}".format( max ) )
        print( "\tsum\t{:,d}".format( sum ) )

        d1 = {}
        repeats = 0
        for k_pos, v_cnt in d.iteritems() :
            if v_cnt > 1 :
                d1[ k_pos ] = v_cnt
                repeats += v_cnt
        print( "\trepeats\t{:,d}".format( repeats ) )
        
        od = OrderedDict( reversed( sorted( d1.items(), key = itemgetter( 1 ) ) ) )
        i = 0
        for k_pos, v_cnt in od.iteritems() :
            print( "\tpos {:,d}\t{:d} times".format( k_pos, v_cnt ) )
            i += 1
            if i > 5 :
                break
        print( " " )

if __name__ == '__main__':
    if len( sys.argv ) > 1 :
        inspect( "%s.rec"%( sys.argv[ 1 ] ) )
        inspect( "%s.inner.rec"%( sys.argv[ 1 ] ) )        
        inspect( "%s.outer.rec"%( sys.argv[ 1 ] ) )        
    else :
        print( "no input given" )
        print( "example: python inspect_rec.py SRR341578" )
