/*

  vdb3.kfc.except


 */

#pragma once

#include <vdb3/kfc/xp.hpp>

#include <string>

/**
 * @file kfc/except.hpp
 * @brief Exception classes
 *
 *  A common exception family for this library's exception types.
 */

namespace vdb3
{

    /*=====================================================*
     *                      Exception                      *
     *=====================================================*/
    
    /**
     * @class Exception
     * @brief base class for errors
     */
    class Exception
    {
    public:

        /*=================================================*
         *                     GETTERS                     *
         *=================================================*/

        /**
         * what
         * @return const XMsg with formatted explanation of "what went wrong"
         */
        const XMsg what () const noexcept;

        /**
         * problem
         * @return const XMsg of "what is the problem"
         */
        const XMsg problem () const noexcept;

        /**
         * context
         * @return const XMsg of "in what context did it occur"
         */
        const XMsg context () const noexcept;

        /**
         * cause
         * @return const XMsg of "what may have caused the problem"
         */
        const XMsg cause () const noexcept;

        /**
         * suggestion
         * @return const XMsg of "how can I fix/avoid the problem"
         */
        const XMsg suggestion () const noexcept;

        /**
         * file
         * @return const XMsg of file name
         */
        const XMsg file () const noexcept;

        /**
         * line
         * @return line number where object was constructed
         */
        unsigned int line () const noexcept;

        /**
         * function
         * @return const XMsg of simple function name
         */
        const XMsg function () const noexcept;

        /**
         * status
         * @return ReturnCodes for process exit status
         */
        ReturnCodes status () const noexcept;


        /*=================================================*
         *                    C++ STUFF                    *
         *=================================================*/
        
        /**
         * Exception
         * @brief constructor
         * @param XP with all parameters
         */
        explicit Exception ( const XP & params ) noexcept;

        CXX_RULE_OF_FOUR_NE ( Exception );
        
        /**
         * Exception
         * @overload default constructor
         */
        Exception () = delete;

        /**
         * ~Exception
         * @brief destroys message storage strings
         */
        virtual ~ Exception () noexcept;
        
    private:

        // preserved function pointers on call stack
        void * callstack [ 128 ];

        // text buffers
        mutable XBuf prob_msg;
        mutable XBuf ctx_msg;
        mutable XBuf cause_msg;
        mutable XBuf suggest_msg;

        // constants around point of origin
        const UTF8 * file_name;
        const ASCII * func_name;
        unsigned int lineno;
        
        int stack_frames;
        ReturnCodes rc;

        friend class XBackTrace;
    };

    //!< OS-specific error message
    XMsg syserr ( int err ) noexcept;


    /**
     * @def DECLARE_EXCEPTION
     * @brief macro for easy declaration of exception classes
     */
#define DECLARE_EXCEPTION( class_name, super_class )                    \
    struct class_name : super_class                                     \
    {                                                                   \
        class_name ( const vdb3 :: Exception & x )                      \
            : super_class ( x )                                         \
        {                                                               \
        }                                                               \
        class_name ( const vdb3 :: XP & params )                        \
            : super_class ( params )                                    \
        {                                                               \
        }                                                               \
    }


    /*=====================================================*
     *                     EXCEPTIONS                      *
     *=====================================================*/

    DECLARE_EXCEPTION ( LogicException, Exception );
    DECLARE_EXCEPTION ( RuntimeException, Exception );

    DECLARE_EXCEPTION ( InternalError, LogicException );
    DECLARE_EXCEPTION ( UnimplementedError, InternalError );
    DECLARE_EXCEPTION ( InsufficientBuffer, InternalError );
    DECLARE_EXCEPTION ( NullReferenceException, InternalError );
    DECLARE_EXCEPTION ( InvalidArgument, LogicException );
    DECLARE_EXCEPTION ( NullArgumentException, InvalidArgument );
    DECLARE_EXCEPTION ( InvalidStateException, LogicException );
    DECLARE_EXCEPTION ( IteratorInvalid, InvalidStateException );
    DECLARE_EXCEPTION ( IteratorsUnrelated, LogicException );
    DECLARE_EXCEPTION ( IncompatibleTypeException, LogicException );
    DECLARE_EXCEPTION ( UnsupportedException, LogicException );
    DECLARE_EXCEPTION ( InternalPolicyViolation, InternalError );

    DECLARE_EXCEPTION ( EmptyException, RuntimeException );
    DECLARE_EXCEPTION ( BoundsException, RuntimeException );
    DECLARE_EXCEPTION ( OverflowException, RuntimeException );
    DECLARE_EXCEPTION ( UnderflowException, RuntimeException );
    DECLARE_EXCEPTION ( BadCastException, RuntimeException );
    DECLARE_EXCEPTION ( ConstraintViolation, RuntimeException );
    DECLARE_EXCEPTION ( SizeViolation, ConstraintViolation );
    DECLARE_EXCEPTION ( LengthViolation, ConstraintViolation );
    DECLARE_EXCEPTION ( PermissionViolation, ConstraintViolation );
    DECLARE_EXCEPTION ( UniqueConstraintViolation, ConstraintViolation );
    DECLARE_EXCEPTION ( ConstSizeConstraintViolation, LogicException );
    DECLARE_EXCEPTION ( NotFoundException, RuntimeException );
    DECLARE_EXCEPTION ( InvalidDataException, RuntimeException );
    DECLARE_EXCEPTION ( InvalidInputException, InvalidDataException );
    DECLARE_EXCEPTION ( PolicyViolation, ConstraintViolation );
    DECLARE_EXCEPTION ( TransferIncomplete, RuntimeException );
    DECLARE_EXCEPTION ( ArithmeticException, RuntimeException );
}
