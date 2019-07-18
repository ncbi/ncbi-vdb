#!/usr/bin/env python

from vdb import *

lib_wr = "./2.8.0/libncbi-wvdb.so.2.8.0"

schema_txt = '''
version 1;

table SUB_TAB #1.0
{
    column U32 C1;
    column ascii C2;
};

database SUB_DB #1.0
{
    table SUB_TAB #1 TAB20;
};

database MAIN_DB #1.0
{
    table SUB_TAB #1 TAB10;
    database SUB_DB #1 SUBDB;
};
''' 

def fill_table( tbl, row_count, value_count ) :
    try :
        cur = tbl.CreateCursor( OpenMode.Write )
        cols = cur.OpenColumns( [ "C1", "C2" ] )
        for idx in xrange( 0, row_count ) :
            cur.OpenRow()
            cols[ "C1" ].write_rand( value_count, 100 )
            cols[ "C2" ].write_rand( value_count, 80 )
            cur.CommitRow()
            cur.CloseRow()
        cur.Commit()
    except vdb_error as e :
        print( e )

def create_database( mgr, spec, path ) :
    #make a schema from the text above
    schema = mgr.MakeSchema( schema_txt )

    #create a database with this schema
    spec_in_schema = "MAIN_DB"
    path_to_create = "L5"
    db = mgr.CreateDB( schema, spec, path )
    
    t1 = db.CreateTable( "TAB10" )
    fill_table( t1, 2, 3 )
    
    subdb = db.CreateDB( "SUBDB" )
    t2 = subdb.CreateTable( "TAB20" )
    fill_table( t2, 2, 3 )

# ------------------------------------------------------------------------

def print_tables( db, prefix ) :
    try :
        tables = db.ListTbl()
        for tabname in tables :
            try :
                t = db.OpenTable( tabname )
                print( "%s|---TABLE: '%s'"%( prefix, t.Name() ) )
                t.print_rows( None, None, "%s    "%( prefix ) )
            except vdb_error as e :
                print( e )
    except vdb_error as e :
        pass

def print_databases( db, prefix ) :
    print_tables( db, prefix )
    try :
        databases = db.ListDB()
        for dbname in databases :
            try :
                d = db.OpenDB( dbname )
                print( "%s|---DATABASE: '%s'"%( prefix, d.Name() ) )
                print_databases( d, "%s    "%( prefix ) )
            except vdb_error as e :
                print( e )
    except vdb_error as e :
        pass

def print_db( db ) :
    print( "DATABASE: '%s':"%( db.Name() ) )
    print_databases( db, "" )

# ------------------------------------------------------------------------
        
if __name__ == '__main__' :
    
    try :
        #open a manager in read-mode
        mgr = manager( OpenMode.Write, lib_wr )
        
        #make a schema from the text above
        schema = mgr.MakeSchema( schema_txt )

        #create a database with this schema
        spec_in_schema = "MAIN_DB"
        path_to_create = "L5"
        create_database( mgr, spec_in_schema, path_to_create )
        
        #print the content of the created database
        print_db( mgr.OpenDB( path_to_create ) )
       
    except vdb_error as e :
        print( e )
