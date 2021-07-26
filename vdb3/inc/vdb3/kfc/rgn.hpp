/*

  vdb3.kfc.rgn

 */

#pragma once

#include <vdb3/kfc/defs.hpp>
#include <vdb3/kfc/except.hpp>

/**
 * @file kfc/rgn.hpp
 * @brief accessor to a region of addressable memory
 *
 * A region as described herein is intended to replace pointers in the
 * majority of cases where something like a void* would be used. The
 * very idea of using an address without a size relies upon self-describing
 * data or knowledge of type structure and size. A large number of bugs in
 * C and C++ code stem from this strange idea.
 *
 * A pointer or C++ reference to a strictly sized structure works fine,
 *
 * a pointer to a sequence with an end sentinel - like the NUL-terminated
 * C string - is questionable, especially since a receiver cannot distinguish
 * between an initialized and uninitialized buffer.
 *
 * self-describing structures where the delineation is inline with the data
 * also complicate the ability to create selections of sub-regions. This is
 * one of the great failings of the std::string approach.
 *
 * The region is the simplest possible out-of-line description of a range
 * of virtual memory, and it allows for definitions of several operations.
 *
 * There is no explanation for HOW the region came to be mapped into VAddrSpace,
 * only an assertion THAT it is mapped and addressable. To increase the chances
 * that the described range is actually mapped, creation of a region is restricted
 * to certain implementations.
 */

namespace vdb3
{

    /*=====================================================*
     *                      FORWARDS                       *
     *=====================================================*/

    class CRgn;
    

    /*=====================================================*
     *                         Rgn                         *
     *=====================================================*/
    
    /**
     * @class Rgn
     * @brief describes a region of addressable virtual memory
     *
     * contains a raw VAddr pointer plus a size in bytes
     */
    class  Rgn
    {
    public:

        /*=================================================*
         *                   PREDICATES                    *
         *=================================================*/

        /**
         * isEmpty
         *  @brief predicate for detecting an empty region
         *  @return Boolean true if empty, false if not empty
         */
        inline bool isEmpty () const noexcept
        { return sz == 0; }

        /**
         * contains
         *  @brief test an offset for inclusion in region
         *  @return Boolean true if location is valid for region
         */
        inline bool contains ( size_t loc ) const noexcept
        { return loc < sz; }


        /*=================================================*
         *                   PROPERTIES                    *
         *=================================================*/

        /**
         * size
         *  @brief access the size in bytes of the memory-mapped region
         *  @return number of bytes in the region
         */
        inline size_t size () const noexcept
        { return sz; }

        /**
         * end
         *  @return a location of the exclusive end of the region
         */
        inline size_t end () const noexcept
        { return sz; }


        /*=================================================*
         *                    ACCESSORS                    *
         *=================================================*/

        /**
         * getByte
         *  @param idx quantity in 0..size()-1
         *  @return indexed byte
         */
        byte_t getByte ( size_t idx ) const;


        /*=================================================*
         *                   COMPARISON                    *
         *=================================================*/

        /**
         * equal
         *  @brief test two regions for content equality
         *  @param r a region to compare against self
         *  @return true iff two regions have equal content
         */
        bool equal ( const Rgn & r ) const noexcept;

        /**
         * compare
         *  @brief compare self against another region
         *  @r region to compare against self
         *  @return "self" - "r" => { < Z-, 0, Z+ }
         */
        int compare ( const Rgn & r ) const noexcept;

        /**
         * fwdFind
         * @overload forward search to find a sub-region
         * @param sub the sub-region being sought
         * @return the location of start of sub-region if found, otherwise end of region
         */
        size_t fwdFind ( const Rgn & sub ) const noexcept;

        /**
         * fwdFind
         * @overload forward search to find a single character
         * @param b a byte_t
         * @return the location of start of sub-region if found, otherwise end of region
         */
        size_t fwdFind ( byte_t b ) const noexcept;

        /**
         * revFind
         * @overload reverse search to find a sub-region
         * @param sub the sub-region being sought
         * @return the location of start of sub-region if found, otherwise end of region
         */
        size_t revFind ( const Rgn & sub ) const noexcept;

        /**
         * revFind
         * @overload reverse search to find a single character
         * @param b a byte_t
         * @return the location of start of sub-region if found, otherwise end of region
         */
        size_t revFind ( byte_t b ) const noexcept;
    
        /**
         * findFirstOf
         * @overload forward search to find a character of a set
         * @param cset the set of characters being sought
         * @return the location of character if found, otherwise end of region
         */
        size_t findFirstOf ( const Rgn & cset ) const noexcept;
    
        /**
         * findLastOf
         * @overload reverse search to find a character of a set
         * @param cset the set of characters being sought
         * @return the location of character if found, otherwise end of region
         */
        size_t findLastOf ( const Rgn & cset ) const noexcept;

        /**
         * beginsWith
         * @overload test whether self region begins with sub region
         * @param sub region with subsequence in question
         * @return Boolean true if self begins with "sub"
         */
        bool beginsWith ( const Rgn & sub ) const noexcept;

        /**
         * beginsWith
         * @overload test whether self region begins with character
         * @param b byte_t with value in question
         * @return Boolean true if self begins with "ch"
         */
        bool beginsWith ( byte_t b ) const noexcept;

        /**
         * endsWith
         * @overload test whether self region ends with sub region
         * @param sub region with subsequence in question
         * @return Boolean true if self ends with "sub"
         */
        bool endsWith ( const Rgn & sub ) const noexcept;

        /**
         * endsWith
         * @overload test whether self region ends with character
         * @param b byte_t with value in question
         * @return Boolean true if self ends with "ch"
         */
        bool endsWith ( byte_t b ) const noexcept;


        /*=================================================*
         *           C++ OPERATOR OVERLOADS                *
         *=================================================*/

        /**
         * operator[]
         * @param idx an ordinal byte index into the region
         * @exception IndexOutOfBounds
         * @return a byte at the given index
         */
        inline byte_t operator [] ( count_t idx ) const
        { return getByte ( idx ); }


        inline bool operator < ( const Rgn & t ) const noexcept
        { return compare ( t ) < 0; }
        inline bool operator <= ( const Rgn & t ) const noexcept
        { return compare ( t ) <= 0; }
        inline bool operator == ( const Rgn & t ) const noexcept
        { return equal ( t ); }
        inline bool operator != ( const Rgn & t ) const noexcept
        { return ! equal ( t ); }
        inline bool operator >= ( const Rgn & t ) const noexcept
        { return compare ( t ) >= 0; }
        inline bool operator > ( const Rgn & t ) const noexcept
        { return compare ( t ) > 0; }


    protected:


        /*=================================================*
         *                       C++                       *
         *=================================================*/
        
        /**
         * Rgn
         *  @brief default constructor
         */
        inline Rgn () noexcept
            : a ( nullptr ), sz ( 0 ) {}
        
        /**
         * ~Rgn
         *  @brief trivial destructor
         */
        inline ~ Rgn () noexcept { a = nullptr; sz = 0; }

        /**
         * Rgn
         *  @brief controlled-access primary constructor
         *  @param _a the starting address to a mapped region
         *  @param _sz the size in bytes of the mapped region
         */
        inline Rgn ( const byte_t * _a, size_t _sz ) noexcept
            : a ( const_cast < byte_t * > ( _a ) )
            , sz ( _sz )
        {
        }
        
        byte_t * a;
        size_t sz;

    private:

        inline byte_t * getAddr () { return a; }
        inline const byte_t * getAddr () const { return a; }
    };

    
    /*=====================================================*
     *                         MRgn                        *
     *=====================================================*/
    
    /**
     * @class MRgn
     * @brief describes a region of mutable addressable virtual memory
     *
     * contains a raw VAddr pointer plus a size in bytes
     */
    class  MRgn : public Rgn
    {
    public:

        /*=================================================*
         *                    ACCESSORS                    *
         *=================================================*/

        /**
         * addr
         *  @brief access the starting virtual address of memory-mapped region
         *  @return pointer to byte
         */
        inline byte_t * addr () const noexcept
        { return a; }


        /*=================================================*
         *                     UPDATE                      *
         *=================================================*/

        /**
         * copy
         *  @brief copy bytes from a source rgn
         *  @return subset of dest region that was updated
         */
        MRgn copy ( const CRgn & src ) const noexcept;

        /**
         * fill
         *  @brief set all bytes of region to single value
         */
        void fill ( byte_t val ) const noexcept;


        /*=================================================*
         *               REGION GENERATION                 *
         *=================================================*/

        /**
         * subRgn
         *  @brief create a new mutable region that is a subset of original
         *  @param offset the number of bytes from origin to start of sub-region
         *  @return subset of original region - empty if no intersection
         */
        MRgn subRgn ( size_t offset ) const noexcept;

        /**
         * subRgn
         *  @brief create a new mutable region that is a subset of original
         *  @param offset the number of bytes from origin to start of sub-region
         *  @param bytes the size in bytes of the sub-region intersected with original
         *  @return subset of original region - empty if no intersection
         */
        MRgn subRgn ( size_t offset, size_t bytes ) const noexcept;

        /**
         * intersect
         *  @return intersection between regions
         */
        MRgn intersect ( const MRgn & r ) const noexcept;

        /**
         * join
         *  @return union of regions
         */
        MRgn join ( const MRgn & r ) const;


        /*=================================================*
         *                       C++                       *
         *=================================================*/

        /**
         * MRgn
         *  @brief default constructor
         */
        inline MRgn () noexcept {}
        
        /**
         * ~MRgn
         *  @brief trivial destructor
         */
        inline ~ MRgn () noexcept {}

    private:

        /**
         * MRgn
         *  @brief controlled-access primary constructor
         *  @param _a the starting address to a mapped region
         *  @param _sz the size in bytes of the mapped region
         */
        inline MRgn ( byte_t * _a, size_t _sz ) noexcept
            : Rgn ( _a, _sz ) {}

        friend class RgnFactory;
    };

    
    /*=====================================================*
     *                         CRgn                        *
     *=====================================================*/
    
    /**
     * @class CRgn
     * @brief describes a region of constant addressable virtual memory
     */
    class  CRgn : public Rgn
    {
    public:
        
        /*=================================================*
         *                    ACCESSORS                    *
         *=================================================*/

        /**
         * addr
         *  @brief access the starting virtual address of memory-mapped region
         *  @return pointer to constant byte
         */
        inline const byte_t * addr () const noexcept
        { return a; }

        
        /*=================================================*
         *               REGION GENERATION                 *
         *=================================================*/

        /**
         * subRgn
         *  @brief create a new mutable region that is a subset of original
         *  @param offset the number of bytes from origin to start of sub-region
         *  @return subset of original region - empty if no intersection
         */
        CRgn subRgn ( size_t offset ) const noexcept;

        /**
         * subRgn
         *  @brief create a new mutable region that is a subset of original
         *  @param offset the number of bytes from origin to start of sub-region
         *  @param bytes the size in bytes of the sub-region intersected with original
         *  @return subset of original region - empty if no intersection
         */
        CRgn subRgn ( size_t offset, size_t bytes ) const noexcept;

        /**
         * intersect
         *  @return intersection between regions
         */
        CRgn intersect ( const CRgn & r ) const noexcept;

        /**
         * join
         *  @return union of regions
         */
        CRgn join ( const CRgn & r ) const;
        

        /*=================================================*
         *                       C++                       *
         *=================================================*/

        /**
         * operator=
         *  @brief assignment from mutable region operator
         *  @param r a mutable source region
         *  @return C++ reference to updated CRgn
         */
        inline CRgn & operator = ( const MRgn & r ) noexcept
        {
            Rgn :: operator = ( r );
            return self;
        }

        /**
         * CRgn
         *  @brief init from mutable region constructor
         *  @param r the mutable source region
         */
        inline CRgn ( const MRgn & r ) noexcept
            : Rgn ( r )
        {
        }
        
        /**
         * CRgn
         *  @brief default constructor
         */
        inline CRgn () noexcept {}
        
        /**
         * ~CRgn
         *  @brief trivial destructor
         */
        inline ~ CRgn () noexcept {}

    private:

        /**
         * CRgn
         *  @brief controlled-access primary constructor
         *  @param _a the starting address to a mapped region
         *  @param _sz the size in bytes of the mapped region
         */
        inline CRgn ( const byte_t * _a, size_t _sz ) noexcept
            : Rgn ( _a, _sz ) {}

        friend class RgnFactory;
    };

   
    /*=====================================================*
     *                      RgnFactory                     *
     *=====================================================*/

    /**
     * @class RgnFactory
     * @brief a RH mixin class giving an implementation
     *  the ability to create an MRgn and/or a CRgn from
     *  a raw pointer and size.
     */
    class RgnFactory
    {
    protected:

        template < class T = byte_t >
        static inline MRgn makeMRgn ( T * a, size_t sz ) noexcept
        { return MRgn ( reinterpret_cast < byte_t * > ( a ), sz ); }

        template < class T = byte_t >
        static inline CRgn makeCRgn ( const T * a, size_t sz ) noexcept
        { return CRgn ( reinterpret_cast < const byte_t * > ( a ), sz ); }
    };
    


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

    DECLARE_EXCEPTION ( NoCommonRgnException, BoundsException );
}
