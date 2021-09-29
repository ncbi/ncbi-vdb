/*==============================================================================
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
 *  Author: Kurt Rodarmer
 *
 * ===========================================================================
 *
 */

#pragma once

#include <ncbi/jwk.hpp>

/**
 * @file ncbi/token.hpp
 * @brief Token-based Security
 *
 * Token-based security is an alternate approach to Identity-based security.
 * While the latter is predominant in systems throughout the world, it is
 * not always appropriate for addressing every problem. Token-based security
 * is an alternative that elegantly solves problems that cannot be addressed
 * with Identities.
 */

namespace ncbi
{
    class TokenSet;
    class TokenStore;
    class TokenSetEntry;

    /**
     * @typedef Token
     * @brief our tokens are JWSs or JWEs
     * An opaque data structure containing contextually relevant data
     */
    typedef std :: string Token;

    /**
     * @typedef TokenSetRef
     * @brief a unique reference to a TokenSet
     */
    typedef JWRH < TokenSet > TokenSetRef;

    /**
     * @typedef TokenSetEntry
     * @brief a unique reference to a TokenSetEntry
     */
    typedef JWRH < TokenSetEntry > TokenSetEntryRef;

    /**
     * @typedef TokenStoreRef
     * @brief a unique reference to a TokenStore
     */
    typedef JWRH < TokenStore > TokenStoreRef;


    /**
     * @class TokenStore
     * @brief NCBI Token Management
     * handle to external token storage
     */
    class TokenStore
    {
    public:

        /**
         * @fn connect
         * @brief connect to token storage using authenticated identity token
         * @param authenticated_identity token returned from authentication API
         * @exception AUTHInvalid
         * @exception AUTHExpired
         * @return TokenStore pointer
         */
        static TokenStoreRef connect ( const Token & authenticated_identity );

        /**
         * @fn retrieveTokenSet
         * @brief retrieve TokenSet for authenticated identity
         * @return TokenSet reference
         */
        virtual TokenSetRef retrieveTokenSet () const = 0;

        /**
         * @fn storeTokenSet
         * @brief save changes to global TokenSet for identity
         */
        virtual void storeTokenSet ( const TokenSet & token_set ) = 0;

        virtual ~ TokenStore () noexcept;

    protected:

        TokenStore () noexcept;
    };


    /**
     * @class TOK
     * @brief a factory class for creating TokenSetEntries
     */
    class TOK
    {
    public:

        static TokenSetEntryRef makeTokenSetEntry ( const Token & tok );
        static TokenSetEntryRef makeTokenSetEntry ( const JWKRef & key );
        static TokenSetEntryRef makeTokenSetEntry ( const JWKSetRef & key_set );
    };


    /**
     * @class TokenSetEntry
     * @brief an entry in a token set
     * can represent a Token, a JWK, a JWKSet
     */
    class TokenSetEntry
    {
    public:

        /*=================================================*
         *                TYPE PREDICATES                  *
         *=================================================*/

        /**
         * @fn isTokenSet
         * @return Boolean true if entry is a set of more entries
         */
        virtual bool isTokenSet () const noexcept;

        /**
         * @fn isToken
         * @return Boolean true if entry is a Token
         * a token means a JWT, JWS, JWE, or similarly encapsulated object
         */
        virtual bool isToken () const noexcept;

        /**
         * @fn isJWK
         * @return Boolean true if entry contains a JWK ( see <ncbi/jwk.hpp> )
         */
        virtual bool isJWK () const noexcept;

        /**
         * @fn isJWKSet
         * @return Boolean true if entry contains a JWKSet ( see <ncbi/jwk.hpp> )
         */
        virtual bool isJWKSet () const noexcept;


        /*=================================================*
         *                     SETTERS                     *
         *=================================================*/

        /**
         * @fn setToken
         * @brief change type if necessary to Token and set value
         * @param tok Token string
         * @exception TOKPermViolation
         * @return self reference as a convenience to aid in C++ expressions
         */
        virtual TokenSetEntry & setToken ( const Token & tok );

        /**
         * @fn setJWK
         * @brief change type if necessary to JWK and set value
         * @param key JWK key structure
         * @exception TOKPermViolation
         * @exception TOKNullEntry
         * @return self reference as a convenience to aid in C++ expressions
         */
        virtual TokenSetEntry & setJWK ( const JWK & key );

        /**
         * @fn setJWKSet
         * @brief change type if necessary to JWKSet and set value
         * @param key_set JWKSet key-set structure
         * @exception TOKPermViolation
         * @exception TOKNullEntry
         * @return self reference as a convenience to aid in C++ expressions
         */
        virtual TokenSetEntry & setJWKSet ( const JWKSet & key_set );


        /*=================================================*
         *                     GETTERS                     *
         *=================================================*/

        /**
         * @fn toToken
         * @exception TOKIncompatibleType
         * @return Token
         */
        virtual Token toToken () const;

        /**
         * @fn toJWK
         * @overload returns pointer to non-const JWK
         * @exception TOKIncompatibleType
         * @return JWK pointer
         */
        virtual JWKRef toJWK () const;

        /**
         * @fn toJWK
         * @overload returns pointer to const JWK
         * @exception TOKIncompatibleType
         * @return const JWK pointer
         */
        virtual JWKSetRef toJWKSet () const;


        /*=================================================*
         *                   TYPE CASTS                    *
         *=================================================*/

        /**
         * @fn toTokenSet
         * @overload attempts to downcast from TokenSetEntry to TokenSet
         * @exception TOKBadCast
         * @return TokenSet reference
         * a C++ reference is returned rather than a pointer in order
         * to facilitate certain idiomatic C++ expressions, in addition
         * to the fact that success can never return nullptr.
         */
        virtual TokenSet & toTokenSet ();

        /**
         * @fn toTokenSet
         * @overload attempts to downcast from const TokenSetEntry to const TokenSet
         * @exception TOKBadCast
         * @return TokenSet reference
         * a C++ reference is returned rather than a pointer in order
         * to facilitate certain idiomatic C++ expressions, in addition
         * to the fact that success can never return nullptr.
         */
        virtual const TokenSet & toTokenSet () const;

        /**
         * @fn clone
         * @return creates a deep copy of entry
         */
        virtual TokenSetEntryRef clone () const = 0;

        /**
         * @fn invalidate
         * @brief overwrite potentially sensitive contents in memory
         */
        inline void invalidate () const noexcept
        {
            const_cast < TokenSetEntry* > ( this ) -> invalidateImpl ();
        }

        /**
         * @fn setReadOnly
         * @brief cause object and any clones to be permanently read-only
         * has no effect if an entry is already read-only
         * removes write capability otherwise
         */
        inline void setReadOnly () const noexcept
        {
            const_cast < TokenSetEntry* > ( this ) -> setReadOnlyImpl ();
        }


        /*=================================================*
         *           C++ OPERATOR OVERLOADS                *
         *=================================================*/

        /**
         * fn operator=
         * @overload C++ only Token assignment operator
         * @param tok Token string
         * @exception TOKPermViolation
         * @return self reference as a convenience to aid in C++ expressions
         */
        inline TokenSetEntry & operator = ( const Token & tok )
        {
            return setToken ( tok );
        }

        /**
         * fn operator=
         * @overload C++ only Key assignment operator
         * @param key JWK key structure
         * @exception TOKPermViolation
         * @return self reference as a convenience to aid in C++ expressions
         */
        inline TokenSetEntry & operator = ( const JWK & key )
        {
            return setJWK ( key );
        }

        /**
         * fn operator=
         * @overload C++ only Key-set assignment operator
         * @param key_set JWKSet key-set structure
         * @exception TOKPermViolation
         * @return self reference as a convenience to aid in C++ expressions
         */
        inline TokenSetEntry & operator = ( const JWKSet & key_set )
        {
            return setJWKSet ( key_set );
        }

        /**
         * @fn operator()
         * @overload C++ only cast to Token operator
         * @exception TOKIncompatibleType
         * @return Token entry value
         */
        inline operator Token () const
        {
            return toToken ();
        }

        /**
         * @fn operator()
         * @overload C++ only cast to const JWK * operator
         * @exception TOKIncompatibleType
         * @return const JWK pointer entry value
         */
        inline operator JWKRef () const
        {
            return toJWK ();
        }

        /**
         * @fn operator()
         * @overload C++ only cast to const JWKSet * operator
         * @exception TOKIncompatibleType
         * @return JWKSet pointer entry value
         */
        inline operator JWKSetRef () const
        {
            return toJWKSet ();
        }

        /**
         * @fn operator[]
         * @overload C++ only array operator to access non-const named TokenSetEntry
         * @param path TokenSet access path
         * @exception TOKBadCast if entry is not a TokenSet
         * @exception TOKNoSuchEntry if path does not exist
         * @return non-const TokenSetEntry reference
         */
        inline TokenSetEntry & operator [] ( const std :: string & path );

        /**
         * @fn operator[]
         * @overload C++ only array operator to access const named TokenSetEntry
         * @param path TokenSet access path
         * @exception TOKBadCast if entry is not a TokenSet
         * @exception TOKNoSuchEntry if path does not exist
         * @return const TokenSetEntry reference
         */
        inline const TokenSetEntry & operator [] ( const std :: string & path ) const;

        /**
         * @fn ~TokenSetEntry
         * @brief disposes of dynmically allocated content in derived classes
         */
        virtual ~ TokenSetEntry ();

    protected:

        // implementation of "invalidate" and "setReadOnly"
        virtual void invalidateImpl () noexcept = 0;
        virtual void setReadOnlyImpl () noexcept = 0;

        TokenSetEntry ();
    };


    /**
     * @class TokenSet
     * @brief an unordered set of entries.
     */
    class TokenSet : public TokenSetEntry
    {
    public:

        /*=================================================*
         *                   OVERRIDES                     *
         *=================================================*/

        // type predicate
        virtual bool isTokenSet () const noexcept override
        { return true; }

        // casts
        virtual TokenSet & toTokenSet () override
        { return * this; }
        virtual const TokenSet & toTokenSet () const override
        { return * this; }

        // clone
        virtual TokenSetEntryRef clone () const override
        { return cloneSet () . release (); }


        /*=================================================*
         *             CONTAINER INTERFACE                 *
         *=================================================*/

        /**
         * @fn isEmpty
         * @return Boolean true if set has no members
         */
        bool isEmpty () const noexcept;

        /**
         * @fn shallowCount
         * @return Natural number with the shallow cardinality
         */
        unsigned long int shallowCount () const noexcept;

        /**
         * @fn deepCount
         * @return Natural number with the deep ( recursive ) cardinality
         */
        unsigned long int deepCount () const noexcept;

        /**
         * @fn exists
         * @brief answers whether the indicated entry exists
         * @param name std::string with the path
         * @return Boolean true if entry exists
         */
        bool exists ( const std :: string & path ) const noexcept;

        /**
         * @fn getNames
         * @return std::vector<std::string> of entry names
         */
        std :: vector < std :: string > getNames () const;

        /**
         * @fn addEntry
         * @brief add a new TokenSetEntry
         * @param path std::string with unique entry path
         * @param entry a non-null TokenSetEntry pointer
         * @exception TOKPathInvalid if the path is invalid due to being empty
         * @exception TOKPathInvalid if an element of the path is invalid or there are too many.
         * @exception TOKPathIncorrect if a parent segment of path is not a TokenSet
         * @exception TOKNotTokenSet if path ends in delimiter but entry is not a TokenSet
         * will create missing TokenSets within path if necessary
         */
        void addEntry ( const std :: string & path, const TokenSetEntryRef & entry );
        
        /**
         * @fn setEntry
         * @brief set existing or add new entry
         * @param path std::string with entry path
         * @param entry a non-null TokenSetEntry pointer
         */
        void setEntry ( const std :: string & path, const TokenSetEntryRef & entry );

        /**
         * @fn getEntry
         * @overload non-const accessor
         * @param path std::string with entry path
         * @return TokenSetEntry reference to existing value
         */
        TokenSetEntry & getEntry ( const std :: string & path );

        /**
         * @fn getEntry
         * @overload const accessor
         * @param path std::string with entry path
         * @return const TokenSetEntry reference to existing value
         */
        const TokenSetEntry & getEntry ( const std :: string & path ) const;
        
        /**
         * @fn removeEntry
         * @brief remove and return named entry
         * @param path std::string with entry path
         * @return TokenSetEntryRef with removed entry
         * returns reference to nullptr if not found
         */
        TokenSetEntryRef removeEntry ( const std :: string & path );
        
        /**
         * @fn deleteEntry
         * @brief remove and delete named entry
         * @param path std::string with entry path
         * @return TokenSetEntryRef with removed entry
         * ignored if entry is not found, returning invalid reference.
         */
        inline void deleteEntry ( const std :: string & path )
        {
            TokenSetEntryRef removed = removeEntry ( path );
        }

        /**
         * @fn cloneSet
         * @return creates a deep copy of TokenSet
         * avoids need to typecast a TokenSetRef
         */
        TokenSetRef cloneSet () const;


        /*=================================================*
         *           C++ OPERATOR OVERLOADS                *
         *=================================================*/

        /**
         * @fn operator[]
         * @overload C++ only array operator to access non-const named TokenSetEntry
         * @param path TokenSet access path
         * @exception TOKNoSuchEntry if path does not exist
         * @return non-const TokenSetEntry reference
         */
        inline TokenSetEntry & operator [] ( const std :: string & path )
        {
            return getEntry ( path );
        }

        /**
         * @fn operator[]
         * @overload C++ only array operator to access const named TokenSetEntry
         * @param path TokenSet access path
         * @exception TOKBadCast if entry is not a TokenSet
         * @exception TOKNoSuchEntry if path does not exist
         * @return const TokenSetEntry reference
         */
        inline const TokenSetEntry & operator [] ( const std :: string & path ) const
        {
            return getEntry ( path );
        }

        /**
         * @fn operator =
         * @brief assignment operator
         * @param obj source of contents to clone
         * @return C++ self-reference for use in idiomatic C++ expressions
         * will delete any current contents
         * clones contents of source object.
         */
        TokenSet & operator = ( const TokenSet & ts );

        /**
         * @fn TokenSet
         * @overload copy constructor
         * @param obj source of contents to clone
         * clones contents of source object.
         */
        TokenSet ( const TokenSet & ts );

        /**
         * @fn ~TokenSet
         * @brief deletes any contents and destroys internal structures
         */        
        virtual ~ TokenSet () override;

        
    private:

        void addEntry ( const std :: string & path,
            const TokenSetEntryRef & entry, unsigned int depth );
        void setEntry ( const std :: string & path,
            const TokenSetEntryRef & entry, unsigned int depth );

        void invalidateImpl () noexcept override;
        void setReadOnlyImpl () noexcept override;

        TokenSet ();

        std :: map < std :: string, TokenSetEntryRef > entries;
        bool read_only;

        friend class TokenStore;
    };


    /*=================================================*
     *                 C++ INLINES                     *
     *=================================================*/

    inline
    TokenSetEntry & TokenSetEntry :: operator [] ( const std :: string & path )
    {
        return toTokenSet () . getEntry ( path );
    }

    inline
    const TokenSetEntry & TokenSetEntry :: operator [] ( const std :: string & path ) const
    {
        return toTokenSet () . getEntry ( path );
    }


    /*=================================================*
     *                   EXCEPTIONS                    *
     *=================================================*/
    DECLARE_JWX_MSG_EXCEPTION ( AUTHException, JWX );
    DECLARE_JWX_MSG_EXCEPTION ( AUTHInvalid, AUTHException );
    DECLARE_JWX_MSG_EXCEPTION ( AUTHExpired, AUTHException );
    DECLARE_JWX_MSG_EXCEPTION ( TOKException, JWX );
    DECLARE_JWX_MSG_EXCEPTION ( TOKNotTokenSet, TOKException );
    DECLARE_JWX_MSG_EXCEPTION ( TOKPermViolation, TOKException );
    DECLARE_JWX_MSG_EXCEPTION ( TOKNullEntry, TOKException );
    DECLARE_JWX_MSG_EXCEPTION ( TOKIncompatibleType, TOKException );
    DECLARE_JWX_MSG_EXCEPTION ( TOKBadCast, TOKException );

}
