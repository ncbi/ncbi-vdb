#!/usr/bin/env python3

import sys, os, platform, glob

class lib_find :
	def __init__( self ) :
		self.__home = os.path.expanduser( "~" )
		self.__platform = platform.system()
		d1 = { "Linux":self.__find_linux, "Darwin":self.__find_mac, "Windows":self.__find_win }
		self.__f = d1 . get( self.__platform )
		dr = { "Linux":"libncbi-vdb.so", "Darwin":"libncbi-vdb.dylib", "Windows":"ncbi-vdb.dll" }
		dw = { "Linux":"libncbi-wvdb.so", "Darwin":"libncbi-wvdb.dylib", "Windows":"ncbi-wvdb.dll" }
		self.__rd = dr . get( self.__platform )
		self.__wr = dw . get( self.__platform )

	def __pick_1st( self, p, libname ) :
		pattern = os.path.join( os.path.sep, p, f"{libname}*" )
		files = glob.glob( pattern )
		f = files[ 0 ] if files else None
		if f : return f if os.path.isfile( f ) else None
		return None

	def __probe( self, lst_of_lst, writable ) :
		libname = self.__wr if writable else self.__rd
		if not libname : return None
		for lst in lst_of_lst :
			l = [ self.__home ] + lst
			p = os.path.join( os.path.sep, *l )
			if os.path.isdir( p ) :
				file = self.__pick_1st( p, libname )
				if file : return file
		return None

	def __find_linux( self, writable ) :
		l = [ [ ".ncbi", "lib64" ],
			[ "ncbi-outdir", "ncbi-vdb", "linux", "gcc", "x86_64", "rel", "lib" ],
			[ "ncbi-outdir", "ncbi-vdb", "linux", "gcc", "x86_64", "dbg", "lib" ] ]
		return self . __probe( l, writable )
	
	def __find_mac( self, writable ) :
		l = [ [ ".ncbi", "lib64" ],
			[ "ncbi-outdir", "ncbi-vdb", "mac", "clang", "x86_64", "rel", "lib" ],
			[ "ncbi-outdir", "ncbi-vdb", "mac", "clang", "x86_64", "dbg", "lib" ] ]
		return self . __probe( l, writable )
	
	def __find_win( self, writable ) :
		l = [ [ ".ncbi", "lib64" ],
			[ "OUTDIR", "ncbi-vdb", "Debug" ],
			[ "OUTDIR", "ncbi-vdb", "Release" ] ]
		return self . __probe( l, writable )

	def find( self, writable = False ) :
		return self.__f( writable ) if self.__f else None
	
	def find_at( self, location, writable = False ) :
		if os.path.isdir( location ) :
			libname = self.__wr if writable else self.__rd
			if not libname : return None
			return self.__pick_1st( location, libname )
		return None

#-----------------------------------------------------------------------------------------------------
if __name__ == '__main__' :
	print( "looking in default locations for libncbi-vdb:" )
	finder = lib_find()
	found = finder.find( False )
	if not found :
		print( "not found!" )
	else :
		print( f"found at: {found}" )

	print( "looking in default locations for libncbi-wvdb:" )
	found = finder.find( True )
	if not found :
		print( "not found!" )
	else :
		print( f"found at: {found}" )
