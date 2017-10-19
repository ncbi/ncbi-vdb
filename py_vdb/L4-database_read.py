#!/usr/bin/env python

from vdb import *

# ------------------------------------------------------------------------

#exam the database what sub-databases, sub-tables
def exam_db( db ) :
    try :
        print( "database \t: %s"% ( db.Name() ) )
    except vdb_error as e :
        pass
    try :
        print( "sub-db-s \t: %s"%( db.ListDB() ) )
    except vdb_error as e :
        pass

# ------------------------------------------------------------------------

def exam_tables( db ) :
    try :
        sub_tabs = db.ListTbl()
        print( "sub-tables's \t: %s"%sub_tabs )
        for t in sub_tabs :
            tab = db.OpenTable( t )
            print( "\n%s.columns = %s"%( tab.Name(), tab.ListCol() ) )
    except vdb_error as e :
        pass

# ------------------------------------------------------------------------

def exam_index( db ) :
    try :
        for tablename in db.ListTbl() :
            t = db.OpenTable( tablename )
            index_list = t.ListIdx()
            print( "\nINDEX %s.%s : %s"%( db.Name(), tablename, index_list ) )
            if len( index_list ) > 0 :
                for index_name in index_list :
                    index = t.OpenIndexRead( index_name )
                    print( "Version( '%s' ) = %s"%( index_name, index.Version() ) )
                    print( "Type( '%s' ) = %s"%( index_name, index.Type() ) )
                    print( "Locked( '%s' ) = %s"%( index_name, index.Locked() ) )
    except vdb_error as e :
        pass

# ------------------------------------------------------------------------

if __name__ == '__main__' :
    try :
        #open database in read-mode ( default )
        with manager().OpenDB( "SRR834507" ) as db :
            exam_db( db )
            exam_tables( db )
            exam_index( db )

    except vdb_error as e :
        print( e )
