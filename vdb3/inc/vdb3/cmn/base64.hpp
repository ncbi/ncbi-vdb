/*

  vdb3.cmn.base64

 */

#pragma once

#include <vdb3/kfc/rgn.hpp>
#include <vdb3/kfc/except.hpp>
#include <vdb3/kfc/rsrc-mem.hpp>
#include <vdb3/cmn/buffer.hpp>
#include <vdb3/cmn/string.hpp>


/**
 * @file cmn/base64.hpp
 * @brief useful binary <=> ASCII encoding functions
 *
 * This functionality has become associated with cryptography lately,
 * but there's no need for this to be the case. There are several useful
 * ASCII encodings of binary data. The intention is to transform binary
 * into a character sequence that can be transmitted across other protocols
 * that expect text. Going beyond the base 64 approach to use base 128 or
 * even base 256 ( i.e. binary ) would work if all protocols allowed for
 * binary in the first place...
 */

namespace vdb3
{

    /*=====================================================*
     *                       Base64                        *
     *=====================================================*/

    /**
     * @class Base64
     * @brief Base64 encoding management
     */
    class Base64
    {
    public:
    
        /**
         * @var do_allow_whitespace
         * @brief constant used to allow embedded whitespace in text
         */
        static const bool do_allow_whitespace = true;

        /**
         * @var dont_allow_whitespace
         * @brief constant used to disallow any non-coding characters in text
         */
        static const bool dont_allow_whitespace = false;

        /**
         * @var do_trim_padding
         * @brief constant used to remove '=' padding characters from an encoding
         */
        static const bool do_trim_padding = true;

        /**
         * @var dont_trim_padding
         * @brief constant used to keep padding in encoded output for whole quadruplets
         */
        static const bool dont_trim_padding = false;

        /**
         * encode
         * @brief apply base64 encoding to binary data
         * @param data pointer to start of binary data to encode
         * @param bytes size in bytes of binary data to encode
         * @return base64-encoded String
         */
        String encode ( const RsrcKfc & rsrc, const CRgn & r ) const;

        /**
         * decode
         * @brief remove base64 encoding from binary payload
         * @param base64 the base64-encoded text
         * @param allow_whitespace if true, allow whitespace in text
         * @return Buffer of extracted binary data
         */
        Buffer decode ( const RsrcKfc & rsrc, const String & base64 ) const;

        /**
         * decodeText
         * @brief remove base64 encoding where content is expected to be UTF-8 text
         * @param base64 the base64-encoded text
         * @param allow_whitespace if true, allow whitespace in text
         * @return String with extracted text
         */
        String decodeText ( const RsrcKfc & rsrc, const String & base64 ) const;
    
        /**
         * urlEncode
         * @brief apply base64-url encoding to binary data
         * @param data pointer to start of binary data to encode
         * @param bytes size in bytes of binary data to encode
         * @return base64url-encoded String
         */
        String urlEncode ( const RsrcKfc & rsrc, const CRgn & r ) const;

        /**
         * urlDecode
         * @brief remove base64-url encoding from binary payload
         * @param base64url the base64url-encoded text
         * @param allow_whitespace if true, allow whitespace in text
         * @return Buffer of extracted binary data
         */
        Buffer urlDecode ( const RsrcKfc & rsrc, const String & base64url ) const;
 
        /**
         * urlDecodeText
         * @brief remove base64-url encoding where content is expected to be UTF-8 text
         * @param base64url the base64url-encoded text
         * @param allow_whitespace if true, allow whitespace in text
         * @return String with extracted text
         */
        String urlDecodeText ( const RsrcKfc & rsrc, const String & base64url ) const;

        /**
         * setWhitespacePolicy
         * @brief sets policy around allowing whether whitespace should be allowed
         * @param allow_whitespace is either "do_allow_whitespace" or "dont_allow_whitespace"
         */
        void setWhitespacePolicy ( bool allow_whitespace ) noexcept;

        /**
         * setPaddingPolicy
         * @brief sets policy around padding which maintains quadruplets or not
         * @param trim_padding is either "do_trim_padding" or "dont_trim_padding"
         */
        void setPaddingPolicy ( bool trim_padding ) noexcept;


        /*=================================================*
         *                       C++                       *
         *=================================================*/

        CXX_RULE_OF_SIX_NE ( Base64 );

        /**
         * Basee64
         * @brief constructs with desired policy
         */
        Base64 ( bool allow_whitespace, bool trim_padding ) noexcept;

    private:

        String encode ( const RsrcKfc & rsrc, const CRgn & r, const ASCII encode_table [] ) const;
        Buffer decode ( const RsrcKfc & rsrc, const String & base64, const char decode_table [] ) const;
        static String payloadToUTF8 ( const RsrcKfc & rsrc, const Buffer & b );

        bool allow_whitespace;
        bool trim_padding;
    };
    
    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

}
