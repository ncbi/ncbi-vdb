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

#ifndef _hpp_simple_sra_
#define _hpp_simple_sra_

#include <stdint.h>
#include <string>
#include <vector>
#include <exception>

/* NOTES

   01. may need our own String abstraction to represent
       pointers into cursor cache rather than copies.
       std::string may be too heavyweight, but then
       the same problem exists in Java... then again,
       maybe it's just that we shouldn't use String for
       bases...

 */

extern "C"
{
    /*----------------------------------------------------------------------
     * forwards and typedefs
     */
    struct SRA_Spot;
    struct SRA_Object;
    struct SRA_Sequence;
    struct SRA_Reference;
    struct SRA_Alignment;
    struct SAM_Flags;
    struct SAM_Headers;
    struct SAM_Alignment;
}

namespace sra
{

    /*----------------------------------------------------------------------
     * forwards and typedefs
     */
    typedef :: std :: string String;
    typedef :: std :: exception Exception;
    typedef :: SRA_Spot * SRASpotRef;
    typedef :: SRA_Object * SRAObjectRef;
    typedef :: SRA_Sequence * SRASequenceRef;
    typedef :: SRA_Reference * SRAReferenceRef;
    typedef :: SRA_Alignment * SRAAlignmentRef;
    typedef :: SAM_Flags * SAMFlagsRef;
    typedef :: SAM_Headers * SAMHeadersRef;
    typedef :: SAM_Alignment * SAMAlignmentRef;


    /*----------------------------------------------------------------------
     * SRAException
     *  all exceptions generated within sra :: simple are subclasses
     */
    class SRAException : public :: std :: exception
    {
    public:

        const char * what () const
            throw ();
        
        SRAException ()
            throw ();

        SRAException ( const String & message )
            throw ();

        SRAException ( const char * fmt, ... )
            throw ();

    public:

        // C++ support

        SRAException ( const SRAException & x )
            throw ();
        SRAException & operator = ( const SRAException & x )
            throw ();

        virtual ~ SRAException ()
            throw ();

    private:

        String msg;
    };


    /*----------------------------------------------------------------------
     * StringRef
     *  behaves similarly to a String except that it points to
     *  data held within a cache whenever possible.
     */
    class StringRef
    {
    public:

        const char * data () const
            throw ();

        size_t length () const
            throw ();

        String toString () const
            throw ( SRAException );

    public:
        StringRef ( const StringRef & s )
            throw ();
        StringRef & operator = ( const StringRef & s )
            throw ();

        virtual ~ StringRef ()
            throw ();

    private:

        const char * str;
        size_t len;

        // TBD - additional opaque stuff for C object references
    };

    /*----------------------------------------------------------------------
     * SRASequence
     * SRASequenceIterator
     *  a nucleotide sequence
     */
    class SRASequence
    {
    public:

        /* name
         *  returns a textual identifier if available
         *  or a numeric version otherwise
         */
        String name () const
            throw ( SRAException );

        /* length
         *  return the number of bases in sequence
         */
        uint64_t length () const
            throw ( SRAException );

        /* bases
         *  returns bases using IUPAC character representation
         */
        String bases () const
            throw ( SRAException );

        /* isReverse
         *  true if reverse read
         */
        bool isReverse () const
            throw ( SRAException );

        /* colorSpaceNative
         *  returns true if the native sequence is in color-space
         */
        bool colorSpaceNative () const
            throw ( SRAException );

        /* hasColorSpace
         *  returns true if sequence can return color-space
         */
        bool hasColorSpace () const
            throw ();

        /* colorSpaceKey
         *  returns key ( starting base ) for first read
         */
        char colorSpaceKey () const
            throw ( SRAException );

        /* colorSpace
         *  returns color-space representation
         *  NB - may fail on some sequences
         */
        String colorSpace () const
            throw ( SRAException );

        /* qualities
         *  returns per-base/color phred quality scores
         */
        std :: vector < uint8_t > qualities () const
            throw ( SRAException );

#if 0
        // THESE ARE QUESTIONABLE...
        // CONVENIENT PLACES TO ACCUMMULATE CRUFT, BUT
        // DO WE WANT TO KEEP DRAGGING IT ALONG? AND
        // HOW MUCH FACTORIZATION DO WE WANT IN API?

        /* isTechnical
         *  true if the sequence represents a technical read
         */
        bool isTechnical () const
            throw ( SRAException );

        /* hasPrimaryAlignment
         *  returns true iff any part of sequence is aligned,
         *  and the sequence is capable of returning an SRAAlignment
         */
        bool hasPrimaryAlignment () const
            throw ();

        /* primaryAlignment
         *  if the sequence is aligned and the alignment is available
         *  assign to an SRAAlignment
         */
        SRAAlignmentRef primaryAlignment () const
            throw ( SRAException );

#endif

    public:

        // C++ support

        SRASequence & operator = ( SRASequenceRef s )
            throw ();
        SRASequence ( SRASequenceRef s )
            throw ();

        SRASequence & operator = ( const SRASequence & s )
            throw ( SRAException );
        SRASequence ( const SRASequence & s )
            throw ( SRAException );

        ~ SRASequence ()
            throw ();

    protected:

        SRASequenceRef self;
    };

    class SRASequenceIterator : public SRASequence
    {
    public:

        /* next
         *  advance to the next sequence
         *  returns false if no more sequences are available
         */
        bool next ()
            throw ( SRAException );


    public:

        // C++ support

        SRASequenceIterator ( SRASequenceRef r )
            throw ();

        SRASequenceIterator & operator = ( const SRASequenceIterator & i )
            throw ( SRAException );
        SRASequenceIterator ( const SRASequenceIterator & i )
            throw ( SRAException );

        ~ SRASequenceIterator ()
            throw ();

    private:
        SRASequence & operator = ( const SRASequence & s )
            throw ( SRAException );
        SRASequenceIterator & operator = ( SRASequenceRef s )
            throw ();
    };


    /*----------------------------------------------------------------------
     * SRASpot
     * SRASpotIterator
     *  a complete machine read,
     *  including technical and biological segments
     */
    class SRASpot : public SRASequence
    {
    public:

        /* platform
         *  return platform name as string
         */
        String platform () const
            throw ( SRAException );

        /* spotGroup
         *  returns a spot group name if known
         *  returns an empty string otherwise
         */
        String spotGroup () const
            throw ( SRAException );

        /* id
         *  returns the spot's numeric identifier
         */
        int64_t id () const
            throw ( SRAException );

        /* numReads
         *  return the number of individual sub-sequences
         */
        uint32_t numReads () const
            throw ( SRAException );

        /* reads
         *  return all reads - mixed technical and biological
         */
        SRASequenceIterator reads () const
            throw ( SRAException );

        /* numBioReads
         *  the number of biological reads
         */
        uint32_t numBioReads () const
            throw ( SRAException );

        /* bioReads
         *  return only biological reads
         *  if "trimmed" is true, apply quality clipping if indicated
         */
        SRASequenceIterator bioReads ( bool trimmed = false ) const
            throw ( SRAException );

    public:

        // C++ support

        SRASpot & operator = ( SRASpotRef r )
            throw ();
        SRASpot ( SRASpotRef r )
            throw ();

        SRASpot & operator = ( const SRASpot & s )
            throw ( SRAException );
        SRASpot ( const SRASpot & s )
            throw ( SRAException );

        ~ SRASpot ()
            throw ();

    protected:
        SRASpotRef spot;
    };

    class SRASpotIterator : public SRASpot
    {
    public:

        /* next
         *  advance to the next spot
         *  returns false if no more are available
         */
        bool next ()
            throw ( SRAException );


        SRASpotIterator ( SRASpotRef r )
            throw ();

        SRASpotIterator & operator = ( const SRASpotIterator & i )
            throw ( SRAException );
        SRASpotIterator ( const SRASpotIterator & i )
            throw ( SRAException );

        ~ SRASpotIterator ()
            throw ();

    private:
        SRASpot & operator = ( const SRASpot & s )
            throw ( SRAException );
        SRASpotIterator & operator = ( SRASpotRef r )
            throw ();
    };


    /*----------------------------------------------------------------------
     * SRAReference
     * SRAReferenceIterator
     *  a reference sequence
     */
    class SRAReference : public SRASequence
    {
    public:

        /* name
         *  returns a textual identifier if available
         *  or a numeric version otherwise
         */
        String name () const
            throw ( SRAException );

        /* accession
         *  TBD - explain
         */
        String accession () const
            throw ( SRAException );

        /* isCircular
         */
        bool isCircular () const
            throw ( SRAException );

        /* baseCount
         */
        uint64_t baseCount () const
            throw ( SRAException );

        /* bases
         *  returns bases using IUPAC character representation
         *
         *  "first" is a zero-based starting position,
         *  "length" gives the total number of bases, which will be limited
         *   to the reference boundaries unless circular
         */
        String bases ( uint64_t first, uint64_t length ) const
            throw ( SRAException );

    public:

        // C++ support

        SRAReference & operator = ( SRAReferenceRef r )
            throw ();
        SRAReference ( SRAReferenceRef r )
            throw ();
        
        SRAReference & operator = ( const SRAReference & r )
            throw ( SRAException );
        SRAReference ( const SRAReference & r )
            throw ( SRAException );

        ~ SRAReference ()
            throw ();

    protected:

        SRAReferenceRef self;
    };

    class SRAReferenceIterator : public SRAReference
    {
    public:

        /* next
         *  advance to the next reference
         *  returns false if no more are available
         */
        bool next ()
            throw ( SRAException );

    public:

        // C++ support

        SRAReferenceIterator ( SRAReferenceRef r )
            throw ();

        SRAReferenceIterator & operator = ( const SRAReferenceIterator & i )
            throw ( SRAException );
        SRAReferenceIterator ( const SRAReferenceIterator & i )
            throw ( SRAException );

        ~ SRAReferenceIterator ()
            throw ();

    private:
        SRAReference & operator = ( const SRAReference & r )
            throw ( SRAException );
        SRAReferenceIterator & operator = ( SRAReferenceRef r )
            throw ();
    };


    /*----------------------------------------------------------------------
     * SRAAlignment
     * SRAAlignmentIterator
     *  a relationship between two sequences
     */
    class SRAAlignment
    {
    public:

        /* querySequence
         *  get the query sequence
         *  this is generally the sequence aligned against the reference
         */
        SRASequence querySequence () const
            throw ( SRAException );

        /* reference
         *  get the reference sequence
         */
        SRAReference reference () const
            throw ( SRAException );
        
        /* referenceStart
         *  returns the alignment's zero-based starting coordinate on reference
         */
        uint64_t referenceStart () const
            throw ( SRAException );

        /* referenceLength
         *  returns the alignment's projected length upon reference
         */
        uint64_t referenceLength () const
            throw ( SRAException );

        /* mappingQuality
         *  how closely aligned are the sequences
         */
        int64_t mappingQuality () const
            throw ( SRAException );

        /* hasMate
         */
        bool hasMate () const
            throw ();

        /* mate
         *  retrieve mate if available
         */
        SRAAlignmentRef mate () const
            throw ( SRAException );

        /* need to tell spot-id and read-idx of sequence
           this could be available on the sequence object itself, but isn't.
           it's very cheap in our case.
        */

        /* OTHER STUFF
           read group ( should be modeled? should be from sequence? )
           is read paired
           is pair proper
           is read on negative strand
           is ( or is not ) primary alignment
           is duplicate
           is mate unmapped
           is mate on negative strand
           mate alignment start
        */

    public:

        // C++ support

        SRAAlignment & operator = ( SRAAlignmentRef a )
            throw ();
        SRAAlignment ( SRAAlignmentRef a )
            throw ();

        SRAAlignment & operator = ( const SRAAlignment & a )
            throw ( SRAException );
        SRAAlignment ( const SRAAlignment & a )
            throw ( SRAException );

        ~ SRAAlignment ()
            throw ();

    protected:

        SRAAlignmentRef self;
    };

    class SRAAlignmentIterator : public SRAAlignment
    {
    public:

        /* next
         *  advance to the next alignment
         */
        bool next ()
            throw ( SRAException );

    public:

        // C++ support

        SRAAlignmentIterator ( SRAAlignmentRef a )
            throw ();

        SRAAlignmentIterator & operator = ( const SRAAlignmentIterator & i )
            throw ( SRAException );
        SRAAlignmentIterator ( const SRAAlignmentIterator & i )
            throw ( SRAException );

        ~ SRAAlignmentIterator ()
            throw ();

    private:
        SRAAlignment & operator = ( const SRAAlignment & a )
            throw ( SRAException );
        SRAAlignmentIterator & operator = ( SRAAlignmentRef a )
            throw ();
    };


    /*----------------------------------------------------------------------
     * SRAAlignmentDetail
     *  gives raw information about relationship between sequences
     *  from the point of view of the query
     */
    class SRAAlignmentDetail
    {
    public:

        /* hasOffset
         *  a vector of boolean values corresponding to each query base
         *  "false" means that there is no offset to apply to alignment
         *  "true" means that there is an offset that must be applied
         *  to the sequence alignment.
         */
        std :: vector < bool > hasOffset () const
            throw ( SRAException );

        /* offset
         *  a packed vector of alignment offsets to be applied to the
         *  query position before handling mismatches ( see documentation ).
         *  NB - this is a signed quantity and can produce a negative
         *  alignment coordinate in certain cases.
         */
        std :: vector < int32_t > offset () const
            throw ( SRAException );

        /* hasMismatch
         *  a vector of boolean values corresponding to each query base
         *  "false" means that the query sequence matches the reference
         *  "true" means there is a mismatch, and the actual query base
         *  may be obtained from the mismatch vector.
         */
        std :: vector < bool > hasMismatch () const
            throw ( SRAException );

        /* mismatch
         *  a packed vector of query bases that differ from reference
         *  there is one base for every "true" value in hasMismatch
         */
        std :: vector < char > mismatch () const
            throw ( SRAException );
            
        // C++ constructor from alignment

        SRAAlignmentDetail & operator = ( const SRAAlignment & a )
            throw ( SRAException );
        SRAAlignmentDetail ( const SRAAlignment & a )
            throw ( SRAException );

    public:

        // C++ support

        SRAAlignmentDetail & operator = ( SRAAlignmentRef r )
            throw ();
        SRAAlignmentDetail ( SRAAlignmentRef r )
            throw ();

        SRAAlignmentDetail & operator = ( const SRAAlignmentDetail & d )
            throw ( SRAException );
        SRAAlignmentDetail ( const SRAAlignmentDetail & d )
            throw ( SRAException );

        ~ SRAAlignmentDetail ()
            throw ();

    private:

        SRAAlignmentRef self;
    };


    /*----------------------------------------------------------------------
     * SRAObject
     *  represents an SRA-capable object
     */
    class SRAObject
    {
    public:

        /* constructor
         *  tries to locate object described by "spec"
         *  does not fully OPEN object
         */
        SRAObject ( const String & spec )
            throw ( SRAException );

        /* combine constructor
         *  combines two objects into a single object
         */
        SRAObject ( const SRAObject & a, const SRAObject & b )
            throw ( SRAException );

        /* slice
         *  create a sub-selection of object
         *  the sub-selection is based upon reference spec
         *  and/or coordinates within a reference
         */
        SRAObjectRef slice ( const String & referenceSpec ) const
            throw ( SRAException );
        SRAObjectRef slice ( uint64_t reference_start,
                uint64_t reference_length, bool fully_contained ) const
            throw ( SRAException );

        /* open - IDEMPOTENT
         *  causes object to create internal resources
         */
        void open ()
            throw ( SRAException );

        /* close
         *  releases underlying resources
         *  does NOT delete object
         */
        void close ()
            throw ();

        /* getReferences
         *  returns an iterator of all references included within the object
         */
        SRAReferenceIterator getReferences () const
            throw ( SRAException );

        /* primaryAlignments
         *  return an iterator of primary alignments
         *  ordering is based upon object properties
         */
        SRAAlignmentIterator primaryAlignments () const
            throw ( SRAException );
        SRAAlignmentIterator primaryAlignments ( uint64_t startIdx, uint64_t count ) const
            throw ( SRAException );

        /* secondaryAlignments
         *  return an iterator of secondary alignments
         *  ordering is based upon object properties
         */
        SRAAlignmentIterator secondaryAlignments () const
            throw ( SRAException );
        SRAAlignmentIterator secondaryAlignments ( uint64_t startIdx, uint64_t count ) const
            throw ( SRAException );

        /* allAlignments
         *  return an iterator of combined primary and secondary alignments
         *  ordering is based upon object properties
         */
        SRAAlignmentIterator allAlignments () const
            throw ( SRAException );


        /* alignedSequences
         *  return an iterator of aligned sequences
         *  ordering is based upon object properties
         */
        SRASequenceIterator alignedSequences () const
            throw ( SRAException );
        SRASequenceIterator alignedSequences ( uint64_t startIdx, uint64_t count ) const
            throw ( SRAException );

        /* unalignedSequences
         *  return an iterator of unaligned sequences
         */
        SRASequenceIterator unalignedSequences () const
            throw ( SRAException );
        SRASequenceIterator unalignedSequences ( uint64_t startIdx, uint64_t count ) const
            throw ( SRAException );

        /* allSequences
         *  return an iterator of all sequences
         */
        SRASequenceIterator allSequences () const
            throw ( SRAException );


        /* alignedSpots
         *  return an iterator of fully or partially aligned spots
         */
        SRASpotIterator alignedSpots ( bool partially_aligned ) const
            throw ( SRAException );
        SRASpotIterator alignedSpots ( uint64_t startIdx, uint64_t count, bool partially_aligned ) const
            throw ( SRAException );

        /* unalignedSpots
         *  return an iterator of fully unaligned spots
         */
        SRASpotIterator unalignedSpots () const
            throw ( SRAException );
        SRASpotIterator unalignedSpots ( uint64_t startIdx, uint64_t count ) const
            throw ( SRAException );

        /* allSpots
         *  return an iterator of all spots
         */
        SRASpotIterator allSpots () const
            throw ( SRAException );
        SRASpotIterator allSpots ( uint64_t startIdx, uint64_t count ) const
            throw ( SRAException );

    public:

        // C++ support

        SRAObject & operator = ( SRAObjectRef o )
            throw ();
        SRAObject ( SRAObjectRef o )
            throw ();

        SRAObject & operator = ( const SRAObject & o )
            throw ( SRAException );
        SRAObject ( const SRAObject & o )
            throw ( SRAException );

        ~ SRAObject ()
            throw ();

    protected:

        SRAObjectRef self;
    };


    /*----------------------------------------------------------------------
     * SAMAlignment
     *  represents data in a SAM/BAM record
     */
    class SAMAlignment
    {
    public:

        /* qName
         *  query-template name
         */
        String qName () const
            throw ( SRAException );

        /* rName
         *  reference-sequence name
         */
        String rName () const
            throw ( SRAException );

        /* pos
         *  1-based leftmost mapping position
         *  starting position on reference
         */
        uint32_t pos () const
            throw ( SRAException );

        /* mapQ
         *  mapping quality
         */
        uint8_t mapQ () const
            throw ( SRAException );

        /* cigar
         *  cigar-string
         *  a quasi-run-length-encoded representation of alignment traceback
         */
        String cigar () const
            throw ( SRAException );

        /* rNext
         *  ref. name of the mate/next segment
         */
        String rNext () const
            throw ( SRAException );

        /* pNext
         *  ref. position of the mate/next segment
         */
        uint32_t pNext () const
            throw ( SRAException );

        /* tLen
         *  observed template length
         */
        int32_t tLen () const
            throw ( SRAException );

        /* seq
         *  IUPAC bases or color-space
         */
        String seq () const
            throw ( SRAException );

        /* qual
         *  returns per-base/color phred-33 quality scores
         *  NB - phred-33 is an ASCII encoding using offset of 33
         */
        String qual () const
            throw ( SRAException );


        // conversion from SRA alignment

        SAMAlignment & operator = ( const SRAAlignment & a )
            throw ();
        SAMAlignment ( const SRAAlignment & a )
            throw ();

    public:

        // C++ support

        SAMAlignment & operator = ( SAMAlignmentRef r )
            throw ();
        SAMAlignment ( SAMAlignmentRef r )
            throw ();

        SAMAlignment & operator = ( const SAMAlignment & a )
            throw ( SRAException );
        SAMAlignment ( const SAMAlignment & a )
            throw ( SRAException );

        ~ SAMAlignment ()
            throw ();

    protected:

        SAMAlignmentRef self;
    };

    class SAMAlignmentIterator : public SAMAlignment
    {
    public:

        /* next
         *  advance to the next alignment
         */
        bool next ()
            throw ( SRAException );

        // conversion from SRA alignment iterator

        SAMAlignmentIterator & operator = ( const SRAAlignmentIterator & i )
            throw ( SRAException );
        SAMAlignmentIterator ( const SRAAlignmentIterator & i )
            throw ( SRAException );

    public:

        // C++ support

        SAMAlignmentIterator ( SAMAlignmentRef r )
            throw ();

        SAMAlignmentIterator & operator = ( const SAMAlignmentIterator & i )
            throw ( SRAException );
        SAMAlignmentIterator ( const SAMAlignmentIterator & i )
            throw ( SRAException );

        ~ SAMAlignmentIterator ()
            throw ();

    private:
        SAMAlignment & operator = ( const SRAAlignment & a )
            throw ();
        SAMAlignment & operator = ( const SAMAlignment & a )
            throw ();
        SAMAlignmentIterator & operator = ( SAMAlignmentRef r )
            throw ();
    };


    /*----------------------------------------------------------------------
     * SAMHeaders
     *  attempts to assemble SAM header information
     */
    class SAMHeaders
    {
    public:

        /* toString
         *  generate headers as a text object
         */
        String toString () const
            throw ( SRAException );

        /* TBD
         * list of readgroups
         */

        // extraction from an object
        SAMHeaders & operator = ( const SRAObject & o )
            throw ( SRAException );
        SAMHeaders ( const SRAObject & o )
            throw ( SRAException );

    public:

        // C++ support

        SAMHeaders & operator = ( const SAMHeaders & h )
            throw ();
        SAMHeaders ( const SAMHeaders & h )
            throw ();

        ~ SAMHeaders ()
            throw ();

    private:

        SAMHeadersRef self;
    };


    /*----------------------------------------------------------------------
     * SAMFlags
     *  extract "SAM flags" bits/word from an alignment
     */
    class SAMFlags
    {
    public:

        /* multipleSegments ( COSTLY )
         *  template having multiple segments in sequencing
         */
        bool multipleSegments () const
            throw ( SRAException );

        /* properlyAligned ( VERY EXPENSIVE )
         *  each segment properly aligned according to the aligner
         *  all the reads are aligned to the same chromosome
         */
        bool properlyAligned () const
            throw ( SRAException );

        /* unmapped
         *  segment unmapped
         */
        bool unmapped () const
            throw ( SRAException );

        /* mateUnmapped ( COSTLY )
         *  next segment in the template unmapped
         */
        bool mateUnmapped () const
            throw ( SRAException );

        /* reverseComp
         *  SEQ being reverse complemented
         */
        bool reverseComp () const
            throw ( SRAException );

        /* mateReverseComp ( VERY EXPENSIVE )
         *  SEQ of the next segment in the template being reversed
         */
        bool mateReverseComp () const
            throw ( SRAException );

        /* isFirst
         *  the first segment in the template
         */
        bool isFirst () const
            throw ( SRAException );

        /* isLast
         *  the last segment in the template
         */
        bool isLast () const
            throw ( SRAException );

        /* isSecondary
         *  secondary alignment
         */
        bool isSecondary () const
            throw ( SRAException );

        /* failsQualityControl ( COSTLY )
         *  not passing quality controls
         */
        bool failsQualityControl () const
            throw ( SRAException );

        /* isDuplicate ( COSTLY )
         *  PCR or optical duplicate
         */
        bool isDuplicate () const
            throw ( SRAException );

        /* bits ( VERY EXPENSIVE )
         *  access flags as bit-field
         *  NB - may incur performance penalty
         *  unless needing all fields, it may be
         *  cheaper to access bit-fields individually
         */
        uint16_t bits () const
            throw ( SRAException );

    public:

    private:

        SAMAlignmentRef self;
    };
}

#endif /* _hpp_simple_sra_ */
