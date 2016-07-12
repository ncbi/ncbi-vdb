/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

#ifndef _h_svdb_
#define _h_svdb_

#ifndef _h_svdb_extern_
#include <svdb/extern.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define ALL_COLUMNS 0
#define STATIC_COLUMNS 1
#define NON_STATIC_COLUMNS 2
#define VISIBLE_COLUMNS 3
#define NON_VISIBLE_COLUMNS 4

/*****************************************************************************
    (1)
    translate an accession into a path ( if possible )

    returns:    NULL, if accession is not an accession
                path, if the accession is an accession

    ! path has to be free'd by caller !
*/
SVDB_EXTERN const char * CC svdb_accession_2_path( const char * accession );


/*****************************************************************************
    (2)
    opens a vdb-object by path ( an accession has to be translated into a path )

    returns:    NULL,   if path cannot be opened
                handle, if the path can be opened ( as db or as table )

    ! handle has to be closed by caller with svdb_close() !
*/
SVDB_EXTERN void * CC svdb_open_path( const char * path );


/*****************************************************************************
    (3)
    closes an open vdb-object

    ! NULL ptr will be ignored !
*/
SVDB_EXTERN void CC svdb_close( void * self );


/*****************************************************************************
    (4)
    returns the last error as plain-text if one occured

    ! NULL ptr will be ignored !
    returns empty string if no error occured or self == NULL
*/
SVDB_EXTERN const char * CC svdb_last_err( void );


/*****************************************************************************
    (5)
    returns if the svdb-object is a database

    ! NULL ptr will be ignored !
*/
SVDB_EXTERN int CC svdb_is_db( void * self );


/*****************************************************************************
    (6)
    returns how many tables are if the svdb-object

    ! NULL ptr will be ignored !
*/
SVDB_EXTERN int CC svdb_count_tabs( void * self );


/*****************************************************************************
    (7)
    returns the name of a table, indexed by tab_id

    ! NULL ptr in self will be ignored !
    ! returns NULL if tab_id is out of bounds !
*/
SVDB_EXTERN const char * CC svdb_tabname( void * self, const int tab_id );


/*****************************************************************************
    (8)
    returns the index of a table, found by name-lookup

    ! NULL ptr in self will be ignored !
    ! returns -1 if the given name is not in the svdb-object !
*/
SVDB_EXTERN int CC svdb_tab_idx( void * self, const char * name );


/*****************************************************************************
    (9)
    returns how many columns a table has

    ! NULL ptr in self will be ignored !
    ! returns 0 if tab_id is out of bounds !
*/
SVDB_EXTERN int CC svdb_count_cols( void * self, const int tab_id,
            const int selection );


/*****************************************************************************
    (10)
    returns the name of a column in a table

    ! NULL ptr in self will be ignored !
    ! returns NULL if tab_id/col_id is out of bounds !
*/
SVDB_EXTERN const char * CC svdb_colname( void * self, const int tab_id,
            const int selection, const int col_id );


/*****************************************************************************
    (11)
    returns the index of a column in a table, found by name-lookup

    ! NULL ptr in self will be ignored !
    ! returns -1 if tab_id is out of bounds !
    ! returns -1 if the given name is not in the svdb-object !
*/
SVDB_EXTERN int CC svdb_col_idx( void * self, const int tab_id,
            const int selection, const char * name );


/*****************************************************************************
    (12)
    changes the visibility of a column

    ! NULL ptr in self will be ignored !
    ! returns 0 if tab_id or col_id is out of bounds !
    ! returns 1 if suceessful !
*/
SVDB_EXTERN int CC svdb_set_column_visibility( void * self, const int tab_id,
            const int selection, const int col_id, const int visible );


/*****************************************************************************
    (13)
    returns the index of the default-type of a column in a table

    ! NULL ptr in self will be ignored !
    ! returns -1 if tab_id/col_id is out of bounds !
*/
SVDB_EXTERN int CC svdb_dflt_type_idx( void * self, const int tab_id,
            const int selection, const int col_id );


/*****************************************************************************
    (14)
    returns how many types a column in a table has

    ! NULL ptr in self will be ignored !
    ! returns 0 if tab_id/col_id is out of bounds !
*/
SVDB_EXTERN int CC svdb_count_types( void * self, const int tab_id,
            const int selection, const int col_id );


/*****************************************************************************
    (15)
    returns the name of the type of a column in a table

    ! NULL ptr in self will be ignored !
    ! returns NULL if tab_id/col_id/type_id is out of bounds !
*/
SVDB_EXTERN const char * CC svdb_typename( void * self, const int tab_id,
            const int selection, const int col_id, const int type_id );


/*****************************************************************************
    (16)
    returns the index of a type in a column/table, found by name-lookup

    ! NULL ptr in self will be ignored !
    ! returns -1 if tab_id/col_id is out of bounds !
    ! returns -1 if the given type is not a type of this column !
*/
SVDB_EXTERN int CC svdb_type_idx( void * self, const int tab_id,
            const int selection, const int col_id, const char * name );


/*****************************************************************************
    (17)
    returns the domain-id of a type of a column in a table

    ! NULL ptr in self will be ignored !
    ! returns -1 if tab_id/col_id/type_id is out of bounds !
*/
SVDB_EXTERN int CC svdb_typedomain( void * self, const int tab_id,
            const int selection, const int col_id, const int type_id );


/*****************************************************************************
    (18)
    returns the bitsize of a type of a column in a table

    ! NULL ptr in self will be ignored !
    ! returns -1 if tab_id/col_id/type_id is out of bounds !
*/
SVDB_EXTERN int CC svdb_typebits( void * self, const int tab_id,
            const int selection, const int col_id, const int type_id );


/*****************************************************************************
    (19)
    returns the dimensionality of a type of a column in a table

    ! NULL ptr in self will be ignored !
    ! returns -1 if tab_id/col_id/type_id is out of bounds !
*/
SVDB_EXTERN int CC svdb_typedim( void * self, const int tab_id,
            const int selection, const int col_id, const int type_id );


/*****************************************************************************
    (20)
    opens a table of a svdb-object

    ! NULL ptr in self will be ignored !
    ! returns -1 if tab_id is out of bounds !
    NULL or empty string as defline opens all readable columns without a cast
    defline = "READ,QUALITY" opens only these 2 columns (if the table has them)
    defline = "(INSDC:2na:bin)READ" opens columns with a cast
*/
SVDB_EXTERN int CC svdb_open_table( void * self, const int tab_id,
            const char *defline );


/*****************************************************************************
    (21)
    returns the length of the longest column-name that is enabled

    ! NULL ptr in self will be ignored !
    ! returns 0 if tab_id is out of bounds !
*/
SVDB_EXTERN int CC svdb_max_colname_length( void * self, const int tab_id,
            const int selection );


/*****************************************************************************
    (22)
    sets the element-separator-string

    ! NULL ptr in self will be ignored !
    tab_id = -1 ... for all tables
    col_id = -1 ... for all columns
    s      = separator-string to set
*/
SVDB_EXTERN void CC svdb_set_elem_separator( void * self, const int tab_id,
            const int selection, const int col_id, const char * s );


/*****************************************************************************
    (23)
    sets the dimension-separator-string

    ! NULL ptr in self will be ignored !
    tab_id = -1 ... for all tables
    col_id = -1 ... for all columns
    s      = separator-string to set
*/
SVDB_EXTERN void CC svdb_set_dim_separator( void * self, const int tab_id,
            const int selection, const int col_id, const char * s );


/*****************************************************************************
    (24)
    returns if a column is enabled...

    ! NULL ptr in self will be ignored !
*/
SVDB_EXTERN int CC svdb_is_enabled( void * self, const int tab_id,
            const int selection, const int col_id );


/*****************************************************************************
    (25)
    returns how many rows a table has

    ! NULL ptr in self will be ignored !
    ! returns 0 if tab_id is out of bounds !
*/
SVDB_EXTERN unsigned long long int CC svdb_row_range( void * self,
            const int tab_id );


/*****************************************************************************
    (26)
    returns the offset of the first row ( offset to zero )

    ! NULL ptr in self will be ignored !
    ! returns 0 if tab_id is out of bounds !
*/
SVDB_EXTERN signed long long int CC svdb_first_row( void * self,
            const int tab_id );


/*****************************************************************************
    (27)
    returns a data-cell as text

    ! NULL ptr in self/buf will be ignored !
    ! returns 0 if tab_id, col_id, row is out of bounds !
    returns the numbers of characters written into buf, if successful
*/
SVDB_EXTERN int CC svdb_cell( void * self, char * buf, int buflen,
            const int tab_id, const int selection, const int col_id,
            const unsigned long long int row );


/*****************************************************************************
    (28/29)
    returns the row-number of the row where the pattern was found in col

    ! NULL ptr in self/pattern will be ignored !
    ! returns 0xFFFFFFFFFFFFFFFF if tab_id, col_id, row is out of bounds !
    ! returns 0xFFFFFFFFFFFFFFFF if pattern not found... !
*/
SVDB_EXTERN unsigned long long int CC svdb_find_fwd( void * self, const int tab_id,
            const int selection, const int col_id, const unsigned long long int row,
            const int chunksize, const char * pattern );
SVDB_EXTERN unsigned long long int CC svdb_find_bwd( void * self, const int tab_id,
            const int selection, const int col_id, const unsigned long long int row,
            const int chunksize, const char * pattern );


/*****************************************************************************
    (30)
    returns a the id of the meta-root-node of a table

    ! NULL ptr in self will be ignored !
    ! returns -1 if tab_id is out of bounds, or no metadata found !
*/
SVDB_EXTERN int CC svdb_tab_meta_root( void * self, const int tab_id );


/*****************************************************************************
    (31)
    returns the name of a meta-node

    ! NULL ptr in self/buf will be ignored !
    ! returns 0 if tab_id, meta_id is out of bounds !
    returns the numbers of characters written into buf, if successful
*/
SVDB_EXTERN int CC svdb_tab_meta_name( void * self, const int tab_id,
            const int meta_id, char * buf, int buflen );


/*****************************************************************************
    (32)
    returns the length of the value of a meta-node ( in bytes )

    ! NULL ptr in self will be ignored !
    ! returns 0 if tab_id, meta_id is out of bounds !
*/
SVDB_EXTERN int CC svdb_tab_meta_value_len( void * self, const int tab_id,
            const int meta_id );


/*****************************************************************************
    (33)
    returns if the meta-value contains only printable chars

    ! NULL ptr in self will be ignored !
    ! returns 0 if tab_id, meta_id is out of bounds !
*/
SVDB_EXTERN int CC svdb_tab_meta_value_printable( void * self,
            const int tab_id, const int meta_id );


/*****************************************************************************
    (34)
    returns a ptr to the meta-node-value

    ! NULL ptr in self will be ignored !
    ! returns 0 if tab_id, meta_id is out of bounds !
*/
SVDB_EXTERN const char * CC svdb_tab_meta_value_ptr( void * self, const int tab_id,
                                              const int meta_id );


/*****************************************************************************
    (35)
    returns the value of a meta-node as ascii-text (this is a guess)

    ! NULL ptr in self/buf will be ignored !
    ! returns 0 if tab_id, meta_id is out of bounds !
    returns the numbers of characters written into buf, if successful
*/
SVDB_EXTERN int CC svdb_tab_meta_value( void * self, const int tab_id,
            const int meta_id, char * buf, int buflen, int trim );


/*****************************************************************************
    (36)
    returns the number of childs of a meta-node

    ! NULL ptr in self will be ignored !
    ! returns -1 if tab_id, meta_id is out of bounds !
*/
SVDB_EXTERN int CC svdb_tab_meta_child_count( void * self,
            const int tab_id, const int meta_id );


/*****************************************************************************
    (37)
    returns the id of a meta-node child

    ! NULL ptr in self will be ignored !
    ! returns -1 if tab_id, meta_id, child_idx is out of bounds !
*/
SVDB_EXTERN int CC svdb_tab_meta_child_id( void * self, const int tab_id,
            const int meta_id, const int child_idx );





/*****************************************************************************
    (38)

*/
SVDB_EXTERN void * CC svdb_open_fastq( const char * path );


/*****************************************************************************
    (39)

*/
SVDB_EXTERN void CC svdb_close_fastq( void * self );


/*****************************************************************************
    (40)
    returns what the fastq-obj can produce
    0 ... nothing, handle invalid
    1 ... only READ ( that means fasta )
    2 ... READ and QUALITY ( but not spot splitting )
    3 ... READ, QUALITY and READ_START/READ_LEN ( splitted spots )
*/
SVDB_EXTERN int CC svdb_fastq_scope( void * self );


/*****************************************************************************
    (41)
    returns how many reads a fastq-row has
*/
SVDB_EXTERN int CC svdb_fastq_readcount( void * self, const unsigned long long int row );


/*****************************************************************************
    (42)
    produces a fastq-name ( without the prefix... @/+ ) into a buffer
    seq ... 0 the whole unsplitted spot
    seq >   0 the read/part of the spot 1,2
    returns the number of bytes written
*/
SVDB_EXTERN int CC svdb_fastq_name( void * self, char * buf, int buflen,
                                   int seq, const unsigned long long int row );


/*****************************************************************************
    (43)
    produces a fastq-sequence into a buffer
    seq ... 0 the whole unsplitted spot
    seq >   0 the read/part of the spot 1,2
    returns the number of bytes written
*/
SVDB_EXTERN int CC svdb_fastq_sequence( void * self, char * buf, int buflen,
                                       int seq, const unsigned long long int row );

/*****************************************************************************
    (44)
    produces a fastq-quality into a buffer
    seq ... 0 the whole unsplitted spot
    seq >   0 the read/part of the spot 1,2
    returns the number of bytes written
*/
SVDB_EXTERN int CC svdb_fastq_quality( void * self, char * buf, int buflen,
                                      int seq, const unsigned long long int row );


SVDB_EXTERN int CC svdb_fastq_rd_type_available( void * self );

SVDB_EXTERN int CC svdb_fastq_rd_filter_available( void * self );

SVDB_EXTERN int CC svdb_fastq_read_type_is_bio( void * self, int seq, const unsigned long long int row );

SVDB_EXTERN int CC svdb_fastq_read_filter_is_pass( void * self, int seq, const unsigned long long int row );

SVDB_EXTERN unsigned long long int CC svdb_fastq_row_count( void * self );

#ifdef __cplusplus
}
#endif

#endif
