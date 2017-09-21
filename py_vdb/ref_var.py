#!/usr/bin/env python

from vdb import *
import multiprocessing, sys

'''
def f1_a( mgr, ref, del_pos, del_len, ins ) :
    del_pos = 2
    del_len = 1
    ins = "CCAA"
    
    print "ref    = '%s' del: %d:%d ins: '%s'" % ( ref, del_pos, del_len, ins )
    
    ref_var = mgr.make_ref_var( ref, del_pos, del_len, ins )
    
    #q  = ref_var.search()
    #qr = ref_var.search_len()
    #print "query  = '%s' at: %d:%d, on ref: %d" %( q[0], q[2], q[1], qr )
    
    ( a_bases, a_len, a_pos ) = ref_var.allele()
    ar = ref_var.allele_len()
    print "allele = '%s' at: %d:%d, on ref: %d" %( a_bases, a_pos, a_len, ar )

#------------------------------------------------------------------------------------------------------------
def f1( mgr ):
    ref = "ACCGGTTAACC"
    
    del_pos = 2
    del_len = 1
    ins = "CCAA"

    f1_a( mgr, ref, del_pos, del_len, ins )
    f1_a( mgr, ref, del_pos, del_len, ins )    

#------------------------------------------------------------------------------------------------------------
def f2( mgr, acc ) :
    refs = mgr.make_reflist( acc )
    count = refs.count()
    print "we have %d references:" % count
    for idx in xrange( count ) :
        print "\t No. %d" % idx
        obj = refs.get( idx )
        #print "\t\tidx:\t%d" % obj.get_idx()
        rr = obj.get_row_range()
        print "\t\trows:\t%d..%d" % ( rr[0], rr[1] )
        #print "\t\tbin:\t%d" % obj.get_bin()
        print "\t\tSeqId:\t%s" % obj.get_seq_id()
        print "\t\tname:\t%s" % obj.get_name()
        print "\t\tlength:\t%d" % obj.get_length()
        print "\t\tcirc:\t%s" % obj.is_circular()
        print "\t\text:\t%s" % obj.is_external()
        #print "\t\tdata:\t%s" % obj.read( 0, 50 )
        #print "\t\tid-cnt:\t%d" % obj.id_count( rr[ 0 ] )


#------------------------------------------------------------------------------------------------------------
def num( s ):
    try:
        return int( s )
    except ValueError:
        return 0


#------------------------------------------------------------------------------------------------------------
def split_cigar( cigar ) :
    res = list()
    op_len = ""
    for i in xrange( 0, len( cigar ) ) :
        op = cigar[ i ]
        if op >= '0' and op <= '9' :
            op_len = op_len + op
        else :
            tup = ( num( op_len ), op )
            op_len = ""
            res.append( tup )
    return res


#------------------------------------------------------------------------------------------------------------
def f3( mgr, acc ) :
    cur = mgr.open_db( acc ).open_tab( "PRIMARY_ALIGNMENT" ).make_cursor()
    cols = cur.open( [ "CIGAR_SHORT", "READ", "REF_SEQ_ID", "REF_POS", "REF_LEN" ] )
    row = read_row( cols, 1 )
    print row


#------------------------------------------------------------------------------------------------------------
def f4( mgr, acc ) :
    cur = mgr.open_db( acc ).open_tab( "PRIMARY_ALIGNMENT" ).make_cursor()
    cols = cur.open( [ "CIGAR_SHORT", "READ", "REF_SEQ_ID", "REF_POS", "REF_LEN" ] )
    row_range = cols[ "READ" ].range()
    print row_range
    for row in row_range :
        row_data = read_row( cols, row )
        if row % 1000 == 0 :
            sys.stdout.write( '.' )
            sys.stdout.flush()

#------------------------------------------------------------------------------------------------------------
def cigar_splitter( cigar, ref_pos, reference ) :
    ops = split_cigar( cigar )
    

#------------------------------------------------------------------------------------------------------------
def handle_reference( ref_obj, total, prim_alig_id_col, prim_cols, cigars ) :
    res = total
    ref_rows = ref_obj.get_row_range()
    ref_len = ref_obj.get_length()
    name = ref_obj.get_seq_id()
    #read the whole reference in...
    reference = ref_obj.read( 0, ref_len )
    print "\n", name, ref_len, len( reference )
    # for each row in the reference-table of this reference
    for ref_row in xrange( ref_rows[ 0 ], ref_rows[ 1 ] + 1 ) :
        prim_ids = prim_alig_id_col.read( ref_row )
        #for each alignment in this reference-block
        for prim_id in prim_ids :
            row_data = read_row( prim_cols, prim_id )
            cigar = row_data[ "CIGAR_SHORT" ]
            if cigar in cigars.keys() :
                cigars[ cigar ] += 1
            else :
                cigars[ cigar ] = 1
            res += 1
            if res % 1000 == 0 :
                sys.stdout.write( '.' )
                sys.stdout.flush()
    return res


  
#------------------------------------------------------------------------------------------------------------
def f5( mgr, acc, ref_idx = None ) :
    db = mgr.open_db( acc )
    cur_a = db.open_tab( "PRIMARY_ALIGNMENT" ).make_cursor()
    cur_r = db.open_tab( "REFERENCE" ).make_cursor()
    refs = db.make_reflist()
    prim_cols = cur_a.open( [ "CIGAR_SHORT", "READ", "REF_SEQ_ID", "REF_POS", "REF_LEN" ] )
    prim_alig_id_col = cur_r.open( "PRIMARY_ALIGNMENT_IDS" )
    total = 0
    cigars = {}
    
    if ref_idx == None :
    # for each reference
        for idx in xrange( refs.count() ) :
            total += handle_reference( refs.get( idx ), total, prim_alig_id_col, prim_cols, cigars )       
    else :
        total += handle_reference( refs.get( ref_idx ), total, prim_alig_id_col, prim_cols, cigars )       
    
    print "\nhandled ", total, " alignments"
    print "we have ", len( cigars ), " different cigar-strings"
    sorted_cigars = sorted( cigars, key = cigars.get, reverse = True )
    for w in sorted_cigars[ 0 : 10 ] :
        print w, cigars[ w ]

'''

#------------------------------------------------------------------------------------------------------------
def cigar2events( cigar ) :
    res = list()
    tmp = ""
    for c in cigar :
        if c >= '0' and c <= '9' :
            tmp += c
        else :
            res.append( ( int( tmp ), c ) )
            tmp = ""
    return res


#------------------------------------------------------------------------------------------------------------
def events2dict( events ) :
    res = {}
    for ( len, op ) in events :
        res[ op ] = res.get( op, 0 ) + 1
    return res


#------------------------------------------------------------------------------------------------------------
def adjacent( events, op1, op2 ) :
    res = 0
    last = 'x'
    for ( len, op ) in events :
        if op == op1 and last == op2 :
            res += 1
        if op == op2 and last == op1 :
            res += 1
        last = op
    return res


#------------------------------------------------------------------------------------------------------------
def cigar_events( cigar, read, refname, pos ) :
    ref_pos  = pos
    ali_pos = 0
    events = cigar2events( cigar )
    for ( len, op ) in events :
        if op == '=' :          # we have a perfect match between reference and alignment '='
            ref_pos += len      # we advance on alignment AND reference
            ali_pos += len

        elif op == 'X' :        #we have a mismatch  between reference and alignment 'X'
            yield ( refname, ref_pos, 0, read[ ali_pos : ali_pos + len ] )
            ref_pos += len      # we advance on alignment AND reference
            ali_pos += len

        elif op == 'D' :        #we have a deletion on the reference 'D'
            yield ( refname, ref_pos, len, '' )
            ref_pos += len      # we advance only on reference

        elif op == 'I' :        #we have a insertion on the reference 'I'
            yield ( refname, ref_pos, 0, read[ ali_pos : ali_pos + len ] )
            ali_pos += len      # we advance only on alignment
            
        elif op == 'S' :        #we have a soft clip 'S'
            ali_pos += len      # we advance only on alignment

        elif op == 'H' :        #we have a hard clip 'H'
            ali_pos += 0        # we do nothing

        elif op == 'N' :        #we have a reference 'N'
            ref_pos += len      # we advance on alignment AND reference

        else :
            ali_pos += 0        # we do nothing

#------------------------------------------------------------------------------------------------------------
# a generator of alleles...
def alleles( db, row_range = None ) :
    column_list = [ 'CIGAR_LONG', 'READ', 'REF_SEQ_ID', 'REF_POS', 'REF_LEN' ]
    prim_cols = db.open_tab( "PRIMARY_ALIGNMENT" ).make_cursor().open( column_list )
    for row in row_gen( prim_cols, row_range ) :
        cigar = row[ 'CIGAR_LONG' ]
        read = row[ 'READ' ]
        refname = row[ 'REF_SEQ_ID' ]
        refpos = row[ 'REF_POS' ][ 0 ]
        reflen = row[ 'REF_LEN' ][ 0 ]
        for c in cigar_events( cigar, read, refname, refpos, reflen ) :
            yield c
        print

#------------------------------------------------------------------------------------------------------------
def refvar_consumer( q, filename ) :
    print( "refvar_consumer() started" )
    d = {}
    while True :
        signature = q.get()
        if signature == None :
            break
        d[ signature ] = d.get( signature, 0 ) + 1

    f = open( filename, "w" )
    for k, v in sorted( [ ( value, key ) for ( key, value ) in d.items() ], reverse=True ) :
        f.write( "%d %s\n" %( k, v ) )
    f.close()
    print( "refvar_consumer() done" )


#------------------------------------------------------------------------------------------------------------
def allel_consumer( mgr, acc, q_in, q_out1, q_out2 ) :
    print( "allel_consumer() started" )
    try :
        ref_list = mgr.OpenDB( acc ).ReferenceList()
        curr_ref = None
        ref_bases = None
        while True :
            t = q_in.get()
            if t == None :
                break
            ( ref_name, ref_pos, del_len, bases ) = t
            if curr_ref == None or curr_ref != ref_name :
                curr_ref = ref_name
                try :
                    ref_obj = ref_list.find( ref_name )
                    ref_bases = ref_obj.Read( 0, ref_obj.SeqLength() )[:]
                except vdb_error as e :
                    print( e )

            sig1 = "%s:%d:%d:%s" % ( ref_name, ref_pos, del_len, bases )
            q_out1.put( sig1 )
            
            if len( bases ) > 0 :
                # we have insertion/mismatch : let's canonicalize it
                ref_var = mgr.RefVariation( ref_bases, ref_pos, del_len, bases )
                ( a_bases, a_len, a_pos ) = ref_var.GetAllele()
                sig2 = "%s:%d:%d:%s" % ( ref_name, a_pos, a_len, a_bases )
            else :
                # we have a pure deletion
                sig2 = sig1

            q_out2.put( sig2 )
    except vdb_error as e :
        print( e )
    q_out1.put( None )
    q_out2.put( None )
    print( "allel_consumer() done" )


#------------------------------------------------------------------------------------------------------------    
def row_consumer_allel_producer( q_in, q_out ) :
    print( "row_consumer() started" )
    while True :
        row = q_in.get()
        if row == None :
            break
        try :
            cigar = row[ 'CIGAR_LONG' ]
            read = row[ 'READ' ]
            refname = row[ 'REF_SEQ_ID' ]
            refpos = row[ 'REF_POS' ][ 0 ]
            for c in cigar_events( cigar, read, refname, refpos ) :
                q_out.put( c )
        except vdb_error as e :
            print( e )

    print( "row_consumer() done" )
    q_out.put( None )


#------------------------------------------------------------------------------------------------------------
def row_producer( mgr, acc, row_range, q ) :
    try :
        print( "row_producer() started" )
        db = mgr.OpenDB( acc )
        cols = [ 'CIGAR_LONG', 'READ', 'REF_SEQ_ID', 'REF_POS' ]
        prim_cols = db.OpenTable( "PRIMARY_ALIGNMENT" ).CreateCursor().OpenColumns( cols )
        for row in row_gen( prim_cols, row_range ) :
            q.put( row )
        q.put( None )
        print( "row_producer() done" )
    except vdb_error as e :
        print( e )


def process_accession( mgr, acc, row_range = None ) :
    row_q = multiprocessing.Queue()

    p_row_producer = multiprocessing.Process( target = row_producer, args = ( mgr, acc, row_range, row_q ), )

    allel_q = multiprocessing.Queue()
    
    p_row_cons = multiprocessing.Process( target = row_consumer_allel_producer, args = ( row_q, allel_q ), )    
    
    refvar_q1 = multiprocessing.Queue()
    refvar_q2 = multiprocessing.Queue()
    
    p_allel_cons = multiprocessing.Process( target = allel_consumer, args = ( mgr, acc, allel_q, refvar_q1, refvar_q2 ), )

    p_refvar1 = multiprocessing.Process( target = refvar_consumer, args = ( refvar_q1, "ref_var_1.txt" ), )
    p_refvar2 = multiprocessing.Process( target = refvar_consumer, args = ( refvar_q2, "ref_var_2.txt" ), )
    
    p_row_producer.start()
    p_row_cons.start()
    p_allel_cons.start()
    p_refvar1.start()
    p_refvar2.start()
    
    p_row_producer.join()    
    p_row_cons.join()
    p_allel_cons.join()
    p_refvar1.join()
    p_refvar2.join()


#------------------------------------------------------------------------------------------------------------
if __name__ == '__main__':
    lib_r = "./2.8.0/libncbi-vdb.so.2.8.0"
    ACC = "SRR1531793"
    
    if len( sys.argv ) > 1 :
        ACC = sys.argv[ 1 ]

    mgr = manager( OpenMode.Read, lib_r )
    process_accession( mgr, ACC )
    #f1( mgr )
