from ctypes import *
from enum import Enum
import datetime, string, random, sys, os, platform

PY3 = sys.version_info[ 0 ] == 3

if PY3 :
    def xrange( *args, **kwargs ) :
        return iter( range( *args, **kwargs ) )

def to_bytes( s ) :
    if PY3 and ( type( s ) == str ) :
        return str.encode( s )
    return s

def to_char_p( s ) :
    if PY3  and ( type( s ) == str ) :
        return c_char_p( str.encode( s ) )
    return c_char_p( s )


class CreateMode( Enum ) :
    Open = 0
    Init = 1
    Create = 2
    MD5 = ( 1 << 6 )

class TypeDomain( Enum ) :
    Bool = 1
    UInt = 2
    Int = 3
    Float = 4
    Ascii = 5
    Unicode = 6
    
class CursorMode( Enum ) :
    Update = 0
    Replace = 1
    Insert = 2
    
class OpenMode( Enum ) :
    Read = 0
    Write = 1

class RepoCat( Enum ) :
    Bad = 0
    User = 1
    Site = 2
    Remote = 3
    
class RepoSubCat( Enum ) :
    Bad = 0
    Main = 1
    Aux = 2
    Protected = 3
    
class RemoteProto( Enum ) :
    Http = 0
    Fasp = 1
    FaspHttp = 2
    HttpFasp = 3
    
class ResolvEnable( Enum ) :
    UseConfig = 0
    AlwaysEnable = 1
    AlwaysDisable = 2

class PathType( Enum ) :
    NotFound = 0
    BadPath = 1
    File = 2
    Dir = 3
    CharDev = 4
    BlockDev = 5
    Fifo = 6
    ZombieFile = 7
    FakeRoot = 8
    Dataset = 9
    Datatype = 10
    Database = 11
    Table = 12
    Index = 13
    Column = 14,
    Metadata = 15
    PrereleaseTbl = 16
    Alias = 128

class IndexType( Enum ) :
    Text = 0
    UInt64 = 1
    Text_reverse = 128
    UInt64_reverse = 129

RepoCatDict = { RepoCat.Bad : "Bad", RepoCat.User : "User", RepoCat.Site : "Site", RepoCat.Remote : "Remote" }
def RepoCat2String( cat ) :
    x = RepoCatDict[ cat ]
    if x == None :
        x = "Bad"
    return x

RepoSubCatDict = { RepoSubCat.Bad : "Bad", RepoSubCat.Main : "Main", RepoSubCat.Aux : "Aux", RepoSubCat.Protected : "Protected" }
def RepoSubCat2String( cat ) :
    x = RepoSubCatDict[ cat ]
    if x == None :
        x = "Bad"
    return x

class vdb_string( Structure ) :
    _fields_ = [ ( "addr", c_char_p ), ( "size", c_int ), ( "len", c_int ) ]
    
    def __str__( self ) :
        return self.addr.value

class vdb_vector( Structure ) :
    _fields_ = [ ( "v", c_void_p ), ( "start", c_int ), ( "len", c_int ), ( "mask", c_int ) ]


#------------------------------------------------------------------------------------------------------------
class version :
    major = 0
    minor = 0
    release = 0
    
    def __init__( self, s ) :
        if PY3 :
            string_types = str
        else :
            string_types = basestring        
        if isinstance( s, string_types ) :
            a = s.split( '.' )
            l = len( a )
            if l > 0 :
                self.major = int( a[ 0 ] )
            if l > 1 :
                self.minor = int( a[ 1 ] )
            if l > 2 :
                self.release = int( a[ 2 ] )
        elif isinstance( s, int ) :
            self.major = ( s & 0xFF000000 ) >> 24
            self.minor = ( s & 0xFF0000 ) >> 16
            self.release = s & 0xFFFF
            
    def __str__( self ) :
        return "%d.%d.%d"%( self.major, self.minor, self.release )

    def __cmp__( self, other ) :
        if not isinstance( other, version ) :
            return NotImplemented
        d = cmp( self.major, other.major )
        if d != 0 :
            return d
        d = cmp( self.minor, other.minor )
        if d != 0 :
            return d
        return cmp( self.release, other.release )


#------------------------------------------------------------------------------------------------------------
class vdb_error( Exception ) :
    """Exception thrown by vdb-objects like mananger, schema, database, table, cursor, column
    
    Args:
        rc  (int)      :    rc-code from vdb-library call 
        msg ( string ) :    explanation of error
        obj            :    object that caused the error ( manager, schema, database, table, cursor, column )
    """
    def __init__( self, rc, msg, obj ) :
        super( vdb_error, self ).__init__( "%s.%s"%( obj.__class__.__name__, msg ) )
        self.obj_name = obj.__class__.__name__
        self.rc = rc


#------------------------------------------------------------------------------------------------------------
class KNamelist :
    def __init__( self, mgr, ptr ) :
        self.__mgr = mgr
        self.__ptr = ptr

    def __del__( self ) :
        rc = self.__mgr.KNamelistRelease( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KNamelistRelease()", self )

    def count( self ) :
        n = c_int()
        rc = self.__mgr.KNamelistCount( self.__ptr, byref( n ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KNamelistCount()", self )
        return n.value

    def to_list( self ) :
        res = list()
        for idx in xrange( 0, self.count() ) :
            name = c_char_p()
            rc = self.__mgr.KNamelistGet( self.__ptr, idx, byref( name ) )
            if rc != 0 :
                self.__mgr.raise_rc( rc, "KNamelistGet( %d )"%( idx ), self )
            if PY3 :
                res.append( name.value.decode( "utf-8" ) )
            else :
                res.append( name.value )
        return res


#------------------------------------------------------------------------------------------------------------
def random_string( size = 12, chars = string.ascii_uppercase + string.ascii_lowercase + string.digits ) :
    return ''.join( random.choice( chars ) for _ in xrange( size ) )

def random_data( count, min_value, max_value ) :
    res = list()
    for _ in xrange( count ) :
        res.append( random.randint( min_value, max_value ) )
    return res

#------------------------------------------------------------------------------------------------------------
class typedecl( Structure ) :
    _fields_ = [ ( "type_id", c_int ), ( "dim", c_int ) ]
    
    def __str__( self ) :
        return "( type_id=%d, dim=%d )"%( self.type_id, self.dim )
    
class typedesc( Structure ) :
    _fields_ = [ ( "bits", c_int ), ( "dim", c_int ), ( "domain", c_int ) ]

    def __str__( self ) :
        return "( bits=%d, dim=%d, domain=%d )"%( self.bits, self.dim, self.domain )


#------------------------------------------------------------------------------------------------------------
uint_xf  = { 8  : c_ubyte, 16 : c_ushort, 32 : c_uint, 64 : c_ulonglong }
int_xf   = { 8  : c_byte,  16 : c_short,  32 : c_int,  64 : c_longlong }
float_xf = { 32 : c_float, 64 : c_double }
txt_xf   = { 8  : c_char }

if platform.system() == "Windows" :
    type_xf = { TypeDomain.Bool     : ( uint_xf,  c_ubyte ),
                TypeDomain.UInt     : ( uint_xf,  c_ubyte ),
                TypeDomain.Int      : ( int_xf,   c_byte ),
                TypeDomain.Float    : ( float_xf, c_double ),
                TypeDomain.Ascii    : ( txt_xf,   c_char ),
                TypeDomain.Unicode  : ( txt_xf,   c_char ) }

else :
    type_xf = { TypeDomain.Bool.value     : ( uint_xf,  c_ubyte ),
                TypeDomain.UInt.value     : ( uint_xf,  c_ubyte ),
                TypeDomain.Int.value      : ( int_xf,   c_byte ),
                TypeDomain.Float.value    : ( float_xf, c_double ),
                TypeDomain.Ascii.value    : ( txt_xf,   c_char ),
                TypeDomain.Unicode.value  : ( txt_xf,   c_char ) }


#------------------------------------------------------------------------------------------------------------
class VColumn :
    """representing a column of a vdb-cursor
    """
    def __init__( self, mgr, cur, id, name, tabname ) :
        self.__mgr = mgr
        self.__cur = cur
        self.__id = id
        p1 = name.find( '(' )
        p2 = name.find( ')' )
        if p1 > -1 and p2 > -1 :
            self.cast = name[ p1 + 1 : p2 ]
            self.name = name[ p2 + 1 : ]
        else :
            self.cast = ""
            self.name = name
        self.tabname = tabname
        self.__tdec = typedecl( 0, 0 )
        self.__tdes = typedesc( 0, 0, 0 )
        self.column_type = None
        self.min_value = None
        self.max_value = None
        
    def __str__( self ) :
        return "%s.%s: (%d) %s %s"%( self.tabname, self.name, self.__id, self.tdec, self.tdes )

    def __enter__( self ) :
        return self

    def __exit__( self, type, value, traceback ) :
        pass

    def _update( self ) :
        rc = self.__mgr.VCursorDatatype( self.__cur._VCursor__ptr, self.__id, byref( self.__tdec ), byref( self.__tdes ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VCursorDatatype( '%s.%s' )"%( self.tabname, self.name ), self )
        ( dict, dflt ) = type_xf[ self.__tdes.domain ]
        self.column_type = dict[ self.__tdes.bits ]
        if self.column_type == None :
            self.column_type == dflt

    def domain( self ) :
        return TypeDomain( self.__tdes.domain )
        
    def bits( self ) :
        return self.__tdes.bits

    def dim( self ) :
        return self.__tdes.dim
        
    def Read( self, row ) :
        """read values from a column
        returns either a string or a list of integer, float, boolean values

        Args:
            row (longlong)  :  row to read from
        """
        if self.column_type == None :
            raise vdb_error( 0, "read: undefined column-type", self )
        row_id = c_longlong( row )
        data = c_void_p()
        row_len = c_int()
        rc = self.__mgr.VCursorCellDataDirect( self.__cur._VCursor__ptr, row_id, self.__id, None, byref( data ), None, byref( row_len ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VCursorCellDataDirect( '%s.%s', #%d )"%( self.tabname, self.name, row ), self )
        if self.column_type == c_char :
            return string_at( data, row_len.value )
        else :
            typed_ptr = cast( data, POINTER( self.column_type ) )
            l = list()
            for idx in xrange( 0, row_len.value ) :
                l.append( typed_ptr[ idx ] )        
            return l

    def __write_values( self, data ) :
        if isinstance( data, list ) :
            l = len( data )
            t = self.column_type * l
            arr = t()
            idx = 0
            for x in data :
                arr[ idx ] = x
                idx += 1
        else :
            l = 1
            t = self.column_type * l
            arr = t()
            arr[ 0 ] = data

        bits = c_int( self.__tdes.bits )
        rc = self.__mgr.VCursorWrite( self.__cur._VCursor__ptr, self.__id, bits, arr, c_int( 0 ), c_int( l ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VCursorWrite( %s.%s )"%( self.tabname, self.name ), self )

    def __write_string( self, data ) :
        p = create_string_buffer( to_bytes( data ) )
        rc = self.__mgr.VCursorWrite( self.__cur._VCursor__ptr, self.__id, c_int( 8 ), p, c_int( 0 ), c_int( len( data ) ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VCursorWrite( %s.%s, %s )"%( self.tabname, self.name, data ), self )

    def write( self, data ) :
        """write values to a column
        raises vdb_error if error occurs

        Args:
            data ( list of values or string ) :  data to be written
        """
        if self.column_type == None :
            raise vdb_error( 0, "write: undefined column-type", self )
        elif self.column_type == c_char :
            self.__write_string( data )
        else :
            self.__write_values( data )

    def write_rand( self, count = 1, max_value = 255, min_value = 0 ) :
        if self.column_type == None :
            raise vdb_error( 0, "write_rand: undefined column-type", self )
        else :
            dom = self.domain()
            if dom == TypeDomain.Ascii or dom == TypeDomain.Unicode :
                self.__write_string( random_string( count ) )
            elif dom == TypeDomain.Bool :
                self.__write_values( random_data( count, 0, 1 ) )
            else :
                self.__write_values( random_data( count, min_value, max_value ) )

    def set_default( self, data ) :
        if isinstance( data, list ) :
            l = len( data )
            t = self.column_type * l
            arr = t()
            idx = 0
            for x in data :
                arr[ idx ] = x
                idx += 1
        else :
            l = 1
            t = self.column_type * l
            arr = t()
            arr[ 0 ] = data
        
        bits = c_int( self.__tdes.bits )
        rc = self.__mgr.VCursorDefault( self.__cur._VCursor__ptr, self.__id, bits, arr, c_int( 0 ), c_int( l ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VCursorDefault( %s.%s )"%( self.tabname, self.name ), self )

    def default_string( self, data ) :
        p = create_string_buffer( data )
        rc = self.__mgr.VCursorDefault( self.__cur._VCursor__ptr, self.__id, c_int( 8 ), p, c_int( 0 ), c_int( len( data ) ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VCursorDefault( %s.%s, %s )"%( self.tabname, self.name, data ), self )

    def default( self, data ) :
        if self.column_type == None :
            raise vdb_error( 0, "default: undefined column-type", self )
        if self.column_type == c_char :
            return self.default_string( data )
        else :
            return self.default_values( data )

    def row_range( self ) :
        first = c_longlong()
        count = c_longlong()
        rc = self.__mgr.VCursorIdRange( self.__cur._VCursor__ptr, self.__id, byref( first ), byref( count ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VCursorIdRange( '%s.%s' )"%( self.tabname, self.name ), self )
        return ( first.value, count.value )

    def range( self ) :
        ( first, count ) = self.row_range()
        return xrange( first, first + count )

    def next_row( self, current_row ) :
        res = c_longlong( 0 )
        rc = self.__mgr.VCursorFindNextRowIdDirect( self.__cur._VCursor__ptr, self.__id, c_longlong( current_row ), byref( res ) ) 
        if rc != 0 :
            return None
        else :
            return res.value
    

#------------------------------------------------------------------------------------------------------------
class ReferenceObj :
    def __init__( self, mgr, ptr ) :
        self.__mgr = mgr
        self.__ptr = ptr

    def __del__( self ) :
        self.__mgr.ReferenceObj_Release( self.__ptr )

    def __str__( self ) :
        idx = self.Idx()
        start, stop = self.IdRange()
        seq_id = self.SeqId()
        name = self.Name()
        return "Idx\t%d\nIdRange\t%d..%d\nSeqId\t%s\nName\t%s"%( idx, start, stop, seq_id, name )

    def Idx( self ) :
        res = c_int()
        rc = self.__mgr.ReferenceObj_Idx( self.__ptr, byref( res ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "ReferenceObj_Idx()", self )
        return res.value
        
    def IdRange( self ) :
        start = c_longlong()
        stop = c_longlong()
        rc = self.__mgr.ReferenceObj_IdRange( self.__ptr, byref( start ), byref( stop ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "ReferenceObj_IdRange()", self )
        return ( start.value, stop.value )
        
    def Bin( self ) :
        res = c_int()
        rc = self.__mgr.ReferenceObj_Bin( self.__ptr, byref( res ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "ReferenceObj_Bin()", self )
        return res.value

    def SeqId( self ) :
        res = c_char_p()
        rc = self.__mgr.ReferenceObj_SeqId( self.__ptr, byref( res ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "ReferenceObj_SeqId()", self )
        return res.value
        
    def Name( self ) :
        res = c_char_p()
        rc = self.__mgr.ReferenceObj_Name( self.__ptr, byref( res ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "ReferenceObj_Name()", self )
        return res.value

    def SeqLength( self ) :
        res = c_int()
        rc = self.__mgr.ReferenceObj_SeqLength( self.__ptr, byref( res ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "ReferenceObj_SeqLength()", self )
        return res.value

    def Circular( self ) :
        res = c_bool()
        rc = self.__mgr.ReferenceObj_Circular( self.__ptr, byref( res ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "ReferenceObj_Circular()", self )
        return res.value
        
    def External( self ) :
        res = c_bool()
        rc = self.__mgr.ReferenceObj_External( self.__ptr, byref( res ), c_void_p( 0 ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "ReferenceObj_External()", self )
        return res.value

    def Read( self, offs, len ) :
        buffer = create_string_buffer( len )
        written = c_int()
        rc = self.__mgr.ReferenceObj_Read( self.__ptr, c_int( offs ), c_int( len ), buffer, byref( written ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "ReferenceObj_Read( %d.%d )" % ( offs, len ), self )
        if PY3 :
            return buffer.value.decode( "utf-8" )
        return buffer.value
        
    def GetIdCount( self, row_id ) :
        res = c_int()
        rc = self.__mgr.ReferenceObj_GetIdCount( self.__ptr, c_longlong( row_id ), byref( res ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "ReferenceObj_GetIdCount( %d )" % row_id, self )
        return res.value


#------------------------------------------------------------------------------------------------------------
class ReferenceList :
    def __init__( self, mgr, ptr ) :
        self.__mgr = mgr
        self.__ptr = ptr

    def __del__( self ) :
        rc = self.__mgr.ReferenceList_Release( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "ReferenceList_Release()", self )

    def count( self ) :
        res = c_int()
        rc = self.__mgr.ReferenceList_Count( self.__ptr, byref( res ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "ReferenceList_Count()", self )
        return res.value

    def find( self, name ) :
        ptr = c_void_p()
        rc = self.__mgr.ReferenceList_Find( self.__ptr, byref( ptr ), to_char_p( name ), c_int( len( name ) ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "ReferenceList_Find( '%s' )" % name, self )
        return ReferenceObj( self.__mgr, ptr )
        
    def get( self, idx ) :
        ptr = c_void_p()
        rc = self.mgr.ReferenceList_Get( self.__ptr, byref( ptr ), c_int( idx ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "ReferenceList_Get( %d )" % idx, self )
        return ReferenceObj( self.__mgr, ptr )


#------------------------------------------------------------------------------------------------------------
def read_row( cols, row_id ) :
    res = {}
    try :
        for ( name, column ) in cols.iteritems() :
            res[ name ] = column.Read( row_id )
    except AttributeError :
        for ( name, column ) in cols.items() :
            res[ name ] = column.Read( row_id )
    return res


#------------------------------------------------------------------------------------------------------------
def first_none_static_col( cols ) :
    res = None
    try :
        v = cols.itervalues()
    except AttributeError :
        v = cols.values()
    for c in v :
        rr = c.row_range()
        if res == None and rr[ 1 ] > 0 :
            res = c
    return res
    

#------------------------------------------------------------------------------------------------------------
def row_gen( cols, row_range = None ) :
    if isinstance( cols, dict ) :
        first_col = first_none_static_col( cols )
        if first_col != None :
            if row_range == None :
                row_id = 1
                while row_id != None :
                    yield read_row( cols, row_id )
                    row_id = first_col.next_row( row_id + 1 )
            else :
                range_idx = 0
                try :
                    row_id = row_range[ range_idx ]
                except :
                    row_id = None
                while row_id != None :
                    yield read_row( cols, row_id )
                    range_idx += 1
                    try :
                        row_id = first_col.next_row( row_range[ range_idx ] )
                    except :
                        row_id = None
    else :
        if row_range == None :
            row_id = 1
            while row_id != None :
                yield cols.Read( row_id )
                row_id = cols.next_row( row_id + 1 )
        else :
            range_idx = 0
            try :
                row_id = row_range[ range_idx ]
            except :
                row_id = None
            while row_id != None :
                yield cols.Read( row_id )
                range_idx += 1
                try :
                    row_id = cols.next_row( row_range[ range_idx ] )
                except :
                    row_id = None


#------------------------------------------------------------------------------------------------------------
class VCursor :
    def __init__( self, tab, ptr ) :
        self.__mgr = tab._VTable__mgr
        self.__tab = tab
        self.__ptr = ptr

    def __del__( self ) :
        rc = self.__mgr.VCursorRelease( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VCursorRelease( '%s' )"%( self.__tab.name ), self )

    def __enter__( self ) :
        return self

    def __exit__( self, type, value, traceback ) :
        pass

    def OpenColumns( self, col_names ) :
        if isinstance( col_names, list ) :
            res = {}
            for name in col_names :
                res[ name ] = self.AddColumn( name )
            self.Open()
            try :
                v = res.itervalues()
            except AttributeError :
                v = res.values()
            for c in v :
                c._update()
            return res
        elif isinstance( col_names, str ) :
            c = self.AddColumn( col_names )
            self.Open()
            c._update()
            return c
        else :
            raise vdb_error( 0, "cursor.open( x ) x is not list or string", self )
            
    def AddColumn( self, name ) :
        idx = c_int()
        rc = self.__mgr.VCursorAddColumn( self.__ptr, byref( idx ), to_char_p( name ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VCursorAddColumn( %s.%s )"%( self.__tab.name, name ), self )
        return VColumn( self.__mgr, self, idx.value, name, self.__tab._VTable__name )
        
    def Open( self ) :
        rc = self.__mgr.VCursorOpen( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VCursorOpen( '%s' )"%( self.__tab.name ), self )

    def Commit( self ) :
        rc = self.__mgr.VCursorCommit( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VCursorCommit( %s )"%( self.__tab.name ), self )

    def OpenRow( self ) :
        rc = self.__mgr.VCursorOpenRow( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VCursorOpenRow( '%s' )"%( self.__tab.name ), self )
        
    def CommitRow( self ) :
        rc = self.__mgr.VCursorCommitRow( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VCursorCommitRow( %s )"%( self.__tab.name ), self )

    def RepeatRow( self, count ) :
        rc = self.__mgr.VCursorRepeatRow( self.__ptr, count )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VCursorRepeatRow( %s, %d )"%( self.__tab.name, count ), self )

    def CloseRow( self ) :
        rc = self.__mgr.VCursorCloseRow( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VCursorCloseRow( '%s' )"%( self.__tab.name ), self )

    def FlushPage( self ) :
        rc = self.__mgr.VCursorFlushPage( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VCursorFlushPage( %s )"%( self.__tab.name ), self )

    def RowId( self ) :
        row_id = c_longlong()
        rc = self.__mgr.VCursorRowId( self.__ptr, byref( row_id ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VCursorRowId( '%s' )"%( self.__tab.name ), self )
        return row_id.value

    def ReferenceList( self ) :
        reflist_ptr = c_void_p()
        rc = self.__mgr.ReferenceList_MakeCursor( byref( reflist_ptr ), self.__ptr, c_int( 0 ), c_char_p( 0 ), c_int( 0 ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "ReferenceList_MakeCursor()", self )
        return ReferenceList( self.__mgr, reflist_ptr )
    

def max_colname_len( cols, colnames ) :
    res = 0
    if colnames == None :
        try :
            for ( name, column ) in cols.iteritems() :
                l = len( name )
                if l > res :
                    res = l
        except AttributeError :
            for ( name, column ) in cols.items() :
                l = len( name )
                if l > res :
                    res = l
    else :
        for name in colnames :
            l = len( name )
            if l > res :
                res = l
    return res


#------------------------------------------------------------------------------------------------------------
# cols ....... dictionay : key = colname, value = column-object
# rowrange ... a xrange - iterator ( None: all rows )
# colnames ... a list of string ( None: all columns )
# prefix ..... a string printed at the beginning of each line
def print_cols( cols, rowrange, colnames = None, prefix = "" ) :
    if cols != None :
        if rowrange == None :
            try :
                column = cols.itervalues().next()
            except AttributeError :
                column = list( cols.values() )[ 0 ]
            rr = column.range()
        else :
            rr = rowrange
        w = max_colname_len( cols, colnames )
        for row_id in rr :
            if colnames == None :
                try :
                    for ( name, column ) in sorted( cols.iteritems() ) :
                        print( '{0}{1:<{width}}.{2} : {3}'.format( prefix, name, row_id, column.Read( row_id ), width = w ) )
                except AttributeError :
                    for ( name, column ) in sorted( cols.items() ) :
                        print( '{0}{1:<{width}}.{2} : {3}'.format( prefix, name, row_id, column.Read( row_id ), width = w ) )
            else :
                for name in colnames :
                    column = cols[ name ]
                    if column != None :
                        print( '{0}{1:<{width}}.{2} : {3}'.format( prefix, name, row_id, column.Read( row_id ), width = w ) )
            print( "%s."%( prefix ) )
 

#------------------------------------------------------------------------------------------------------------
class KIndex :
    def __init__( self, mgr, ptr, name ) :
        self.__mgr = mgr
        self.__ptr = ptr
        self.__name = name

    def __del__( self ) :
        rc = self.__mgr.KIndexRelease( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KIndexRelease( '%s' )"%( self.__name ), self )
        
    def Version( self ) :
        vers = c_int()
        rc = self.__mgr.KIndexVersion( self.__ptr, byref( vers ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KIndexVersion( '%s' )"%( self.__name ), self )
        return version( vers.value )

    def Type( self ) :
        type = c_int()
        rc = self.__mgr.KIndexType( self.__ptr, byref( type ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KIndexType( '%s' )"%( self.__name ), self )
        return IndexType( type.value )
 
    def Locked( self ) :
        return self.__mgr.KIndexLocked( self.__ptr )

    def FindText( self, key ) :
        start_id = c_longlong()
        id_count = c_longlong()
        rc = self.__mgr.KIndexFindText( self.__ptr, to_char_p( key ), byref( start_id ), by_ref( id_count ), c_void_p( 0 ), c_void_p( 0 ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KIndexFindText( %s, %s )"%( self.__name, key ), self )
        return ( start_id.value, id_count.value )

    def ProjectText( self, row_id, bufsize = 1024 ) :
        buffer = create_string_buffer( bufsize )
        start_id = c_longlong()
        id_count = c_longlong()
        num_writ = c_int( 0 )
        rc = self.__mgr.KIndexProjectText( self.__ptr, c_longlong( row_id ), byref( start_id ), byref( id_count ), buffer, c_int( bufsize ), byref( num_writ ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KIndexProjectText( %s, %d )"%( self.__name, row_id ), self )
        if PY3 :            
            return ( buffer.value.decode( "utf-8" ), start_id.value, id_count.value )
        return ( buffer.value, start_id.value, id_count.value )
        
    def Commit( self ) :
        rc = self.__mgr.KIndexCommit( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KIndexCommit( %s )"%( self.__name ), self )

    def InsertText( self, unique, key, value ) :
        rc = self.__mgr.KIndexInsertText( self.__ptr, c_bool( unique ), to_char_p( key ), c_longlong( value ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KIndexInsertText( %s, k=%s, v=%d )"%( self.__name, key, value ), self )

    def DeleteText( self, key ) :
        rc = self.__mgr.KIndexDeleteText( self.__ptr, to_char_p( key ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KIndexDeleteText( %s, k=%s )"%( self.__name, key ), self )

    def InsertU64( self, unique, key_start, keylen, value_start, value_len ) :
        rc = self.__mgr.KIndexInsertU64( self.__ptr, c_bool( unique ), c_longlong( key_start ), c_longlong( key_len ), c_longlong( value_start ), c_longlong( value_len ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KIndexInsertU64( %s, k=%d.%d, v=%d.%d )"%( self.__name, key_start, key_len, value_start, value_len ), self )

    def DeleteU64( self, key ) :
        rc = self.__mgr.KIndexDeleteU64( self.__ptr, c_longlong( key ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KIndexDeleteU64( %s, k=%d )"%( self.__name, key ), self )

            
#------------------------------------------------------------------------------------------------------------
class KMDataNode :
    def __init__( self, mgr, ptr ) :
        self.__mgr = mgr
        self.__ptr = ptr

    def __del__( self ) :
        rc = self.__mgr.KMDataNodeRelease( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KMDataNodeRelease()", self )

    def __enter__( self ) :
        return self

    def __exit__( self, type, value, traceback ) :
        pass

    def OpenNode( self, path, open_mode = OpenMode.Read ) :
        node = c_void_p()
        if open_mode == OpenMode.Write :
            rc = self.__mgr.KMDataNodeOpenNodeUpdate( self.__ptr, byref( node ), to_char_p( path ) )
            if rc != 0 :
                self.__mgr.raise_rc( rc, "KMDataNodeOpenNodeUpdate( %s )"%( path ), self )
        else :
            rc = self.__mgr.KMDataNodeOpenNodeRead( self.__ptr, byref( node ), to_char_p( path ) )
            if rc != 0 :
                self.__mgr.raise_rc( rc, "KMDataNodeOpenNodeRead( %s )"%( path ), self )
        return KMDataNode( self.__mgr, node )

    def ByteOrder( self ) :
        order = c_bool()
        rc = self.__mgr.KMDataNodeByteOrder( self.__ptr, byref( order ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KMDataNodeByteOrder()", self )
        return order.value

    def ListChildren( self ) :
        l = c_void_p()
        rc = self.__mgr.KMDataNodeListChildren( self.__ptr, byref( l ) )
        if rc == 0 :
            return KNamelist( self.__mgr, l ).to_list()
        return []

    def ListAttr( self ) :
        l = c_void_p()
        rc = self.__mgr.KMDataNodeListAttr( self.__ptr, byref( l ) )
        if rc == 0 :
            return KNamelist( self.__mgr, l ).to_list()
        return []

    def size( self ) :
        buffer = create_string_buffer( 16 )
        num_read = c_size_t()
        remaining = c_size_t()
        rc = self.__mgr.KMDataNodeRead( self.__ptr, c_size_t( 0 ), buffer, c_size_t( 16 ), byref( num_read ), byref( remaining ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KMDataNodeRead()", self )
        return num_read.value + remaining.value

    def to_read( self, req_len ) :
        ns = self.size() 
        if req_len > 0 :
            return min( req_len, ns )
        return ns
        
    def Read( self, buffer_size, to_read, offset ) :
        buffer = create_string_buffer( int( buffer_size ) )
        num_read = c_size_t()
        remaining = c_size_t()
        rc = self.__mgr.KMDataNodeRead( self.__ptr, c_size_t( offset ), buffer, c_size_t( to_read ), byref( num_read ), byref( remaining ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KMDataNodeRead( offset = %d )"%( offset ), self )
        if PY3 :
            return buffer.value.decode( "utf-8" )
        return buffer.value

    def as_string( self, req_len = 0, offset = 0 ) :
        n_bytes = self.to_read( req_len )
        if n_bytes < 1 :
            return ""
        return self.Read( n_bytes, n_bytes, offset )

    def read_buffer_as( self, t = c_ubyte, align = 1, req_len = 0, offset = 0 ) :
        n_bytes = self.to_read( req_len )
        if n_bytes < 1 :
            return []
        n_values = n_bytes
        if align > 1 :
            n_values = n_bytes / align
            if n_bytes > ( n_values * align ) :
                n_values += 1
        buffer = self.Read( n_values * align, n_bytes, offset )
        ptr = cast( buffer, POINTER( t ) )
        l = list()
        for idx in xrange( 0, int( n_values ) ) :
            l.append( ptr[ idx ] )
        return l

    def as_uint8( self, req_len = 0, offset = 0 ) :
        return self.read_buffer_as( c_ubyte, 1, req_len, offset )
        
    def as_int8( self, req_len = 0, offset = 0 ) :
        return self.read_buffer_as( c_byte, 1, req_len, offset )    

    def as_uint16( self, req_len = 0, offset = 0 ) :
        return self.read_buffer_as( c_ushort, 2, req_len, offset )

    def as_int16( self, req_len = 0, offset = 0 ) :
        return self.read_buffer_as( c_short, 2, req_len, offset )

    def as_uint32( self, req_len = 0, offset = 0 ) :
        return self.read_buffer_as( c_uint, 4, req_len, offset )

    def as_int32( self, req_len = 0, offset = 0 ) :
        return self.read_buffer_as( c_int, 4, req_len, offset )

    def as_uint64( self, req_len = 0, offset = 0 ) :
        return self.read_buffer_as( c_ulonglong, 8, req_len, offset )

    def as_int64( self, req_len = 0, offset = 0 ) :
        return self.read_buffer_as( c_longlong, 8, req_len, offset )

    def write_string( self, data, append = False ) :
        p = create_string_buffer( data )
        if append :
            rc = self.__mgr.KMDataNodeAppend( self.__ptr, p, c_size_t( len( data ) ) )
            if rc != 0 :
                self.__mgr.raise_rc( rc, "KMDataNodeAppend()", self )
        else :
            rc = self.__mgr.KMDataNodeWrite( self.__ptr, p, c_size_t( len( data ) ) )
            if rc != 0 :
                self.__mgr.raise_rc( rc, "KMDataNodeWrite()", self )

    def write_values( self, data, ct, ct_size, append ) :
        if isinstance( data, list ) :
            l = len( data )
            t = ct * l
            arr = t()
            idx = 0
            for x in data :
                arr[ idx ] = x
                idx += 1
        else :
            l = 1
            t = ct * l
            arr = t()
            arr[ 0 ] = data
        if append :
            rc = self.__mgr.KMDataNodeAppend( self.__ptr, arr, c_size_t( l * ct_size ) )
            if rc != 0 :
                self.__mgr.raise_rc( rc, "KMDataNodeAppend()", self )
        else :
            rc = self.__mgr.KMDataNodeWrite( self.__ptr, arr, c_size_t( l * ct_size ) )
            if rc != 0 :
                self.__mgr.raise_rc( rc, "KMDataNodeWrite()", self )

    def write_uint8( self, data, append = False ) :
        self.write_values( data, c_ubyte, 1, append )

    def write_int8( self, data, append = False ) :
        self.write_values( data, c_byte, 1, append )

    def write_uint16( self, data, append = False ) :
        self.write_values( data, c_ushort, 2, append )

    def write_int16( self, data, append = False ) :
        self.write_values( data, c_short, 2, append )

    def write_uint32( self, data, append = False ) :
        self.write_values( data, c_uint, 4, append )

    def write_int32( self, data, append = False ) :
        self.write_values( data, c_int, 4, append )

    def write_uint64( self, data, append = False ) :
        self.write_values( data, c_ulonglong, 8, append )

    def write_int64( self, data, append = False ) :
        self.write_values( data, c_longlong, 8, append )


#------------------------------------------------------------------------------------------------------------
class KMetadata :
    def __init__( self, mgr, ptr ) :
        self.__mgr = mgr
        self.__ptr = ptr

    def __del__( self ) :
        rc = self.__mgr.KMetadataRelease( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KMetadataRelease()", self )

    def __enter__( self ) :
        return self

    def __exit__( self, type, value, traceback ) :
        pass

    def Version( self ) :
        vers = c_int()
        rc = self.__mgr.KMetadataVersion( self.__ptr, byref( vers ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KMetadataVersion()", self )
        return version( vers.value )

    def ByteOrder( self ) :
        order = c_bool()
        rc = self.__mgr.KMetadataByteOrder( self.__ptr, byref( order ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KMetadataByteOrder()", self )
        return order.value

    def Revision( self ) :
        rev = c_int()
        rc = self.__mgr.KMetadataRevision( self.__ptr, byref( rev ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KMetadataRevision()", self )
        return rev.value
        
    def MaxRevision( self ) :
        rev = c_int()
        rc = self.__mgr.KMetadataMaxRevision( self.__ptr, byref( rev ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KMetadataMaxRevision()", self )
        return rev.value

    def Commit( self ) :
        rc = self.__mgr.KMetadataCommit( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KMetadataCommit()", self )

    def Freeze( self ) :
        rc = self.__mgr.KMetadataFreeze( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KMetadataFreeze()", self )

    def OpenRevision( self, rev_nr ) :
        k_meta = c_void_p()
        rc = self.__mgr.KMetadataOpenRevision( self.__ptr, byref( k_meta ), c_int( rev_nr ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KMetadataOpenRevision( %d )"%rev_nr, self )
        return KMetadata( self.__mgr, k_meta )
        
    def GetSequence( self, name ) :
        value = c_longlong()
        rc = self.__mgr.KMetadataGetSequence( self.__ptr, to_char_p( name ), byref( value ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KMetadataGetSequence( %s )"%name, self )
        return value.value
        
    def SetSequence( self, name, value ) :
        rc = self.__mgr.KMetadataSetSequence( self.__ptr, to_char_p( name ), c_longlong( value ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KMetadataSetSequence( %s, %d )"%( name, value ), self )

    def NextSequence( self, name ) :
        value = c_longlong()
        rc = self.__mgr.KMetadataNextSequence( self.__ptr, to_char_p( name ), byref( value ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KMetadataNextSequence( %s )"%( name ), self )
        return value.value

    def OpenNode( self, path, open_mode = OpenMode.Read ) :
        ptr = c_void_p()
        if open_mode == OpenMode.Write :
            rc = self.__mgr.KMetadataOpenNodeUpdate( self.__ptr, byref( ptr ), to_char_p( path ) )
            if rc != 0 :
                self.__mgr.raise_rc( rc, "KMetadataOpenNodeUpdate( %s )"%( path ), self )
        else :
            rc = self.__mgr.KMetadataOpenNodeRead( self.__ptr, byref( ptr ), to_char_p( path ) )
            if rc != 0 :
                self.__mgr.raise_rc( rc, "KMetadataOpenNodeRead( %s )"%( path ), self )
        return KMDataNode( self.__mgr, ptr )


#------------------------------------------------------------------------------------------------------------
class VTable :
    def __init__( self, mgr, ptr, name ) :
        self.__mgr = mgr
        self.__ptr = ptr
        self.__kdb_ptr = None
        self.__name = name
        self.p_cur = None
        self.p_cols = None
        
    def __del__( self ) :
        rc = self.__mgr.VTableRelease( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VTableRelease( %s )"%( self.__name ), self )
        if self.__kdb_ptr != None :
            rc = self.__mgr.KTableRelease( self.__kdb_ptr )
            if rc != 0 :
                self.__mgr.raise_rc( rc, "KTableRelease( %s )"%( self.__name ), self )

    def __enter__( self ) :
        return self

    def __exit__( self, type, value, traceback ) :
        pass

    def Name( self ) :
        return self.__name

    def ListCol( self ) :
        l = c_void_p()
        rc = self.__mgr.VTableListCol( self.__ptr, byref( l ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VTableListCol( %s )"%( self.__name ), self )
        return KNamelist( self.__mgr, l ).to_list()

    def __OpenKTableRead__( self ) :
        k_table = c_void_p()
        rc = self.__mgr.VTableOpenKTableRead( self.__ptr, byref( k_table ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VTableOpenKTableRead( '%s' )"%( self.__name ), self )
        self.__kdb_ptr = k_table

    def __OpenKTableUpdate__( self ) :
        k_table = c_void_p()
        rc = self.__mgr.VTableOpenKTableUpdate( self.__ptr, byref( k_table ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VTableOpenKTableUpdate( '%s' )"%( self.__name ), self )
        self.__kdb_ptr = k_table

    def ListIdx( self ) :
        if self.__kdb_ptr == None :
            self.__OpenKTableRead__()
        l = c_void_p()
        rc = self.__mgr.KTableListIdx( self.__kdb_ptr, byref( l ) )
        if rc == 0 :
            return KNamelist( self.__mgr, l ).to_list()
        return []
        
    def OpenIndexRead( self, name ) :
        if self.__kdb_ptr == None :
            self.__OpenKTableUpdate__()
        k_idx = c_void_p()
        rc = self.__mgr.KTableOpenIndexRead( self.__kdb_ptr, byref( k_idx ), to_char_p( name ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KTableOpenIndexRead( '%s' )"%( self.__name ), self )
        return KIndex( self.__mgr, k_idx, name )

    def CreateIndex( self, name, idx_type = IndexType.Text, create_mode = CreateMode.Init ) :
        if self.__kdb_ptr == None :
            self.__OpenKTableUpdate__()
        k_idx = c_void_p()
        rc = self.__mgr.KTableCreateIndex( self.__kdb_ptr, byref( k_idx ), c_int( idx_type.value ), c_int( create_mode.value ), to_char_p( name ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KTableCreateIndex( '%s' )"%( self.__name ), self )
        return KIndex( self.__mgr, k_idx, name )

    def OpenMetadata( self, open_mode = OpenMode.Read ) :
        if self.__kdb_ptr == None :
            self.__OpenKTableRead__()
        k_meta = c_void_p()
        if open_mode == OpenMode.Write :
            rc = self.__mgr.KTableOpenMetadataUpdate( self.__kdb_ptr, byref( k_meta ) )
            if rc != 0 :
                self.__mgr.raise_rc( rc, "KTableOpenMetadataUpdate( '%s' )"%( self.__name ), self )
        else :
            rc = self.__mgr.KTableOpenMetadataRead( self.__kdb_ptr, byref( k_meta ) )
            if rc != 0 :
                self.__mgr.raise_rc( rc, "KTableOpenMetadataRead( '%s' )"%( self.__name ), self )
        return KMetadata( self.__mgr, k_meta )

    def CreateCursor( self, open_mode = OpenMode.Read, ins_mode = CursorMode.Insert ) :
        c = c_void_p()
        if open_mode == OpenMode.Write :
            rc = self.__mgr.VTableCreateCursorWrite( self.__ptr, byref( c ), c_int( ins_mode.value ) )
            if rc != 0 :
                self.__mgr.raise_rc( rc, "VTableCreateCursorWrite( %s )"%( self.__name ), self )
        else :
            rc = self.__mgr.VTableCreateCursorRead( self.__ptr, byref( c ) )
            if rc != 0 :
                self.__mgr.raise_rc( rc, "VTableCreateCursorRead( %s )"%( self.__name ), self )
        return VCursor( self, c )

    def CreateCachedCursorRead( self, cache_size ) :
        c = c_void_p()
        rc = self.__mgr.VTableCreateCachedCursorRead( self.__ptr, byref( c ), c_longlong( cache_size ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VTableCreateCachedCursorRead( %s, %d )"%( self.__name, cache_size ), self )
        return VCursor( self, c )
        
    def Locked( self ) :
        return self.__mgr.VTableLocked( self.__ptr )

    def ReferenceList( self, options = 2 ) :
        ptr = c_void_p()
        rc = self.__mgr.ReferenceList_MakeTable( byref( ptr ), self.__ptr, c_int( options ), c_int( 0 ), c_char_p( 0 ), c_int( 0 ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "ReferenceList_MakeTable( '%s' )"%( self.__name ), self )
        return ReferenceList( self.__mgr, ptr )

    def print_rows( self, colrange = None, colnames = None, prefix = "" ) :
        if self.p_cur == None :
            self.p_cur = self.CreateCursor()
        if self.p_cur != None :
            if self.p_cols == None :
                if colnames == None :
                    self.p_cols = self.p_cur.OpenColumns( self.ListCol() )
                else :
                    self.p_cols = self.p_cur.OpenColumns( colnames )
            elif colnames != None :
                more_to_open = 0
                for name in colnames :
                    if self.p_cols[ name ] == None :
                        more_to_open += 1
                if more_to_open > 0 :
                    self.p_cur = None
                    self.p_cols = None
                    self.p_cur = self.CreateCursor()
                    if self.p_cur != None :
                        self.p_cols = self.p_cur.OpenColumns( colnames )
            if self.p_cols != None :
                print_cols( self.p_cols, colrange, colnames, prefix )


#------------------------------------------------------------------------------------------------------------
class VDatabase :
    def __init__( self, mgr, ptr, name ) :
        self.__mgr = mgr
        self.__ptr = ptr
        self.__name = name

    def __del__( self ) :
        rc = self.__mgr.VDatabaseRelease( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VDatabaseRelease( %s )"%( self.__name ), self )
        
    def __enter__( self ) :
        return self

    def __exit__( self, type, value, traceback ) :
        pass

    def Name( self ) :
        return self.__name

    def ListTbl( self ) :
        l = c_void_p()
        rc = self.__mgr.VDatabaseListTbl( self.__ptr, byref( l ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VDatabaseListTbl( %s )"%( self.__name ), self )
        return KNamelist( self.__mgr, l ).to_list()

    def ListDB( self ) :
        l = c_void_p()
        rc = self.__mgr.VDatabaseListDB( self.__ptr, byref( l ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VDatabaseListDB( %s )"%( self.__name ), self )
        return KNamelist( self.__mgr, l ).to_list()

    def OpenTable( self, name, mode = OpenMode.Read ) :
        f = self.__mgr.VDatabaseOpenTableUpdate if mode == OpenMode.Write else self.__mgr.VDatabaseOpenTableRead
        tab = c_void_p()
        rc = f( self.__ptr, byref( tab ), to_char_p( name ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VDatabaseOpenTable( %s.%s )"%( self.__name, name ), self )
        return VTable( self.__mgr, tab, name )

    def OpenDB( self, name, mode = OpenMode.Read ) :
        f = self.__mgr.VDatabaseOpenDBUpdate if mode == OpenMode.Write else self.__mgr.VDatabaseOpenDBRead
        db = c_void_p()
        rc = f( self.__ptr, byref( db ), to_char_p( name ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VDatabaseOpenDB( %s.%s )"%( self.__name, name ), self )
        return VDatabase( self.__mgr, db, name )

    def CreateTable( self, spec, name = None, mode = CreateMode.Init ) :
        tab = c_void_p()
        p = to_char_p( name if name != None else spec )
        rc = self.__mgr.VDatabaseCreateTable( self.__ptr, byref( tab ), to_char_p( spec ), c_int( mode.value ), p )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VDatabaseCreateTable( %s.%s )"%( self.__name, spec ), self )
        return VTable( self.__mgr, tab, p.value )

    def CreateDB( self, spec, name = None, mode = CreateMode.Init ) :
        new_db = c_void_p()
        p = to_char_p( name if name != None else spec )
        rc = self.__mgr.VDatabaseCreateDB( self.__ptr, byref( new_db ), to_char_p( spec ), c_int( mode.value ), p )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VDatabaseCreateDB( %s.%s )"%( self.__name, spec ), self )
        return VDatabase( self.__mgr, new_db, p.value )

    def DropTable( self, name ) :
        rc = self.__mgr.VDatabaseDropTable( self.__ptr, to_char_p( name ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VDatabaseDropTable( %s.%s )"%( self.__name, name ), self )

    def DropDB( self, name ) :
        rc = self.__mgr.VDatabaseDropDB( self.__ptr, to_char_p( name ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VDatabaseDropDB( %s.%s )"%( self.__name, name ), self )

    def Lock( self, name, what ) :
        rc = self.__mgr.VDatabaseLock( self.__ptr, c_int( what ), to_char_p( name ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VDatabaseLock( %s.%s )"%( self.__name, name ), self )

    def UnLock( self, name, what ) :
        rc = self.__mgr.VDatabaseUnlock( self.__ptr, c_int( what ), to_char_p( name ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VDatabaseUnlock( %s.%s )"%( self.__name, name ), self )

    def Locked( self ) :
        return self.__mgr.VDatabaseLocked( self.__ptr )

    def ReferenceList( self, options = 2 ) :
        ptr = c_void_p()
        rc = self.__mgr.ReferenceList_MakeDatabase( byref( ptr ), self.__ptr, c_int( options ), c_int( 0 ), c_char_p( 0 ), c_int( 0 ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "ReferenceList_MakeDatabase( %s )"%( self.__name ), self )
        return ReferenceList( self.__mgr, ptr )


#------------------------------------------------------------------------------------------------------------
class VSchema :
    def __init__( self, mgr, ptr ) :
        self.__mgr = mgr
        self.__ptr = ptr

    def __del__( self ) :
        rc = self.__mgr.VSchemaRelease( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VSchemaRelease()", self )

    def __enter__( self ) :
        return self

    def __exit__( self, type, value, traceback ) :
        pass

    def ParseText( self, schema_txt ) :
        txt = to_char_p( schema_txt )
        l = len( schema_txt )
        rc = self.__mgr.VSchemaParseText( self.__ptr, c_char_p( 0 ), txt, c_int( l ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VSchemaParseText()", self )
        
    def ParseFile( self, schema_file ) :
        rc = self.__mgr.VSchemaParseFile( self.__ptr, to_char_p( schema_file ) )
        if rc != 0 :
            self.mgr.raise_rc( rc, "VSchemaParseFile( '%s' )"%schema_file, self )

    def AddIncludePath( self, path ) :
        rc = self.__mgr.VSchemaAddIncludePath( self.__ptr, to_char_p( path ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VSchemaAddIncludePath( '%s' )"%path, self )


#------------------------------------------------------------------------------------------------------------
class KRepository :
    def __init__( self, mgr, ptr ) :
        self.__mgr = mgr
        self.__ptr = ptr

    def __del__( self ) :
        rc = self.__mgr.KRepositoryRelease( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KRepositoryRelease()", self )

    def __enter__( self ) :
        return self

    def __exit__( self, type, value, traceback ) :
        pass

    def __str__( self ) :
        lst = [ "'%s'"%( self.Name() ) ]
        lst.append( "%s"%( RepoCat( self.Category() ) ) )
        lst.append( "%s"%( RepoSubCat( self.SubCategory() ) ) )
        lst.append( "display: %s"%( self.DisplayName() ) )
        lst.append( "root: %s"%( self.Root() ) )
        lst.append( "resolver: %s"%( self.Resolver() ) )
        lst.append( "disabled: %r"%( self.Disabled() ) )
        lst.append( "cached: %r"%( self.CacheEnabled() ) )
        return ", ".join( lst )

    def AddRef ( self ) :
        rc = self.__mgr.KRepositoryAddRef( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KRepositoryAddRef()", self )

    def Category( self ) :
        res = self.__mgr.KRepositoryCategory( self.__ptr )
        return res;
        
    def SubCategory( self ) :
        return self.__mgr.KRepositorySubCategory( self.__ptr )

    def Name( self ) :
        p = create_string_buffer( 1024 )
        n = c_int( 0 )
        rc = self.__mgr.KRepositoryName( self.__ptr, p, sizeof( p ), byref( n ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KRepositoryName()", self )
        if PY3 :
            return p.value.decode( "utf-8" )
        return p.value

    def DisplayName( self ) :
        p = create_string_buffer( 1024 )
        n = c_int( 0 )
        rc = self.__mgr.KRepositoryDisplayName( self.__ptr, p, sizeof( p ), byref( n ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KRepositoryDisplayName()", self )
        if PY3 :
            return p.value.decode( "utf-8" )
        return p.value

    def Root( self ) :
        p = create_string_buffer( 1024 )
        n = c_int( 0 )
        rc = self.__mgr.KRepositoryRoot( self.__ptr, p, sizeof( p ), byref( n ) )
        if rc != 0 :
            return ""
        if PY3 :
            return p.value.decode( "utf-8" )
        return p.value

    def SetRoot( self, root ) :
        rc = self.__mgr.KRepositorySetRoot( self.__ptr, to_char_p( root ), c_int( len( root ) ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KRepositorySetRoot( '%s' )"%( root ), self )

    def Resolver( self ) :
        p = create_string_buffer( 1024 )
        n = c_int( 0 )
        rc = self.__mgr.KRepositoryResolver( self.__ptr, p, sizeof( p ), byref( n ) )
        if rc != 0 :
            return ""
        if PY3 :
            return p.value.decode( "utf-8" )
        return p.value

    def Disabled( self ) :
        return self.__mgr.KRepositoryDisabled( self.__ptr )
        
    def SetDisabled( self, value ) :
        rc = self.__mgr.KRepositorySetDisabled( self.__ptr, c_bool( value ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KRepositorySetDisabled()", self )

    def CacheEnabled( self ) :
        return self.__mgr.KRepositoryCacheEnabled( self.__ptr )
        

#------------------------------------------------------------------------------------------------------------
class KRepositoryMgr :
    def __init__( self, mgr, ptr ) :
        self.__mgr = mgr
        self.__ptr = ptr

    def __del__( self ) :
        rc = self.__mgr.KRepositoryMgrRelease( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KRepositoryMgrRelease()", self )

    def __enter__( self ) :
        return self

    def __exit__( self, type, value, traceback ) :
        pass

    def HasRemoteAccess( self ) :
        return self.__mgr.KRepositoryMgrHasRemoteAccess( self.__ptr );

    def __vector_2_list__( self, v ) :
        res = list()
        for i in xrange( v.start, v.start + v.len ) :
            r = KRepository( self.__mgr, self.__mgr.VectorGet( byref( v ), c_int( i ) ) )
            r.AddRef()
            res.append( r )
        self.__mgr.KRepositoryVectorWhack( byref( v ) )
        return res
    
    def UserRepositories( self ) :
        v = vdb_vector()
        rc = self.__mgr.KRepositoryMgrUserRepositories( self.__ptr, byref( v ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KRepositoryMgrUserRepositories()", self )
        return self.__vector_2_list__( v )

    def SiteRepositories( self ) :
        v = vdb_vector()
        rc = self.__mgr.KRepositoryMgrSiteRepositories( self.__ptr, byref( v ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KRepositoryMgrSiteRepositories()", self )
        return self.__vector_2_list__( v )

    def RemoteRepositories( self ) :
        v = vdb_vector()
        rc = self.__mgr.KRepositoryMgrRemoteRepositories( self.__ptr, byref( v ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KRepositoryMgrRemoteRepositories()", self )
        return self.__vector_2_list__( v )

    def CategoryDisabled( self, cat ) :
        return self.__mgr.KRepositoryMgrCategoryDisabled( self.__ptr, c_int( cat ) )

    def CategorySetDisabled( self, cat, value ) :
        return self.__mgr.KRepositoryMgrCategorySetDisabled( self.__ptr, c_int( cat ), c_bool( value ) )

    def AllRepos( self ) :
        return [ self.UserRepositories(), self.SiteRepositories(), self.RemoteRepositories() ]

#------------------------------------------------------------------------------------------------------------
class KConfig :
    def __init__( self, mgr, ptr ) :
        self.__mgr = mgr
        self.__ptr = ptr
        
    def __del__( self ) :
        rc = self.__mgr.KConfigRelease( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KConfigRelease()", self )
            
    def __enter__( self ) :
        return self

    def __exit__( self, type, value, traceback ) :
        pass

    def Commit( self ) :
        rc = self.__mgr.KConfigCommit( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KConfigCommit()", self )
        return rc

    def ReadBool( self, path ) :
        value = c_bool()
        rc = self.__mgr.KConfigReadBool( self.__ptr, to_char_p( path ), byref( value ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KConfigReadBool( '%s' )"%( path ), self )
        return value.value

    def WriteBool( self, path, value ) :
        rc = self.__mgr.KConfigWriteBool( self.__ptr, to_char_p( path ), c_bool( value ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KConfigWriteBool( '%s', %s )"%( path, value ), self )
        return rc

    def ReadString( self, path ) :
        s = vdb_string( None, 0, 0 )
        sp = pointer( s )
        rc = self.__mgr.KConfigReadString( self.__ptr, to_char_p( path ), byref( sp ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KConfigReadString( '%s' )"%( path ), self )
        res = sp.contents.addr
        self.__mgr.string_whack( sp.contents )
        return res

    def WriteString( self, path, value ) :
        rc = self.__mgr.KConfigWriteString( self.__ptr, to_char_p( path ), to_char_p( value ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KConfigWriteString( '%s', '%s' )"%( path, value ), self )
        return rc

    def ReadI64( self, path ) :
        value = c_longlong()
        rc = self.__mgr.KConfigReadI64( self.__ptr, to_char_p( path ), byref( value ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KConfigReadI64( '%s' )"%( path ), self )
        return value.value

    def ReadU64( self, path ) :
        value = c_ulonglong()
        rc = self.__mgr.KConfigReadU64( self.__ptr, to_char_p( path ), byref( value ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KConfigReadU64( '%s' )"%( path ), self )
        return value.value

    def ReadF64( self, path ) :
        value = c_double()
        rc = self.__mgr.KConfigReadF64( self.__ptr, to_char_p( path ), byref( value ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KConfigReadF64( '%s' )"%( path ), self )
        return value.value

    def DisableUserSettings( self ) :
        rc = self.__mgr.KConfigDisableUserSettings( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "KConfigDisableUserSettings()", self )
        return value.value

    def MakeRepositoryMgr( self, mode = OpenMode.Read ) :
        ptr = c_void_p()
        rc = 0
        if mode == OpenMode.Read :
            rc = self.__mgr.KConfigMakeRepositoryMgrRead( self.__ptr, byref( ptr ) )
            if rc != 0 :
                self.__mgr.raise_rc( rc, "KConfigMakeRepositoryMgrRead()", self )
        else :
            rc = self.__mgr.KConfigMakeRepositoryMgrUpdate( self.__ptr, byref( ptr ) )
            if rc != 0 :
                self.__mgr.raise_rc( rc, "KConfigMakeRepositoryMgrUpdate()", self )
        return KRepositoryMgr( self.__mgr, ptr )


#------------------------------------------------------------------------------------------------------------
class VPath :
    def __init__( self, mgr, ptr ) :
        self.mgr = __mgr
        self.ptr = __ptr

    def __str__( self ) :
        if self.__ptr :
            s = vdb_string( None, 0, 0 )
            sp = pointer( s )
            rc = VPathMakeString( self.__ptr, byref( sp ) )
            if rc != 0 :
                self.__mgr.raise_rc( rc, "VPathMakeString()", self )
            res = sp.contents.addr
            self.__mgr.string_whack( sp.contents )
        else :
            res = ""
        return res


#------------------------------------------------------------------------------------------------------------
class VResolver :
    def __init__( self, mgr, ptr ) :
        self.__mgr = mgr
        self.__ptr = ptr

    def __del__( self ) :
        rc = self.__mgr.VResolverRelease( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VResolverRelease()", self )

    def __enter__( self ) :
        return self

    def __exit__( self, type, value, traceback ) :
        pass

    def QueryLocal( self, path ) :
        loc = c_void_p()
        rc = self.__mgr.VResolverQuery( self.__ptr, c_int( 0 ), path.__ptr, byref( loc ), 0, 0 )
        if rc != 0 :
            return vpath( self.__mgr, 0 )
        return vpath( self.__mgr, loc )

    def QueryRemote( self, path, remote_prot = RemoteProto.Http ) :
        rem = c_void_p()
        rc = self.__mgr.fVResolverQuery( self.__ptr, c_int( remote_prot ), path.__ptr, 0, byref( rem ), 0 )
        if rc != 0 :
            return vpath( self.__mgr, 0 )
        return vpath( self.__mgr, rem )

    def QueryCache( self, path ) :
        cache = c_void_p()
        rc = self.__mgr.VResolverQuery( self.__ptr, c_int( 0 ), path.__ptr, 0, 0, byref( cache ) )
        if rc != 0 :
            return vpath( self.__mgr, 0 )
        return vpath( self.__mgr, cache )

    def RemoteEnable( self, state = ResolvEnable.UseConfig ) :
        rc = self.__mgr.VResolverRemoteEnable( self.__ptr, c_int( state ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VResolverRemoteEnable()", self )


#------------------------------------------------------------------------------------------------------------
class VFSManager :
    def __init__( self, mgr, ptr ) :
        self.__mgr = mgr
        self.__ptr = ptr

    def __del__( self ) :
        rc = self.__mgr.VFSManagerRelease( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VFSManagerRelease()", self )

    def __enter__( self ) :
        return self

    def __exit__( self, type, value, traceback ) :
        pass

    def GetResolver( self ) :
        ptr = c_void_p()
        rc = self.__mgr.VFSManagerGetResolver( self.__ptr, byref( ptr ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VFSManagerGetResolver()", self )
        return VResolver( self.__mgr, ptr )

    def MakeResolver( self, cfg ) :
        ptr = c_void_p()
        rc = self.__mgr.VFSManagerMakeResolver( self.__ptr, byref( ptr ), cfg.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VFSManagerMakeResolver()", self )
        return VResolver( self.__mgr, ptr )
        
    def MakePath( self, s_path ) :
        path = c_void_p()
        rc = self.__mgr.VFSManagerMakePath( self.__ptr, byref( path ), to_char_p( s_path ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "VFSManagerMakePath( '%s' )"%( s_paht ), self )
        return VPath( self.__mgr, path )


#------------------------------------------------------------------------------------------------------------
class RefVariation :
    def __init__( self, mgr, ptr ) :
        self.__mgr = mgr
        self.__ptr = ptr

    def __del__( self ) :
        rc = self.__mgr.RefVariationRelease( self.__ptr )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "RefVariationRelease()", self )

    def __enter__( self ) :
        return self

    def __exit__( self, type, value, traceback ) :
        pass

    def GetIUPACSearchQuery( self ) :
        txt = c_char_p()
        len = c_int()
        start = c_int()
        rc = self.__mgr.RefVariationGetIUPACSearchQuery( self.__ptr, byref( txt ), byref( len ), byref( start ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "RefVariationGetIUPACSearchQuery()", self )
        return ( txt.value, len.value, start.value )
        
    def GetSearchQueryLenOnRef( self ) :
        len = c_int()
        rc = self.__mgr.RefVariationGetSearchQueryLenOnRef( self.__ptr, byref( len ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "RefVariationGetSearchQueryLenOnRef()", self )
        return len.value

    def GetAllele( self ) :
        txt = c_char_p()
        len = c_int()
        start = c_int()
        rc = self.__mgr.RefVariationGetAllele( self.__ptr, byref( txt ), byref( len ), byref( start ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "RefVariationGetAllele()", self )
        return ( txt.value, len.value, start.value )
        
    def GetAlleleLenOnRef( self ) :
        len = c_int()
        rc = self.__mgr.RefVariationGetAlleleLenOnRef( self.__ptr, byref( len ) )
        if rc != 0 :
            self.__mgr.raise_rc( rc, "RefVariationGetAlleleLenOnRef()", self )
        return len.value


#------------------------------------------------------------------------------------------------------------
def make_lib_name( mode, prefix = None ) :
    libname = None
    if platform.system() == "Windows":
        libname = "ncbi-wvdb.dll" if mode == OpenMode.Write else "ncbi-vdb.dll"
    elif platform.system() == "Darwin":
        libname = "libncbi-wvdb.dylib" if mode == OpenMode.Write else "libncbi-vdb.dylib"
    elif platform.system() == "Linux" :
        libname = "libncbi-wvdb.so" if mode == OpenMode.Write else "libncbi-vdb.so"
    if libname == None :
        return None
    if prefix != None :
        return os.path.join( os.path.sep, prefix, libname )
    return libname

def check_lib_path( mode, path ) :
    res = None
    if path != None :
        if not os.path.isfile( path ) :
            raise vdb_error( 0, "lib '%s' does not exist"% path, None )
        else :
            res = path
    else :
        res = make_lib_name( mode )
        if res == None :
            raise vdb_error( 0, "cannot load lib: unknow platform", None )
        else :
            if not os.path.isfile( res ) :
                home_ncbi_lib64 = os.path.join( os.path.sep, os.path.expanduser( "~" ), ".ncbi", "lib64" )
                res = make_lib_name( mode, home_ncbi_lib64 )
        if not os.path.isfile( res ) :
            raise vdb_error( 0, "cannot find lib: '%s'"%p, None )
    return res


class manager :
    def __init__( self, mode = OpenMode.Read, path = None ) :
        self.__mode = mode
        self.__dir = None
        self.__ptr = None

        p = check_lib_path( mode, path )
        try :
            self.__lib = cdll.LoadLibrary( p )
        except :
            raise vdb_error( 0, "cannot load library '%s'"%p, None )

        #we need this one first, because we need it to throw a vdb-error ( used in manager.explain() )
        self.string_printf = self.__func__( "string_printf", [ c_void_p, c_int, c_void_p, c_char_p, c_int ] )
        
        self.KDirectoryNativeDir = self.__func__( "KDirectoryNativeDir_v1", [ c_void_p ] )
        self.KDirectoryRelease = self.__func__( "KDirectoryRelease_v1", [ c_void_p ] )
        
        self.VDBManagerRelease = self.__func__( "VDBManagerRelease", [ c_void_p ] )
        self.VDBManagerVersion = self.__func__( "VDBManagerVersion", [ c_void_p, c_void_p ] )
        self.VDBManagerPathType = self.__func__( "VDBManagerPathType", [ c_void_p, c_char_p ] )
        self.VDBManagerGetObjVersion = self.__func__( "VDBManagerGetObjVersion", [ c_void_p, c_void_p, c_char_p ] )
        self.VDBManagerGetObjModDate = self.__func__( "VDBManagerGetObjModDate", [ c_void_p, c_void_p, c_char_p ] )
        self.VDBManagerOpenDBRead = self.__func__( "VDBManagerOpenDBRead", [ c_void_p, c_void_p, c_void_p, c_char_p ] )
        self.VDBManagerOpenTableRead = self.__func__( "VDBManagerOpenTableRead", [ c_void_p, c_void_p, c_void_p, c_char_p ] )
        self.VDBManagerMakeSchema = self.__func__( "VDBManagerMakeSchema", [ c_void_p, c_void_p ] )
        
        self.VDatabaseOpenDBRead = self.__func__( "VDatabaseOpenDBRead", [ c_void_p, c_void_p, c_char_p ] )
        self.VDatabaseRelease = self.__func__( "VDatabaseRelease", [ c_void_p ] )
        self.VDatabaseOpenTableRead = self.__func__( "VDatabaseOpenTableRead", [ c_void_p, c_void_p, c_char_p ] )
        self.VDatabaseListTbl = self.__func__( "VDatabaseListTbl", [ c_void_p, c_void_p ] )
        self.VDatabaseListDB = self.__func__( "VDatabaseListDB", [ c_void_p, c_void_p ] )
        self.VDatabaseLocked = self.__func__( "VDatabaseLocked", [ c_void_p ], c_bool )
        
        self.VTableRelease = self.__func__( "VTableRelease", [ c_void_p ] )
        self.VTableListCol = self.__func__( "VTableListCol", [ c_void_p, c_void_p ] )
        self.VTableCreateCursorRead = self.__func__( "VTableCreateCursorRead", [ c_void_p, c_void_p ] )
        self.VTableCreateCachedCursorRead = self.__func__( "VTableCreateCachedCursorRead", [ c_void_p, c_void_p, c_longlong ] )
        self.VTableLocked = self.__func__( "VTableLocked", [ c_void_p ], c_bool )
        self.VTableOpenKTableRead = self.__func__( "VTableOpenKTableRead", [ c_void_p, c_void_p ] )
        
        self.KNamelistCount = self.__func__( "KNamelistCount", [ c_void_p, c_void_p ] )
        self.KNamelistGet = self.__func__( "KNamelistGet", [ c_void_p, c_int, c_void_p ] )
        self.KNamelistRelease = self.__func__( "KNamelistRelease", [ c_void_p ] )

        self.VSchemaRelease = self.__func__( "VSchemaRelease", [ c_void_p ] )
        self.VSchemaParseText = self.__func__( "VSchemaParseText", [ c_void_p, c_char_p, c_char_p, c_int ] )
        self.VSchemaParseFile = self.__func__( "VSchemaParseFile", [ c_void_p, c_char_p ] )
        self.VSchemaAddIncludePath = self.__func__( "VSchemaAddIncludePath", [ c_void_p, c_char_p ] )

        self.VCursorRelease = self.__func__( "VCursorRelease", [ c_void_p ] )            
        self.VCursorAddColumn = self.__func__( "VCursorAddColumn", [ c_void_p, c_void_p, c_char_p ] )
        self.VCursorOpen = self.__func__( "VCursorOpen", [ c_void_p ] )
        self.VCursorDatatype = self.__func__( "VCursorDatatype", [ c_void_p, c_int, c_void_p, c_void_p ] )
        self.VCursorCellDataDirect = self.__func__( "VCursorCellDataDirect", [ c_void_p, c_longlong, c_int, c_void_p, c_void_p, c_void_p, c_void_p ] )
        self.VCursorCloseRow = self.__func__( "VCursorCloseRow", [ c_void_p ] )
        self.VCursorIdRange = self.__func__( "VCursorIdRange", [ c_void_p, c_int, c_void_p, c_void_p ] )
        self.VCursorRowId = self.__func__( "VCursorRowId", [ c_void_p, c_void_p ] )
        self.VCursorOpenRow = self.__func__( "VCursorOpenRow", [ c_void_p ] )
        self.VCursorFindNextRowIdDirect = self.__func__( "VCursorFindNextRowIdDirect", [ c_void_p, c_int, c_longlong, c_void_p ] )

        self.KTableRelease = self.__func__( "KTableRelease", [ c_void_p ] )
        self.KTableListIdx = self.__func__( "KTableListIdx", [ c_void_p, c_void_p ] )
        self.KTableOpenIndexRead = self.__func__( "KTableOpenIndexRead", [ c_void_p, c_void_p, c_char_p ] )
        self.KTableOpenMetadataRead = self.__func__( "KTableOpenMetadataRead", [ c_void_p, c_void_p ] )
        
        self.KIndexRelease = self.__func__( "KIndexRelease", [ c_void_p ] )
        self.KIndexVersion = self.__func__( "KIndexVersion", [ c_void_p, c_void_p ] )
        self.KIndexType = self.__func__( "KIndexType", [ c_void_p, c_void_p ] )
        self.KIndexLocked = self.__func__( "KIndexLocked", [ c_void_p ], c_bool )
        self.KIndexFindText = self.__func__( "KIndexFindText", [ c_void_p, c_char_p, c_void_p, c_void_p, c_void_p, c_void_p ] )
        self.KIndexProjectText = self.__func__( "KIndexProjectText", [ c_void_p, c_longlong, c_void_p, c_void_p, c_char_p, c_int, c_void_p ] )

        self.KMetadataRelease = self.__func__( "KMetadataRelease", [ c_void_p ] )
        self.KMetadataVersion = self.__func__( "KMetadataVersion", [ c_void_p, c_void_p ] )
        self.KMetadataByteOrder = self.__func__( "KMetadataByteOrder", [ c_void_p, c_void_p ] )
        self.KMetadataRevision = self.__func__( "KMetadataRevision", [ c_void_p, c_void_p ] )
        self.KMetadataMaxRevision = self.__func__( "KMetadataMaxRevision", [ c_void_p, c_void_p ] )
        self.KMetadataOpenRevision = self.__func__( "KMetadataOpenRevision", [ c_void_p, c_void_p, c_int ] )
        self.KMetadataGetSequence = self.__func__( "KMetadataGetSequence", [ c_void_p, c_char_p, c_void_p ] )
        self.KMetadataOpenNodeRead = self.__func__( "KMetadataOpenNodeRead", [ c_void_p, c_void_p, c_char_p ] )

        self.KMDataNodeOpenNodeRead = self.__func__( "KMDataNodeOpenNodeRead", [ c_void_p, c_void_p, c_char_p ] )
        self.KMDataNodeRelease = self.__func__( "KMDataNodeRelease", [ c_void_p ] )
        self.KMDataNodeListAttr = self.__func__( "KMDataNodeListAttr", [ c_void_p ] )
        self.KMDataNodeListChildren = self.__func__( "KMDataNodeListChildren", [ c_void_p ] )
        self.KMDataNodeByteOrder = self.__func__( "KMDataNodeByteOrder", [ c_void_p, c_void_p ] )
        self.KMDataNodeRead = self.__func__( "KMDataNodeRead", [ c_void_p, c_size_t, c_void_p, c_size_t, c_void_p, c_void_p ] )

        if mode == OpenMode.Write :
            self.VDBManagerMakeUpdate = self.__func__( "VDBManagerMakeUpdate", [ c_void_p, c_void_p ] )
            self.VDBManagerCreateTable = self.__func__( "VDBManagerCreateTable", [ c_void_p, c_void_p, c_void_p, c_char_p, c_int, c_char_p ] )
            self.VDBManagerCreateDB = self.__func__( "VDBManagerCreateDB", [ c_void_p, c_void_p, c_void_p, c_char_p, c_int, c_char_p ] )
            self.VDBManagerOpenTableUpdate = self.__func__( "VDBManagerOpenTableUpdate", [ c_void_p, c_void_p, c_void_p, c_char_p ] )
            self.VDBManagerOpenDBUpdate = self.__func__( "VDBManagerOpenDBUpdate", [ c_void_p, c_void_p, c_void_p, c_char_p ] )

            self.VTableCreateCursorWrite = self.__func__( "VTableCreateCursorWrite", [ c_void_p, c_void_p, c_int ] )
            self.VTableOpenKTableUpdate = self.__func__( "VTableOpenKTableUpdate", [ c_void_p, c_void_p ] )

            self.VCursorCommitRow = self.__func__( "VCursorCommitRow", [ c_void_p ] )
            self.VCursorRepeatRow = self.__func__( "VCursorRepeatRow", [ c_void_p, c_longlong ] )
            self.VCursorFlushPage = self.__func__( "VCursorFlushPage", [ c_void_p ] )
            self.VCursorCommit = self.__func__( "VCursorCommit", [ c_void_p ] )
            self.VCursorWrite = self.__func__( "VCursorWrite", [ c_void_p, c_int, c_int, c_void_p, c_int, c_int ] )
            self.VCursorDefault = self.__func__( "VCursorDefault", [ c_void_p, c_int, c_int, c_void_p, c_int, c_int ] )

            self.VDatabaseCreateDB = self.__func__( "VDatabaseCreateDB", [ c_void_p, c_void_p, c_char_p, c_int, c_char_p ] )
            self.VDatabaseOpenTableUpdate = self.__func__( "VDatabaseOpenTableUpdate", [ c_void_p, c_void_p, c_char_p ] )
            self.VDatabaseOpenDBUpdate = self.__func__( "VDatabaseOpenDBUpdate", [ c_void_p, c_void_p, c_char_p ] )
            self.VDatabaseCreateTable = self.__func__( "VDatabaseCreateTable", [ c_void_p, c_void_p, c_char_p, c_int, c_char_p ] )
            self.VDatabaseDropDB = self.__func__( "VDatabaseDropDB", [ c_void_p, c_char_p ] )
            self.VDatabaseDropTable = self.__func__( "VDatabaseDropTable", [ c_void_p, c_char_p ] )
            self.VDatabaseLock = self.__func__( "VDatabaseLock", [ c_void_p, c_int, c_char_p ] )
            self.VDatabaseUnlock = self.__func__( "VDatabaseUnlock", [ c_void_p, c_int, c_char_p ] )

            self.KTableCreateIndex = self.__func__( "KTableCreateIndex", [ c_void_p, c_void_p, c_int, c_int, c_char_p ] )
            self.KIndexCommit = self.__func__( "KIndexCommit", [ c_void_p ] )
            self.KIndexInsertText = self.__func__( "KIndexInsertText", [ c_void_p, c_bool, c_char_p, c_longlong ] )
            self.KIndexDeleteText = self.__func__( "KIndexDeleteText", [ c_void_p, c_char_p ] )
            self.KIndexInsertU64 = self.__func__( "KIndexInsertU64", [ c_void_p, c_bool, c_longlong, c_longlong, c_longlong, c_longlong ] )
            self.KIndexDeleteU64 = self.__func__( "KIndexDeleteU64", [ c_void_p, c_longlong ] )

            self.KTableOpenMetadataUpdate = self.__func__( "KTableOpenMetadataUpdate", [ c_void_p, c_void_p ] )

            self.KMetadataCommit = self.__func__( "KMetadataCommit", [ c_void_p ] )
            self.KMetadataFreeze = self.__func__( "KMetadataFreeze", [ c_void_p ] )
            self.KMetadataSetSequence = self.__func__( "KMetadataSetSequence", [ c_void_p, c_char_p, c_longlong ] )
            self.KMetadataNextSequence = self.__func__( "KMetadataNextSequence", [ c_void_p, c_char_p, c_void_p ] )
            self.KMetadataOpenNodeUpdate = self.__func__( "KMetadataOpenNodeUpdate", [ c_void_p, c_void_p, c_char_p ] )

            self.KMDataNodeOpenNodeUpdate = self.__func__( "KMDataNodeOpenNodeUpdate", [ c_void_p, c_void_p, c_char_p ] )
            self.KMDataNodeWrite = self.__func__( "KMDataNodeWrite", [ c_void_p, c_void_p, c_size_t ] )
            self.KMDataNodeAppend = self.__func__( "KMDataNodeAppend", [ c_void_p, c_void_p, c_size_t ] )
            
        else :
            self.VDBManagerMakeRead = self.__func__( "VDBManagerMakeRead", [ c_void_p, c_void_p ] )
            
        self.KConfigMake = self.__func__( "KConfigMake", [ c_void_p, c_void_p ] )
        self.KConfigRelease = self.__func__( "KConfigRelease", [ c_void_p ] )
        self.KConfigCommit = self.__func__( "KConfigCommit", [ c_void_p ] )
        self.KConfigReadBool = self.__func__( "KConfigReadBool", [ c_void_p, c_char_p, c_void_p ] )
        self.KConfigWriteBool = self.__func__( "KConfigWriteBool", [ c_void_p, c_char_p, c_bool ] )
        self.KConfigReadString = self.__func__( "KConfigReadString", [ c_void_p, c_char_p, c_void_p ] )
        self.KConfigWriteString = self.__func__( "KConfigWriteString", [ c_void_p, c_char_p, c_char_p ] )
        self.KConfigReadI64 = self.__func__( "KConfigReadI64", [ c_void_p, c_char_p, c_void_p ] )
        self.KConfigReadU64 = self.__func__( "KConfigReadU64", [ c_void_p, c_char_p, c_void_p ] )
        self.KConfigReadF64 = self.__func__( "KConfigReadF64", [ c_void_p, c_char_p, c_void_p ] )
        self.KConfigDisableUserSettings = self.__func__( "KConfigDisableUserSettings", [] )
        self.KConfigMakeRepositoryMgrRead = self.__func__( "KConfigMakeRepositoryMgrRead", [ c_void_p, c_void_p ] )
        self.KConfigMakeRepositoryMgrUpdate = self.__func__( "KConfigMakeRepositoryMgrUpdate", [ c_void_p, c_void_p ] )

        self.KRepositoryMgrRelease = self.__func__( "KRepositoryMgrRelease", [ c_void_p ] )
        self.KRepositoryMgrHasRemoteAccess = self.__func__( "KRepositoryMgrHasRemoteAccess", [ c_void_p ], c_bool )
        self.KRepositoryMgrUserRepositories = self.__func__( "KRepositoryMgrUserRepositories", [ c_void_p, c_void_p ] )
        self.KRepositoryMgrSiteRepositories = self.__func__( "KRepositoryMgrSiteRepositories", [ c_void_p, c_void_p ] )
        self.KRepositoryMgrRemoteRepositories = self.__func__( "KRepositoryMgrRemoteRepositories", [ c_void_p, c_void_p ] )
        self.KRepositoryMgrCategoryDisabled = self.__func__( "KRepositoryMgrCategoryDisabled", [ c_void_p, c_int ], c_bool )
        self.KRepositoryMgrCategorySetDisabled = self.__func__( "KRepositoryMgrCategorySetDisabled", [ c_void_p, c_int, c_bool ] )
        self.KRepositoryVectorWhack = self.__func__( "KRepositoryVectorWhack", [ c_void_p ] )
        self.KRepositoryAddRef = self.__func__( "KRepositoryAddRef", [ c_void_p ] )
        self.KRepositoryRelease = self.__func__( "KRepositoryRelease", [ c_void_p ] )
        self.KRepositoryCategory = self.__func__( "KRepositoryCategory", [ c_void_p ] )
        self.KRepositorySubCategory = self.__func__( "KRepositorySubCategory", [ c_void_p ] )
        self.KRepositoryName = self.__func__( "KRepositoryName", [ c_void_p, c_char_p, c_int, c_void_p ] )
        self.KRepositoryDisplayName = self.__func__( "KRepositoryDisplayName", [ c_void_p, c_char_p, c_int, c_void_p ] )
        self.KRepositoryRoot = self.__func__( "KRepositoryRoot", [ c_void_p, c_char_p, c_int, c_void_p ] )
        self.KRepositorySetRoot = self.__func__( "KRepositorySetRoot", [ c_void_p, c_char_p, c_int ] )
        self.KRepositoryResolver = self.__func__( "KRepositoryResolver", [ c_void_p, c_char_p, c_int, c_void_p ] )
        self.KRepositoryDisabled = self.__func__( "KRepositoryDisabled", [ c_void_p ], c_bool )
        self.KRepositorySetDisabled = self.__func__( "KRepositorySetDisabled", [ c_void_p, c_bool ] )
        self.KRepositoryCacheEnabled = self.__func__( "KRepositoryCacheEnabled", [ c_void_p ], c_bool )

        self.VFSManagerMake = self.__func__( "VFSManagerMake", [ c_void_p ] )
        self.VFSManagerRelease = self.__func__( "VFSManagerRelease", [ c_void_p ] )
        self.VFSManagerGetResolver = self.__func__( "VFSManagerGetResolver", [ c_void_p, c_void_p ] )
        self.VFSManagerMakeResolver = self.__func__( "VFSManagerMakeResolver", [ c_void_p, c_void_p, c_void_p ] )
        self.VFSManagerMakePath = self.__func__( "VFSManagerMakePath", [ c_void_p, c_void_p, c_void_p ] )

        self.VResolverRelease = self.__func__( "VResolverRelease", [ c_void_p ] )
        self.VResolverQuery = self.__func__( "VResolverQuery", [ c_void_p, c_int, c_void_p, c_void_p, c_void_p, c_void_p ] )
        self.VResolverRemoteEnable = self.__func__( "VResolverRemoteEnable", [ c_void_p, c_int ] )

        self.RefVariationIUPACMake = self.__func__( "RefVariationIUPACMake", [ c_void_p, c_char_p, c_longlong, c_longlong, c_longlong, c_char_p, c_longlong, c_int ] )
        self.RefVariationRelease = self.__func__( "RefVariationRelease", [ c_void_p ] )
        self.RefVariationGetIUPACSearchQuery = self.__func__( "RefVariationGetIUPACSearchQuery", [ c_void_p, c_void_p, c_void_p, c_void_p ] )
        self.RefVariationGetSearchQueryLenOnRef = self.__func__( "RefVariationGetSearchQueryLenOnRef", [ c_void_p, c_void_p ] )
        self.RefVariationGetAllele = self.__func__( "RefVariationGetAllele", [ c_void_p, c_void_p, c_void_p, c_void_p ] )
        self.RefVariationGetAlleleLenOnRef = self.__func__( "RefVariationGetAlleleLenOnRef", [ c_void_p, c_void_p ] )

        self.ReferenceList_MakeCursor = self.__func__( "ReferenceList_MakeCursor", [ c_void_p, c_void_p, c_int, c_char_p, c_int ] )
        self.ReferenceList_MakeTable = self.__func__( "ReferenceList_MakeTable", [ c_void_p, c_void_p, c_int, c_int, c_char_p, c_int ] )
        self.ReferenceList_MakeDatabase = self.__func__( "ReferenceList_MakeDatabase", [ c_void_p, c_void_p, c_int, c_int, c_char_p, c_int ] )
        self.ReferenceList_MakePath = self.__func__( "ReferenceList_MakePath", [ c_void_p, c_void_p, c_char_p, c_int, c_int, c_char_p, c_int ] )
        self.ReferenceList_Release = self.__func__( "ReferenceList_Release", [ c_void_p ] )
        self.ReferenceList_Count = self.__func__( "ReferenceList_Count", [ c_void_p, c_void_p ] )
        self.ReferenceList_Find = self.__func__( "ReferenceList_Find", [ c_void_p, c_void_p, c_char_p, c_int ] )
        self.ReferenceList_Get = self.__func__( "ReferenceList_Get", [ c_void_p, c_void_p, c_int ] )

        self.ReferenceObj_Release = self.__func__( "ReferenceObj_Release", [ c_void_p ] )
        self.ReferenceObj_Idx = self.__func__( "ReferenceObj_Idx", [ c_void_p, c_void_p ] )
        self.ReferenceObj_IdRange = self.__func__( "ReferenceObj_IdRange", [ c_void_p, c_void_p, c_void_p ] )
        self.ReferenceObj_Bin = self.__func__( "ReferenceObj_Bin", [ c_void_p, c_void_p ] )
        self.ReferenceObj_SeqId = self.__func__( "ReferenceObj_SeqId", [ c_void_p, c_void_p ] )
        self.ReferenceObj_Name = self.__func__( "ReferenceObj_Name", [ c_void_p, c_void_p ] )
        self.ReferenceObj_SeqLength = self.__func__( "ReferenceObj_SeqLength", [ c_void_p, c_void_p ] )
        self.ReferenceObj_Circular = self.__func__( "ReferenceObj_Circular", [ c_void_p, c_void_p ] )
        self.ReferenceObj_External = self.__func__( "ReferenceObj_External", [ c_void_p, c_void_p, c_void_p ] )
        self.ReferenceObj_Read = self.__func__( "ReferenceObj_Read", [ c_void_p, c_int, c_int, c_void_p, c_void_p ] )
        self.ReferenceObj_GetIdCount = self.__func__( "ReferenceObj_GetIdCount", [ c_void_p, c_longlong, c_void_p ] )

        self.VPathMakeString = self.__func__( "VPathMakeString", [ c_void_p, c_void_p ] )
        self.VectorGet = self.__func__( "VectorGet", [ c_void_p, c_int ], c_void_p )
        self.StringWhack = self.__func__( "StringWhack", [ c_void_p ] )
        
        self.__dir = self.__make_native_dir__()
        self.__ptr = self.__make_mgr__( mode )


    def __del__( self ) :
        if self.__ptr != None :
            if self.VDBManagerRelease != None :
                self.VDBManagerRelease( self.__ptr )
        if self.__dir != None :
            if self.KDirectoryRelease != None :
                self.KDirectoryRelease( self.__dir )

    def __enter__( self ) :
        return self

    def __exit__( self, type, value, traceback ) :
        pass

    def string_whack( self, str ) :
        self.StringWhack( byref( str ) )
        
    def __make_native_dir__( self ) :
        res = c_void_p()
        if self.KDirectoryNativeDir == None :
            raise "'KDirectoryNativeDir' not found in lib"
        else :
            rc = self.KDirectoryNativeDir( byref( res ) )
            if rc != 0 :
                self.raise_rc( rc, "KDirectoryNativeDir()", self )
        return res;

    def __make_mgr__( self, mode ) :
        res = c_void_p()
        if mode == OpenMode.Write :
            rc = self.VDBManagerMakeUpdate( byref( res ), self.__dir )
        else :
            rc = self.VDBManagerMakeRead( byref( res ), self.__dir )
        if rc != 0 :
            self.raise_rc( rc, "make_mgr()", self )
        return res

    def __func__( self, name, argt, rest = c_int ) :
        res = None
        try :
            res = getattr( self.__lib, name )
            res.argtypes = argt
            res.restype = rest
        except :
            pass
        return res

    def explain( self, rc ) :
        buffer = create_string_buffer( 1024 )
        num_writ = c_int( 0 )
        fmt = create_string_buffer( to_bytes( "%R" ) )
        if self.string_printf( buffer, c_int( 1024 ), byref( num_writ ), fmt, c_int( rc ) ) == 0 :
            if PY3 :
                return buffer.value.decode( "utf-8" )
            return buffer.value
        return "cannot explain %d"%( rc )

    def raise_rc( self, rc, funcname, obj ) :
        msg = "%s -> %s"%( funcname, self.explain( rc ) )
        raise vdb_error( rc, msg, obj )

    def Version( self ) :
        vers = c_int( 0 )
        rc = self.VDBManagerVersion( self.__ptr, byref( vers ) )
        if rc != 0 :
            self.raise_rc( rc, "version()", self )
        return version( vers.value )

    def writable( self ) :
        return self.__mode == OpenMode.Write
        
    def PathType( self, path ) :
        return PathType( self.VDBManagerPathType( self.__ptr, to_char_p( path ) ) )
        
    def GetObjVersion( self, path ) :
        vers = c_int()
        rc = self.VDBManagerGetObjVersion( self.__ptr, byref( vers ), to_char_p( path ) )
        if rc != 0 :
            self.raise_rc( rc, "obj_vers( '%s' )"%( path ), self )
        return version( vers.value )
            
    def GetObjModDate( self, path ) :
        t = c_int()
        rc = self.VDBManagerGetObjModDate( self.__ptr, byref( t ), to_char_p( path ) )
        if rc != 0 :
            self.raise_rc( rc, "obj_time( '%s' )"%( path ), self )
        return datetime.datetime.fromtimestamp( t.value )

    def OpenDB( self, path, writable = False, schema = None ) :
        f = self.VDBManagerOpenDBUpdate if writable else self.VDBManagerOpenDBRead
        db = c_void_p()
        vdb_schema_ptr = schema.__ptr if schema != None else c_void_p( 0 )
        rc = f( self.__ptr, byref( db ), vdb_schema_ptr, to_char_p( path ) )
        if rc != 0 :
            self.raise_rc( rc, "open_db( %s )"%( path ), self )
        return VDatabase( self, db, path )

    def OpenTable( self, path, writable = False, schema = None ) :
        f = self.VDBManagerOpenTableUpdate if writable else self.VDBManagerOpenTableRead
        tab = c_void_p()
        vdb_schema_ptr = schema.__ptr if schema != None else c_void_p( 0 )
        rc = f( self.__ptr, byref( tab ), vdb_schema_ptr, to_char_p( path ) )
        if rc != 0 :
            self.raise_rc( rc, "VDBManagerOpenTable( %s )"%( path ), self )
        return VTable( self, tab, path )

    def CreateDB( self, schema, spec, path = None, mode = CreateMode.Init ) :
        if schema == None :
            raise vdb_error( "CreateDB(): schema missing", self, 0 )
        if spec == None :
            raise vdb_error( "CreateDB(): spec missing", self, 0 )
        db = c_void_p()
        px = path if path != None else spec
        p = to_char_p( px )
        rc = self.VDBManagerCreateDB( self.__ptr, byref( db ), schema._VSchema__ptr, to_char_p( spec ), c_int( mode.value ), p )
        if rc != 0 :
            self.raise_rc( rc, "vdb_manger.VDBManagerCreateDB( %s )"%( px ), self )
        return VDatabase( self, db, px )

    def CreateTable( self, schema, spec, path = None, mode = CreateMode.Init ) :
        if schema == None :
            raise vdb_error( "CreateTable(): schema missing", self, 0 )
        if spec == None :
            raise vdb_error( "CreateTable(): spec missing", self, 0 )
        tab = c_void_p()
        px = path if path != None else spec
        p = to_char_p( px )
        rc = self.VDBManagerCreateTable( self.__ptr, byref( tab ), schema._VSchema__ptr, to_char_p( spec ), c_int( mode.value ), p )
        if rc != 0 :
            self.raise_rc( rc, "VDBManagerCreateTable( %s )"%( px ), self )
        return VTable( self, tab, px )

    def MakeSchema( self, schema_text = None ) :
        vdb_schema_ptr = c_void_p()
        rc = self.VDBManagerMakeSchema( self.__ptr, byref( vdb_schema_ptr ) )
        if rc != 0 :
            self.raise_rc( rc, "VDBManagerMakeSchema()", self )
        res = VSchema( self, vdb_schema_ptr )
        if schema_text != None :
            res.ParseText( schema_text )
        return res

    def MakeKConfig( self ) :
        ptr = c_void_p()
        rc = self.KConfigMake( byref( ptr ), c_void_p( 0 ) )
        if rc != 0 :
            self.raise_rc( rc, "KConfigMake()", self )
        return KConfig( self, ptr )

    def MakeVFSManager( self ) :
        ptr = c_void_p()
        rc = self.VFSManagerMake( byref( ptr ) )
        if rc != 0 :
            self.raise_rc( rc, "VFSManagerMake()", self )
        return VFSManager( self, ptr )

    def ReferenceList( self, path, options = 2 ) :
        ptr = c_void_p()
        rc = self.ReferenceList_MakePath( byref( ptr ), self.__ptr, to_char_p( path ), c_int( options ), c_int( 0 ), c_char_p( 0 ), c_int( 0 ) )
        if rc != 0 :
            self.raise_rc( rc, "ReferenceList_MakePath( '%s' )" % path, self )
        return ReferenceList( self, ptr )

    def RefVariation( self, ref_bases, del_pos, del_len, insertion, algo = 2 ) :
        ptr = c_void_p()
        ref = to_char_p( ref_bases )
        ref_len = c_longlong( len( ref_bases ) )
        ins = to_char_p( insertion )
        ins_len = c_longlong( len( insertion ) )
        rc = self.RefVariationIUPACMake( byref( ptr ), ref, ref_len, c_longlong( del_pos ), c_longlong( del_len ), ins, ins_len, c_int( algo ) )
        if rc != 0 :
            self.raise_rc( rc, "RefVariationIUPACMake( %s )"%( ref_bases ), self )
        return RefVariation( self, ptr )
