import sys, os
from collections import namedtuple

Point = namedtuple( 'Point', 'x y' )

rect_fmt = "<rect x=\"%d\" y=\"%f\" width=\"%d\" height=\"%f\" style=\"fill:%s; stroke:none\" />\n"
line_fmt = "<line x1=\"%d\" y1=\"%f\" x2=\"%d\" y2=\"%f\" style=\"stroke:black; stroke-width:0.3\" />\n"
svg_fmt  = "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:ev=\"http://www.w3.org/2001/xml-events\" width=\"%d\" height=\"%d\">\n"

def svg_rect( f, p, w, h, color ) :
    f.write( rect_fmt%( p.x, p.y, w, h, color ) )

def svg_line( f, p1, p2 ) :
    f.write( line_fmt%( p1.x, p1.y, p2.x, p2.y ) )

def inspect( filename ) :
    if os.path.isfile( filename ) :
        print( "visualizing: %s"%( filename ) )

        max_pos = 0
        a_pos = []    # column NR 1  : request-position
        a_len = []    # column NR 2  : request-length

        with open( filename ) as f :
            for line in f :
                v = line.strip().split()
                if v[ 0 ] == "R" and len( v ) > 3:
                    pos = int( v[ 1 ] )
                    if pos > max_pos :
                        max_pos = pos;
                    a_pos.append( pos )
                    a_len.append( int( v[ 2 ] ) )
            f.close()

        with open( "%s.svg"%( filename ), "w" ) as f :

            ofs = 10
            divisor = 100000.0
    
            # calculate the extend of the svg-graphic
            svg_w = ( len( a_pos ) * 3 ) + ( 2 * ofs )
            svg_h = ( max_pos / divisor ) + ( 2 * ofs )
            f.write( svg_fmt%( svg_w, svg_h ) );

            p = Point( ofs, ofs )
            for i in range( len( a_pos ) ) :
            
                p1 = Point( p.x, a_pos[ i ] / divisor + ofs )
                h = a_len[ i ] / divisor
                
                svg_line( f, p, p1 )
                svg_rect( f, p1, 3, h, 'red' )

                p = Point( p1.x + 3, p1.y )

            f.write( "</svg>\n" )
            f.close()

if __name__ == '__main__':
    if len( sys.argv ) > 1 :
        inspect( sys.argv[ 1 ] )
    else :
        print( "no input given" )
        print( "example: python visual_request.py SRR341578.rec" )
