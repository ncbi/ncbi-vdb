#!/usr/bin/env python

import os, sys, argparse, sqlite3

class sqlite_helper :
    def __init__( self, filename : str ) :
        self.try_connect( filename )

    def __del__( self ) :
        if self.con != None :
            self.con.close()

    def try_connect( self, filename : str ) :
        try :
            self.con = sqlite3.connect( filename )
        except Exception as e :
            print( e )
            self.con = None

    def try_exec( self, stm : str ) -> bool :
        if self.con == None :
            return False
        try :
            with self.con :
                self.con.execute( stm )
            return True
        except Exception as e :
            print( e )
            return False

class data :
    def __init__( self, acc : str, org : str, c1 : str, c2 : str ) :
        self.acc = acc
        self.org_size = os.path.getsize( org )
        self.c1_size = os.path.getsize( c1 )
        self.c2_size = os.path.getsize( c2 )        
        self.percent = ( self.c2_size * 100 ) // self.org_size
        
    def __str__( self ):
        return f"accession : {self.acc}\noriginal  : {self.org_size}\ncopy1     : {self.c1_size}\ncopy2     : {self.c2_size}"

class records( sqlite_helper ) :
    def __init__( self, filename ) :
        super().__init__( filename )
        self.create_tables()

    def create_tables( self ) -> bool :
        return self.try_exec( """
            CREATE TABLE IF NOT EXISTS data (
                acc STR PRIMARY KEY UNIQUE,
                org_size INTEGER,
                copy1 INTEGER,
                copy2 INTEGER,
                percent INTEGER
            );""" )

    def write( self, d : data ) -> bool :
        stm = f"""
            INSERT INTO data ( acc, org_size, copy1, copy2, percent )
            VALUES ( '{d.acc}', {d.org_size}, {d.c1_size}, {d.c2_size}, {d.percent} )
        """
        return self.try_exec( stm )


if __name__ == '__main__' :
    parser = argparse.ArgumentParser()
    parser.add_argument( 'accession', nargs=1, type=str )
    parser.add_argument( '-O', '--org', metavar='path', help='original accession', type=str, dest='org', required=True )
    parser.add_argument( '-1', '--copy1', metavar='path', help='1st copy', type=str, dest='copy1', required=True )
    parser.add_argument( '-2', '--copy2', metavar='path', help='2nd copy', type=str, dest='copy2', required=True )
    parser.add_argument( '-D', '--db', metavar='path', help='database to record', type=str, dest='db', required=True )
    args = parser.parse_args()

    if not os.path.exists( args.org ) :
        print( f"original '{args.org}' not found" )
        sys.exit( 3 )

    if not os.path.exists( args.copy1 ) :
        print( f"copy1  '{args.copy1}' not found" )
        sys.exit( 3 )

    if not os.path.exists( args.copy2 ) :
        print( f"copy2  '{args.copy2}' not found" )
        sys.exit( 3 )

    try :
        records( args.db ).write( data( args.accession[ 0 ], args.org, args.copy1, args.copy2 ) )
    except Exception as e :
        print( e )
    except KeyboardInterrupt :
        print( "^C" )
