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

#include <windows.h>
#include <WinBase.h>
#include <AccCtrl.h>
#include <AclApi.h>
#include <WinNT.h>

#include <klib/container.h>
#include <klib/rc.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <kproc/lock.h>

#include <xfs/editors.h>
#include <xfs/perm.h>

#include <sysalloc.h>
#include <stdlib.h>
#include <stdio.h>

#include "zehr.h"

/*)))
 ///    This file contains security related stuff. Main goal is to 
 \\\    convert XFSPerm object to security descriptor.
 (((*/

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
 * That is simple storage for any used SID.
 *
 * The simple reason why I am organizing that storage is :
 *    I should create SECURITY_DESCRIPTOR and convert it to 
 *    PSECURITY_DESCRIPTOR ( which is relative ) and there are
 *    many phases where we should create and free SID by
 *    different reason. And it will complicate uninitialising
 *    procedure in the cases when, for example DACL constucting
 *    failed.
 * There are several disadvantages, for example, if SID was
 *    changed during program execution ... we will think about
 *    that later
 * Another disadvantage is : it suppose to be thread safe
 * 
 * SID is stored and accessed by name ( char * ) of account associated
 * User responsible for initializing SID storage before it's usage
 * and for destroying it after.
 * User could cal method "_SidStorageRehash ()" to renew all sids.
 *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*) That is a lock which I will use to access stored SIDs
 (*/
static struct KLock * _sMutabor = NULL;

/*) That is a SIDs storage
 (*/
static BSTree _sSidStorage;

/*) That struct represents node with SID stored
 (*/
struct __SidNode {
        /*) mandatory
         (*/
    BSTNode node;

        /*) Account name
         (*/
    const char * name;

        /*) SID associated with account
         (*/
    SID * sid;
        /*) SID size, You could always calculate it, but shortcut
         (*/
    DWORD size;
};

/*)))
  |||   Those are prototypes for storage data accessing funcions
  |||   Some are locking, and some are not. Those, which are withou
  |||   locking, they does not check for NuLL
 (((*/
static rc_t CC _ClearSidStorageNoLock ();
static const struct __SidNode * _FindSidNoLock ( const char * Name );
static rc_t _GetSidAndSizeNoLock (
                            const  char * Name,
                            SID ** Sid,
                            DWORD * SidSize
                            );

static SID * _SidStorageGet ( const char * Name );
static DWORD _SidStorageGetSize ( const char * Name );
static rc_t _SidStorageRehash ();

/*)))   Returns true if storage is good and usable.
 ///    Very simple check, but it is better to use method
 \\\    instead copy pasting code
 (((*/
static
bool
_IsSidStorageGood ()
{
    return _sMutabor != NULL;
}   /* _IsSidStorageGood () */

/*)))   Two methods: _InitSidStorage() and _DisposeSidStorage()
 ///    You should call them before and after
(((*/
static
rc_t CC
_SidStorageInit ()
{
    rc_t RCt;
    struct KLock * tLock;

    RCt = 0;
    tLock = NULL;

    if ( _IsSidStorageGood () ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = KLockMake ( & tLock );
    if ( RCt == 0 ) {
        BSTreeInit ( & _sSidStorage );
    }

    if ( RCt == 0 ) {
        _sMutabor = tLock;
    }
    else {
        if ( tLock != NULL ) {
            KLockRelease ( tLock );
        }
    }

    return RCt;
}   /* _SidStorageInit () */

static
rc_t CC
_SidStorageDispose ()
{
    if ( ! _IsSidStorageGood () ) {
        return 0;
    }

        /*) No check for return code 
         (*/
    _ClearSidStorageNoLock ();

        /*) Disengageing Lock
         (*/
    KLockRelease ( _sMutabor );

    return 0;
}   /* _SidStorageDispose () */

SID * CC
_SidStorageGet ( const char * Name )
{
    SID * RetVal;
    rc_t RCt;
    const struct __SidNode * Node;

    RetVal = 0;
    RCt = 0;
    Node = NULL;

    if ( _IsSidStorageGood () ) {
        RCt = KLockAcquire ( _sMutabor );
        if ( RCt == 0 ) {
            Node = _FindSidNoLock ( Name );

            if ( Node != NULL ) {
                RetVal = Node -> sid;
            }

            KLockUnlock ( _sMutabor );
        }
    }

    return RetVal;
}   /* _SidStorageGet () */

DWORD CC
_SidStorageGetSize ( const char * Name )
{
    DWORD RetVal;
    rc_t RCt;
    const struct __SidNode * Node;

    RetVal = 0;
    RCt = 0;
    Node = NULL;

    if ( _IsSidStorageGood () ) {
        RCt = KLockAcquire ( _sMutabor );
        if ( RCt == 0 ) {
            Node = _FindSidNoLock ( Name );

            if ( Node != NULL ) {
                RetVal = Node -> size;
            }

            KLockUnlock ( _sMutabor );
        }
    }

    return RetVal;
}   /* _SidStorageGetSize () */

rc_t CC
_SidStorageRehash ()
{
    rc_t RCt;

    RCt = 0;

    if ( _IsSidStorageGood () ) {
        RCt = KLockAcquire ( _sMutabor );
        if ( RCt == 0 ) {
            RCt = _ClearSidStorageNoLock ();

            KLockUnlock ( _sMutabor );
        }
    }

    return RCt;
}   /* _SidStorageRehash () */

static
rc_t CC
_SidNodeMake ( const char * Name, struct __SidNode ** Node )
{
    struct __SidNode * Ret;
    SID * Sid;
    DWORD SidSize;
    rc_t RCt;

    Ret = NULL;
    Sid = NULL;
    SidSize = 0;
    RCt = 0;

    if ( Name == NULL || Node == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Node = NULL;

        /*)) If Account does not exist, we do create empty node, and
         //  it is users responsibility to handle such account
        ((*/
    RCt = _GetSidAndSizeNoLock ( Name, & Sid, & SidSize );
    if ( RCt != 0 ) {
        return RCt;
    }

    Ret = calloc ( 1, sizeof ( struct __SidNode ) );
    if ( Ret == NULL ) {
        if ( Sid != NULL ) {
            free ( Sid );
        }

        return XFS_RC ( rcExhausted );
    }

    Ret -> name = ( const char * ) string_dup_measure ( Name, NULL );
    if ( Ret -> name == NULL ) {
        if ( Sid != NULL ) {
            free ( Sid );
        }

        free ( Ret );

        return XFS_RC ( rcExhausted );
    }

    Ret -> sid = Sid;
    Ret -> size = SidSize;

    * Node = Ret;

    return 0;
}   /* _SidNodeMake () */

static
rc_t CC
_SidNodeDispose ( struct __SidNode * self )
{
    if ( self != NULL ) {
        if ( self -> name != NULL ) {
            free ( ( char * ) self -> name );
        }

        self -> name = NULL;

        if ( self -> sid != NULL ) {
            free ( self -> sid );
        }

        self -> sid = NULL;

        self -> size = 0;
    }
    return 0;
}   /* _SidNodeDispose () */

static
void CC
_SidStorageWhackCallback ( BSTNode * Node, void * Unused )
{
    if ( Node != NULL ) {
        _SidNodeDispose ( ( struct __SidNode * ) Node );
    }
}   /* _SidStorageWhackCallback () */

rc_t CC
_ClearSidStorageNoLock ()
{
    BSTreeWhack (  & _sSidStorage, _SidStorageWhackCallback, NULL );
    return 0;
}   /* _ClearSidStorageNoLock () */

static
int64_t CC
_SidStorageAddCallback ( const BSTNode * Node1, const BSTNode * Node2 )
{
    return XFS_StringCompare4BST_ZHR (
                            ( ( struct __SidNode * ) Node1 ) -> name,
                            ( ( struct __SidNode * ) Node2 ) -> name
                            );
}   /* _SidStorageAddCallback () */

static
rc_t CC
_AddSidNoLock ( const char * Name, struct __SidNode ** Ret )
{
    struct __SidNode * Node;
    rc_t RCt;

    Node = NULL;
    RCt = 0;

    if ( Name == NULL || Ret == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Ret = NULL;

    RCt = _SidNodeMake ( Name, & Node );
    if ( RCt == 0 ) {
        RCt = BSTreeInsert (
                            & _sSidStorage,
                            & ( Node -> node ),
                            _SidStorageAddCallback
                            );
        if ( RCt == 0 ) {
            * Ret = Node;
        }
    }

    if ( RCt != 0 ) {
        _SidNodeDispose ( Node );
    }

    return RCt;
}   /* _AddSidNoLock () */

static
int64_t CC
_SidStorageFindCallback ( const void * Item, const BSTNode * Node )
{
    return XFS_StringCompare4BST_ZHR (
                            ( const char * ) Item,
                            ( ( struct __SidNode * ) Node ) -> name
                            );
}   /* _SidStorageFindCallback () */

const struct __SidNode *
_FindSidNoLock ( const char * Name )
{
    const struct __SidNode * RetVal;

    RetVal = NULL;

    if ( Name != NULL ) {
        RetVal = ( const struct __SidNode * ) BSTreeFind (
                                            & _sSidStorage,
                                            Name,
                                            _SidStorageFindCallback
                                            );
        if ( RetVal == NULL ) {
            _AddSidNoLock ( Name, ( struct __SidNode ** ) & RetVal );
        }
    }

    return RetVal;
}   /* _FindSidNoLock () */

rc_t
_GetSidAndSizeNoLock ( const  char * Name, SID ** Sid, DWORD * SidSize )
{
    rc_t RCt;
    DWORD Size1, Size2;
    SID_NAME_USE NameUse;
    SID * RetSid;
    BYTE SomeByte [ XFS_SIZE_128 ];

    RCt = 0;
    Size1 = Size2 = 0;
    RetSid = NULL;

    if ( Name == NULL || Sid == NULL || SidSize == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Sid = NULL;
    * SidSize = 0;

        /*) First we should know size of SID
         (*/
    if ( LookupAccountNameA (
                        NULL,
                        Name,
                        NULL,
                        & Size1,
                        NULL,
                        & Size2,
                        & NameUse
                        ) == 0
    ) {
        if ( GetLastError () != ERROR_INSUFFICIENT_BUFFER ) {
            return XFS_RC ( rcExhausted );
        }
    }

    RetSid = calloc ( Size1, sizeof ( BYTE ) );
    if ( RetSid == NULL ) {
        return XFS_RC ( rcExhausted );
    }

    if ( LookupAccountNameA (
                        NULL,
                        Name,
                        RetSid,
                        & Size1,
                        SomeByte,
                        & Size2,
                        & NameUse
                        ) == 0
    ) {
        free ( RetSid );

        return XFS_RC ( rcInvalid );
    }

    * Sid = RetSid;
    * SidSize = Size1;

    return 0;
}   /* _GetSidAndSizeNoLock () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*
 *  Another important part: Names.
 *  We are trying to simulate Unix like permissions, and we are 
 *  going to use names for User and Group. We will ignore Other
 *  group, however, we will reserve name for it. Also, there are
 *  two special accounts which will have all permissions : Admin and
 *  Systems ... So, here are all names
 */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*))
 //     Useful variables
((*/
static const char * _sOtherName   = "Everyone";
static const char * _sAdminName   = "Administrators";
static const char * _sSystemName  = "SYSTEM";
static const char * _sUsersName   = "Users"; /* Users are not User */
static const char * _sCreatorName = "CREATOR OWNER";

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*))
 //     Statics but usefuls
((*/
static
rc_t CC
_GetUserName ( char * Buffer, size_t BufferSize )
{
    DWORD BS;

    if ( Buffer == NULL || BufferSize == 0 ) {
        return XFS_RC ( rcNull );
    }

    BS = BufferSize;

    if ( GetUserNameA ( Buffer, & BS ) == 0 ) {
        return XFS_RC ( rcExhausted );
    }

    return 0;
}   /* _GetUserName () */

static
rc_t CC
_GetGroupNameInLegitWay ( char * Buffer, size_t BufferSize )
{
    rc_t RCt;
    HANDLE ThisProcess;
    HANDLE TokenHandle;
    char TokenInfo [ XFS_SIZE_128 ];
    DWORD LenghtInReturn;
    SID * TheSid;
    SID_NAME_USE peUse;
    char BSht [ XFS_SIZE_128 ];
    size_t BShtSz;

    RCt = 0;
    ThisProcess = TokenHandle = INVALID_HANDLE_VALUE;
    LenghtInReturn = 0;
    TheSid = NULL;
    BShtSz = sizeof ( BSht );

    ThisProcess = GetCurrentProcess ();
    if ( ! OpenProcessToken ( ThisProcess, TOKEN_READ, & TokenHandle ) ) {
        return XFS_RC ( rcInvalid );
    }

    if ( ! GetTokenInformation (
                            TokenHandle,
                            TokenPrimaryGroup,
                            TokenInfo,
                            sizeof ( TokenInfo ),
                            & LenghtInReturn
                            )
    ) {
        RCt = XFS_RC ( rcInvalid );
    }
    else {
        TheSid = ( ( TOKEN_PRIMARY_GROUP * ) TokenInfo ) -> PrimaryGroup;
        if ( TheSid != NULL ) {
            if ( IsValidSid ( TheSid ) ) {
                if ( ! LookupAccountSidA (
                                    NULL,
                                    TheSid,
                                    Buffer,
                                    & BufferSize,
                                    BSht, & BShtSz, /* B-SH#T */
                                    & peUse
                                    )
                ) {
                    RCt = XFS_RC ( rcInvalid );
                }
            }
            else {
                RCt = XFS_RC ( rcInvalid );
            }
        }
        else {
            RCt = XFS_RC ( rcNull );
        }
    }

    CloseHandle ( TokenHandle );

    return 0;
}   /* _GetGroupNameInLegitWay () */

static
rc_t CC
_GetGroupName ( char * Buffer, size_t BufferSize )
{
    size_t NWR;

    if ( Buffer == NULL || BufferSize == 0 ) {
        return XFS_RC ( rcNull );
    }
    * Buffer = 0;

        /*) First we should check if account with name "Users" exists
         (*/
    if ( _SidStorageGet ( _sUsersName ) ) {
        string_printf ( Buffer, BufferSize, &NWR, _sUsersName );
        return 0;
    }

    return _GetGroupNameInLegitWay ( Buffer, BufferSize );
}   /* _GetGroupName () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
 /****************************************************************
  * Here we are initializing security, which includes :
  *  1) Initializing SID storage
  *  2) Checking and resolving names for general accounts
  *     which will be used by XFSPerm object.
  ****************************************************************/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
LIB_EXPORT
rc_t CC
XFSSecurityInit ()
{
    rc_t RCt;
    char Buffer [ XFS_SIZE_128 ];

    RCt = 0;

        /*)  First we are initializing SID storage
         (*/
    RCt = _SidStorageInit ();
    if ( RCt != 0 ) {
        return RCt;
    }

        /* First setting up default User name */
    * Buffer = 0;

    RCt = _GetUserName ( Buffer, sizeof ( Buffer ) ); 
    if ( RCt != 0 ) {
        return RCt;
    }

    RCt = XFSPermSetDefaultName ( kxfsUser, Buffer );
    if ( RCt != 0 ) {
        return RCt;
    }

        /* Second setting up default Group name */
    * Buffer = 0;

    RCt = _GetGroupName ( Buffer, sizeof ( Buffer ) );
    if ( RCt != 0 ) {
        return RCt;
    }

    RCt = XFSPermSetDefaultName ( kxfsGroup, Buffer );
    if ( RCt != 0 ) {
        return RCt;
    }

        /* Third setting up default Other name */
    RCt = XFSPermSetDefaultName ( kxfsOther, _sOtherName );
    if ( RCt != 0 ) {
        return RCt;
    }

    return 0;
}   /* XFSSecurityInit () */

LIB_EXPORT
rc_t CC
XFSSecurityDeinit ()
{
        /*)  I think that is enough
         (*/
    _SidStorageDispose ();

    return 0;
}   /* XFSSecirotuDeinit () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
 /****************************************************************
  * Here we are: making security descriptor
  * There will be two set of functions which will have that pattern
  *       SD_XXXX_Set ()
  * and
  *       SD_XXXX_Size ()
  * and XXXX will be identical. One set is for compiling security
  * descriptor, and another for calculateing it's size
  *
  * We grant all possible rights for Admin and System
  * We will grant/deny unix-like rights for user/group
  * We do not care about others
  *
  *  SecurityDescriptor structure :
  *      SecurityDescriptor header size 0x20 ( 32 )
  *                          sizeof ( _SECURITY_DESCRIPTOR_RELATIVE )
  *    + Owner SID                 size 0x14 ( 20 )
  *                          GetSidLengthRequired ( 5 )
  *    + Group SID                 size 0x14 ( 20 )
  *                          GetSidLengthRequired ( 5 )
  *    + SACL                      size 0x00 ( 00 ) < don't have one
  *    + DACL header               size 0x08 (  8 )
  *                          sizeof ( ACL )
  *    + N * ACE QTY
  *          ACE Header + SID      size 0x1c ( 28 ) * N
  *
  *
  *  We are adding 4 standard ACE GrantAccess entries for file
  *  and 5 standard ACE GrantAccess entries for directory. If here are
  *  DenyAccess ACEs, they will be added directly before GrantAccess
  *  ACEs.
  *
  *  DenyAccess ACEs will be added for owner and users only
  *
  *  We are adding these GrantAccess ACE entries in that order:
  *    For files :
  *                 administrators
  *                 system
  *                 owner
  *                 users
  *    For Directories :
  *                 administrators
  *                 system
  *                 owner
  *                 creator owner
  *                 users
  *
  *  We are setting only 3 types of security information :
  *      OWNER_SECURITY_INFORMATION
  *      GROUP_SECURITY_INFORMATION
  *      DACL_SECURITY_INFORMATION
  *
  *  We will return nothing for those :
  *      SACL_SECURITY_INFORMATION
  *      PROTECTED_DACL_SECURITY_INFORMATION
  *      PROTECTED_SACL_SECURITY_INFORMATION
  *      UNPROTECTED_DACL_SECURITY_INFORMATION
  *      UNPROTECTED_SACL_SECURITY_INFORMATION
  *
  ****************************************************************/
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*)) Inheritance flags
 ((*/
#define _ACE_CONTAINER_FLAGS    \
                              CONTAINER_INHERIT_ACE    \
                            | SUB_CONTAINERS_AND_OBJECTS_INHERIT    \
                            | SUB_CONTAINERS_ONLY_INHERIT    \
                            | SUB_OBJECTS_ONLY_INHERIT

    /*) Directories deny and grant access flags
     (*/
static const DWORD _sAceFlagsDir =
                              _ACE_CONTAINER_FLAGS
                            | OBJECT_INHERIT_ACE
                            ;

static const DWORD _sCreatorAceGrantFlagsDir =
                              _ACE_CONTAINER_FLAGS
                            | INHERIT_ONLY
                            | INHERIT_ONLY_ACE
                            | OBJECT_INHERIT_ACE
                            ;

static const DWORD _sUsersAceGrantFlagsDir =
                              _ACE_CONTAINER_FLAGS
                            | OBJECT_INHERIT_ACE
                            ;

static const DWORD _sOwnerAceGrantFlagsDir =
                              _ACE_CONTAINER_FLAGS
                            | OBJECT_INHERIT_ACE
                            ;

/*))
||| We will suppose that User and Group flags are inheritable
||| 
||| static const DWORD _sUsersAceGrantFlagsDir =
|||                               CONTAINER_INHERIT_ACE
|||                             | SUB_CONTAINERS_ONLY_INHERIT
|||                             ;
||| 
||| static const DWORD _sOwnerAceGrantFlagsDir = NO_INHERITANCE;
(((*/

    /*) Files deny and grant access flags
     (*/
static const DWORD _sAceDenyFlags_File = NO_INHERITANCE;
static const DWORD _sAceGrantFlags_File = NO_INHERITANCE;

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))) Owner and Group for SD
 (((*/
static
rc_t CC
_SD_OwnerGroup_Set (
                    SECURITY_INFORMATION SecInfo,
                    const struct XFSPerm * Permissions,
                    enum XFSAType AccountType,
                    SECURITY_DESCRIPTOR * Descriptor
)
{
    SID * Sid;
    DWORD Flags;
    BOOL ( WINAPI * adder ) (
                        SECURITY_DESCRIPTOR * SD,
                        SID * Sid,
                        BOOL Defaulted
                        );
    const struct XFSAuth * Auth;

    Sid = NULL;
    Flags = 0;
    adder = NULL;
    Auth = NULL;

    if ( Descriptor == NULL || Permissions == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( AccountType == kxfsUser ) {
        Flags = OWNER_SECURITY_INFORMATION;
        adder = SetSecurityDescriptorOwner;
    }
    else {
        if ( AccountType == kxfsGroup ) {
            Flags = GROUP_SECURITY_INFORMATION;
            adder = SetSecurityDescriptorGroup;
        }
        else {
            return XFS_RC ( rcInvalid );
        }
    }

        /*) That information does not needed
         (*/
    if ( ( SecInfo & Flags ) != Flags ) {
        return 0;
    }

    Auth = XFSPermAuth ( Permissions, AccountType );
    if ( Auth == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    Sid = _SidStorageGet ( XFSAuthName ( Auth ) );
    if ( Sid == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( adder ( Descriptor, Sid, FALSE ) == 0 ) { 
        return XFS_RC ( rcInvalid );
    }

    return 0;
}   /* _SD_OwnerGroup_Set () */

static
rc_t CC
_SD_OwnerGroup_Size (
                    SECURITY_INFORMATION SecInfo,
                    const struct XFSPerm * Permissions,
                    enum XFSAType AccountType,
                    ULONG * Size
)
{
    ULONG SidSize;
    DWORD Flags;
    const struct XFSAuth * Auth;

    SidSize = 0;
    Flags = 0;
    Auth = NULL;

    if ( Size == NULL || Permissions == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Size = 0;

    if ( AccountType == kxfsUser ) {
        Flags = OWNER_SECURITY_INFORMATION;
    }
    else {
        if ( AccountType == kxfsGroup ) {
            Flags = GROUP_SECURITY_INFORMATION;
        }
        else {
            return XFS_RC ( rcInvalid );
        }
    }

        /*) That information does not needed
         (*/
    if ( ( SecInfo & Flags ) != Flags ) {
        return 0;
    }

    Auth = XFSPermAuth ( Permissions, AccountType );
    if ( Auth == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    SidSize = _SidStorageGetSize ( XFSAuthName ( Auth ) );
    if ( SidSize == 0 ) {
        return XFS_RC ( rcInvalid );
    }
    * Size = SidSize;

    return 0;
}   /* _SD_OwnerGroup_Size () */

static
rc_t CC
_SD_Owner_Set (
            SECURITY_INFORMATION SecInfo,
            const struct XFSPerm * Permissions,
            SECURITY_DESCRIPTOR * Descriptor
)
{
    return _SD_OwnerGroup_Set (
                            SecInfo,
                            Permissions,
                            kxfsUser,
                            Descriptor
                            );
}   /* _SD_Owner_Set () */

static
rc_t CC
_SD_Owner_Size (
            SECURITY_INFORMATION SecInfo,
            const struct XFSPerm * Permissions,
            ULONG * Size
)
{
    return _SD_OwnerGroup_Size ( SecInfo, Permissions, kxfsUser, Size );
}   /* _SD_Owner_Size () */

static
rc_t CC
_SD_Group_Set (
            SECURITY_INFORMATION SecInfo,
            const struct XFSPerm * Permissions,
            SECURITY_DESCRIPTOR * Descriptor
)
{
    return _SD_OwnerGroup_Set (
                            SecInfo,
                            Permissions,
                            kxfsGroup,
                            Descriptor
                            );
}   /* _SD_Group_Set () */

static
rc_t CC
_SD_Group_Size (
            SECURITY_INFORMATION SecInfo,
            const struct XFSPerm * Permissions,
            ULONG * Size
)
{
    return _SD_OwnerGroup_Size ( SecInfo, Permissions, kxfsGroup, Size );
}   /* _SD_Group_Size () */

static
rc_t CC
_SD_Sacl_Set (
            SECURITY_INFORMATION SecInfo,
            const struct XFSPerm * Permissions,
            XFSNType NodeType,
            SECURITY_DESCRIPTOR * Descriptor
)
{
        /*) He-he ... no SACL
         (*/
    return 0;
}   /* _SD_Sacl_Set () */

static
rc_t CC
_SD_Sacl_Size (
            SECURITY_INFORMATION SecInfo,
            const struct XFSPerm * Permissions,
            XFSNType NodeType,
            ULONG * Size
)
{
        /*) He-he ... no SACL ... but ...
         (*/
    if ( Size == NULL || Permissions == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Size = 0;

    return 0;
}   /* _SD_Sacl_Size () */

static
DWORD CC
_AuthToGrantMode ( const struct XFSAuth * Auth )
{
    DWORD Mode;

    Mode = 0;

    if ( XFSAuthCanRead ( Auth ) ) Mode |= GENERIC_READ;
    if ( XFSAuthCanWrite ( Auth ) ) Mode |= GENERIC_WRITE | DELETE;
    if ( XFSAuthCanExecute ( Auth ) ) Mode |= GENERIC_EXECUTE;

    Mode |= SYNCHRONIZE | 0x1FF;

    return Mode;
}   /* _AuthToGrantMode () */

static
DWORD CC
_AuthToDenyMode ( const struct XFSAuth * Auth )
{
    DWORD Mode;

    Mode = 0;

    if ( ! XFSAuthCanRead ( Auth ) ) Mode |= GENERIC_READ;
    if ( ! XFSAuthCanWrite ( Auth ) ) Mode |= GENERIC_WRITE | DELETE;
    if ( ! XFSAuthCanExecute ( Auth ) ) Mode |= GENERIC_EXECUTE;

    return Mode;
}   /* _AuthToDenyMode () */

static
rc_t CC
_SD_Dacl_Ace_Creator_Size ( XFSNType NodeType, ULONG * Size )
{
    ULONG WholeSize;
    if ( Size == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Size = 0;

    if ( NodeType != kxfsDir ) {
        return 0;
    }

    WholeSize = _SidStorageGetSize ( _sCreatorName );
    if ( WholeSize == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    WholeSize += sizeof ( ACE_HEADER ); /* ACE header */
    WholeSize += sizeof ( DWORD );      /* ACE mask */

    * Size = WholeSize;

    return 0;
}   /* _SD_Dacl_Ace_Creator_Size () */

static
rc_t CC
_SD_Dacl_Ace_Creator_Set ( XFSNType NodeType, ACL * Dacl )
{
    SID * Sid = NULL;

    if ( Dacl == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( NodeType != kxfsDir ) {
        return 0;
    }

    Sid = _SidStorageGet ( _sCreatorName );
    if ( Sid == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( AddAccessAllowedAceEx (
                                Dacl,
                                ACL_REVISION,
                                _sCreatorAceGrantFlagsDir,
                                GENERIC_ALL,
                                Sid
                                ) == 0
    ) {
        return XFS_RC ( rcInvalid );
    }

    return 0;
}   /* _SD_Dacl_Ace_Creator_Set () */

static
rc_t CC
_SD_Dacl_Ace_Power_Size (
                const char * AccName, 
                XFSNType NodeType,  /* not needed really */
                ULONG * Size
)
{
    ULONG WholeSize;
    if ( AccName == NULL || Size == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Size = 0;

    WholeSize = _SidStorageGetSize ( AccName );
    if ( WholeSize == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    WholeSize += sizeof ( ACE_HEADER ); /* ACE header */
    WholeSize += sizeof ( DWORD );      /* ACE mask */

    * Size = WholeSize;

    return 0;
}   /* _SD_Dacl_Ace_Power_Size () */

static
rc_t CC
_SD_Dacl_Ace_Power_Set (
                const char * AccName,
                XFSNType NodeType,
                ACL * Dacl
)
{
    SID * Sid;
    DWORD Flags;

    Sid = NULL;
    Flags = 0;

    if ( AccName == NULL || Dacl == NULL ) {
        return XFS_RC ( rcNull );
    }

    Sid = _SidStorageGet ( AccName );
    if ( Sid == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    Flags = NodeType == kxfsDir ? _sAceFlagsDir : _sAceGrantFlags_File;

    if ( AddAccessAllowedAceEx (
                        Dacl,
                        ACL_REVISION,
                        Flags,
                        SYNCHRONIZE | STANDARD_RIGHTS_REQUIRED | 0x1FF,
                        Sid
                        ) == 0
    ) {
        return XFS_RC ( rcInvalid );
    }

    return 0;
}   /* _SD_Dacl_Ace_Power_Set () */

static
rc_t CC
_SD_Dacl_Ace_GrantDeny_Size (
                const struct XFSPerm * Permissions,
                enum XFSAType AccountType,
                XFSNType NodeType,  /* not needed really */
                bool Grant,
                ULONG * Size
)
{
    ULONG WholeSize;
    const struct XFSAuth * Auth; 
    DWORD Mode;

    WholeSize = 0;
    Auth = NULL;
    Mode = 0;

    if ( Permissions == NULL || Size == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Size = 0;

    if ( AccountType != kxfsUser && AccountType != kxfsGroup ) {
        return 0;
    }

    Auth = XFSPermAuth ( Permissions, AccountType );
    if ( Auth == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    Mode = Grant
                ? _AuthToGrantMode ( Auth )
                : _AuthToDenyMode ( Auth )
                ;
    if ( Mode == 0 ) {
        return 0;
    }

    WholeSize = _SidStorageGetSize ( XFSAuthName ( Auth ) );
    if ( WholeSize == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    WholeSize += sizeof ( ACE_HEADER ); /* ACE header */
    WholeSize += sizeof ( DWORD );      /* ACE mask */

    * Size = WholeSize;

    return 0;
}   /* _SD_Dacl_Ace_GrantDeny_Size () */

static
rc_t CC
_SD_Dacl_Ace_GrantDeny_Set (
                const struct XFSPerm * Permissions,
                enum XFSAType AccountType,
                XFSNType NodeType,
                bool Grant,
                ACL * Dacl
)
{
    const struct XFSAuth * Auth;
    SID * Sid;
    DWORD Flags;
    DWORD Mode;
    BOOL ( WINAPI * adder ) (
                    ACL * Dacl,
                    DWORD Revision,
                    DWORD Flags,
                    DWORD Mode,
                    SID * Sid
                );

    Auth = NULL;
    Sid = NULL;
    Flags = 0;
    Mode = 0;
    adder = NULL;


    if ( Permissions == NULL || Dacl == NULL ) {
        return XFS_RC ( rcNull );
    }

    if ( AccountType != kxfsUser && AccountType != kxfsGroup ) {
        return 0;
    }

    Auth = XFSPermAuth ( Permissions, AccountType );
    if ( Auth == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( Grant ) {
        Mode = _AuthToGrantMode ( Auth );
        Flags = NodeType == kxfsDir
                            ? _sAceFlagsDir
                            : _sAceGrantFlags_File
                            ;
        adder = AddAccessAllowedAceEx;
    }
    else {
        Mode = _AuthToDenyMode ( Auth );
        Flags = NodeType == kxfsDir
                            ? _sAceFlagsDir
                            : _sAceDenyFlags_File
                            ;
        adder = AddAccessDeniedAceEx;
    }

    if ( Mode == 0 ) {
        return 0;
    }


    Sid = _SidStorageGet ( XFSAuthName ( Auth ) );
    if ( Sid == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( adder ( Dacl, ACL_REVISION, Flags, Mode, Sid ) == 0 ) {
        return XFS_RC ( rcInvalid );
    }

    return 0;
}   /* _SD_Dacl_Ace_GrantDeny_Set () */

static
rc_t CC
_SD_Dacl_Aces_Size (
                const struct XFSPerm * Permissions,
                XFSNType NodeType,
                ULONG * Size
)
{
    rc_t RCt;
    ULONG WholeSize, TempSize;

    RCt = 0;
    WholeSize = TempSize = 0;

    if ( Permissions == NULL || Size == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Size = 0;

        /*) Ok, here we are adding denials if it is possible
         (*/
    RCt = _SD_Dacl_Ace_GrantDeny_Size (
                            Permissions,
                            kxfsUser,
                            NodeType,
                            false,
                            & TempSize
                            );
    if ( RCt != 0 ) {
        return RCt;
    }
    WholeSize += TempSize;

    RCt = _SD_Dacl_Ace_GrantDeny_Size (
                            Permissions,
                            kxfsGroup,
                            NodeType,
                            false,
                            & TempSize
                            );
    if ( RCt != 0 ) {
        return RCt;
    }
    WholeSize += TempSize;

        /*) Ok, here we are adding our power users
         (*/
    RCt = _SD_Dacl_Ace_Power_Size (
                                _sAdminName,
                                NodeType,
                                &TempSize
                            );
    if ( RCt != 0 ) {
        return RCt;
    }
    WholeSize += TempSize;

    RCt = _SD_Dacl_Ace_Power_Size (
                                _sSystemName,
                                NodeType,
                                &TempSize
                            );
    if ( RCt != 0 ) {
        return RCt;
    }
    WholeSize += TempSize;

        /*) Ok, here we are adding granted if it is possible
         (*/
    RCt = _SD_Dacl_Ace_GrantDeny_Size (
                            Permissions,
                            kxfsUser,
                            NodeType,
                            true,
                            & TempSize
                            );
    if ( RCt != 0 ) {
        return RCt;
    }
    WholeSize += TempSize;

    RCt = _SD_Dacl_Ace_Creator_Size ( NodeType, & TempSize );
    if ( RCt != 0 ) {
        return RCt;
    }
    WholeSize += TempSize;

    RCt = _SD_Dacl_Ace_GrantDeny_Size (
                            Permissions,
                            kxfsGroup,
                            NodeType,
                            true,
                            & TempSize
                            );
    if ( RCt != 0 ) {
        return RCt;
    }
    WholeSize += TempSize;

    * Size = WholeSize;

    return 0;
}   /* _SD_Dacl_Aces_Size () */

static
rc_t CC
_SD_Dacl_Aces_Set (
                const struct XFSPerm * Permissions,
                XFSNType NodeType,
                ACL * Dacl
)
{
    rc_t RCt;

    if ( Permissions == NULL || Dacl == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = 0;

        /*) Ok, here we are adding denials if it is possible
         (*/
    RCt = _SD_Dacl_Ace_GrantDeny_Set (
                            Permissions,
                            kxfsUser,
                            NodeType,
                            false,
                            Dacl
                            );
    if ( RCt != 0 ) {
        return RCt;
    }

    RCt = _SD_Dacl_Ace_GrantDeny_Set (
                            Permissions,
                            kxfsGroup,
                            NodeType,
                            false,
                            Dacl
                            );
    if ( RCt != 0 ) {
        return RCt;
    }

        /*) Ok, here we are adding our power users
         (*/
    RCt = _SD_Dacl_Ace_Power_Set (
                                _sAdminName,
                                NodeType,
                                Dacl
                            );
    if ( RCt != 0 ) {
        return RCt;
    }

    RCt = _SD_Dacl_Ace_Power_Set (
                                _sSystemName,
                                NodeType,
                                Dacl
                            );
    if ( RCt != 0 ) {
        return RCt;
    }

        /*) Ok, here we are adding grants if it is possible
         (*/
    RCt = _SD_Dacl_Ace_GrantDeny_Set (
                            Permissions,
                            kxfsUser,
                            NodeType,
                            true,
                            Dacl
                            );
    if ( RCt != 0 ) {
        return RCt;
    }

    RCt = _SD_Dacl_Ace_Creator_Set ( NodeType, Dacl );
    if ( RCt != 0 ) {
        return RCt;
    }

    RCt = _SD_Dacl_Ace_GrantDeny_Set (
                            Permissions,
                            kxfsGroup,
                            NodeType,
                            true,
                            Dacl
                            );
    if ( RCt != 0 ) {
        return RCt;
    }

    return 0;
}   /* _SD_Dacl_Aces_Set () */

static
rc_t CC
_SD_Dacl_Size (
            SECURITY_INFORMATION SecInfo,
            const struct XFSPerm * Permissions,
            XFSNType NodeType,
            ULONG * Size
)
{
    rc_t RCt;
    ULONG WholeSize, TempSize;

    RCt = 0;
    WholeSize = TempSize = 0;

    if ( Permissions == NULL || Size == NULL ) {
        return XFS_RC ( rcNull );
    }
    * Size = 0;

    if ( ( SecInfo & DACL_SECURITY_INFORMATION )
                                    != DACL_SECURITY_INFORMATION
    ) {
            /*) Dacl is not needed
             (*/
        return 0;
    }

    WholeSize += sizeof ( ACL );

    RCt = _SD_Dacl_Aces_Size ( Permissions, NodeType, & TempSize );
    if ( RCt != 0 ) {
        return RCt;
    }

    WholeSize += TempSize;

    * Size = WholeSize;

    return 0;
}   /* _SD_Dacl_Size () */

static
rc_t CC
_SD_Dacl_Set (
            SECURITY_INFORMATION SecInfo,
            const struct XFSPerm * Permissions,
            XFSNType NodeType,
            SECURITY_DESCRIPTOR * Descriptor
)
{
    rc_t RCt;
    ACL * Dacl;
    ULONG DaclSize;

    RCt = 0;
    Dacl = NULL;
    DaclSize = 0;

    if ( Permissions == NULL || Descriptor == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    if ( ( SecInfo & DACL_SECURITY_INFORMATION )
                                    != DACL_SECURITY_INFORMATION
    ) {
            /*) Dacl is not needed
             (*/
        return 0;
    }

        /*) Initializing Dacl 
         (*/
    RCt = _SD_Dacl_Size ( SecInfo, Permissions, NodeType, & DaclSize );
    if ( RCt == 0 ) {
        if ( DaclSize == 0 ) {
                /*) Apparently that is impossible
                 (*/
            return XFS_RC ( rcInvalid );
        }

        Dacl = calloc ( DaclSize, sizeof ( BYTE ) );
        if ( Dacl == NULL ) {
            return XFS_RC ( rcExhausted );
        }
    }

    ZeroMemory ( Dacl, sizeof ( BYTE ) * DaclSize );
    if ( InitializeAcl ( Dacl, DaclSize, ACL_REVISION ) == 0 ) {
        free ( Dacl );

        return XFS_RC ( rcInvalid ) ;
    }

    RCt = _SD_Dacl_Aces_Set ( Permissions, NodeType, Dacl );
    if ( RCt != 0 ) {
        free ( Dacl );

        return RCt;
    }

    if ( SetSecurityDescriptorDacl (
                                Descriptor,
                                TRUE,
                                Dacl,
                                FALSE
                                ) == 0
    ) {
        free ( Dacl );

        return XFS_RC ( rcInvalid );
    }

    return 0;
}   /* _SD_Dacl_Set () */

/*)) That function will compose Absolute Security Descriptor
 //  (see also _SD_Size)
((*/
static
rc_t CC
_SD_Set (
        SECURITY_INFORMATION SecInfo,
        const struct XFSPerm * Permissions,
        XFSNType NodeType,
        SECURITY_DESCRIPTOR * Descriptor
) 
{
    rc_t RCt;

    RCt = 0;

        /*) Standard checks
         (*/
    if ( Permissions == NULL || Descriptor == NULL ) {
        return XFS_RC ( rcNull );
    }

        /*) Intializing security descriptor 
         (*/
    ZeroMemory ( Descriptor, sizeof ( SECURITY_DESCRIPTOR ) );
    if ( InitializeSecurityDescriptor (
                                    Descriptor,
                                    SECURITY_DESCRIPTOR_REVISION
                                    ) == 0
    ) {
        return XFS_RC ( rcInvalid );
    }

        /*) Owner information
         (*/
    RCt = _SD_Owner_Set ( SecInfo, Permissions, Descriptor );
    if ( RCt != 0 ) {
        return RCt;
    }

        /*) Group information
         (*/
    RCt = _SD_Group_Set ( SecInfo, Permissions, Descriptor );
    if ( RCt != 0 ) {
        return RCt;
    }

        /*) SACL information
         (*/
    RCt = _SD_Sacl_Set ( SecInfo, Permissions, NodeType, Descriptor );
    if ( RCt != 0 ) {
        return RCt;
    }

        /*) DACL information
         (*/
    RCt = _SD_Dacl_Set ( SecInfo, Permissions, NodeType, Descriptor );
    if ( RCt != 0 ) {
        return RCt;
    }

    return 0;
}   /* _SD_Set () */

/*)) That function will calculate Security Descriptor size
 //  (see also _SD_Set)
((*/
static
rc_t CC
_SD_Size (
        SECURITY_INFORMATION SecInfo,
        const struct XFSPerm * Permissions,
        XFSNType NodeType,
        ULONG * DescSize
)
{
    rc_t RCt;
    ULONG TempSize, WholeSize;

    RCt = 0;
    TempSize = WholeSize = 0;

        /*) Standard checks
         (*/
    if ( Permissions == NULL || DescSize == NULL ) {
        return XFS_RC ( rcNull );
    }
    * DescSize = 0;

        /*) Securit descriptor header
         (*/
    WholeSize += sizeof ( SECURITY_DESCRIPTOR_RELATIVE );

        /*) Owner information
         (*/
    RCt = _SD_Owner_Size ( SecInfo, Permissions, & TempSize );
    if ( RCt != 0 ) {
        return RCt;
    }
    WholeSize += TempSize;

        /*) Group information
         (*/
    RCt = _SD_Group_Size ( SecInfo, Permissions, & TempSize );
    if ( RCt != 0 ) {
        return RCt;
    }
    WholeSize += TempSize;

        /*) SACL information
         (*/
    RCt = _SD_Sacl_Size ( SecInfo, Permissions, NodeType, & TempSize );
    if ( RCt != 0 ) {
        return RCt;
    }
    WholeSize += TempSize;

        /*) DACL information
         (*/
    RCt = _SD_Dacl_Size ( SecInfo, Permissions, NodeType, & TempSize );
    if ( RCt != 0 ) {
        return RCt;
    }
    WholeSize += TempSize;

    * DescSize = WholeSize;

    return 0;
}   /* _SD_Size () */

static
void CC
_SD_Dispose ( SECURITY_DESCRIPTOR * Descriptor )
{
    /*)) dispose only Sacl and Dacl, since Owner and Group are reusable
     ((*/
    if ( Descriptor != 0 ) {
        if ( Descriptor -> Sacl != NULL ) {
            free ( Descriptor -> Sacl );
        }
        if ( Descriptor -> Dacl != NULL ) {
            free ( Descriptor -> Dacl );
        }

        ZeroMemory ( Descriptor, sizeof ( SECURITY_DESCRIPTOR ) );
    }
}   /* _SD_Dispose () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*  Couple cool methods                                          */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
static
void
_SD_AB_Dump ( PSECURITY_DESCRIPTOR Ds, ULONG DsL )
{
    BYTE * Ar;
    int ArL, llp;

    printf ( "[= DUMP SD: [0x%p] [%d]\n", Ds, DsL );

    if ( Ds == NULL || DsL == 0 ) {
        printf ( "INVALID SD\n" );
        return;
    }

    Ar = ( BYTE * ) Ds;
    ArL = DsL / sizeof ( BYTE );

    for ( llp = 0; llp < ArL; llp ++ ) {
        if ( llp % 4 == 0 ) {
            if ( llp != 0 ) { printf ( "\n" ); }
            printf ( " %04d (%02x) ", llp, llp );
        }
        else {
            if ( llp % 2 == 0 ) {
                printf ( " " );
            }
        }

        printf ( "%02x", ( int ) Ar [ llp ] );
    }
    printf ( "\n" );

}   /* _SD_AB_Dump () */

LIB_EXPORT
void
_SD_SR_Dump ( SECURITY_DESCRIPTOR * Sd )
{
    printf ( "[= DUMP USD: [0x%p]\n", Sd );

    if ( Sd == NULL ) { 
        printf ( "INVALID SD\n" );
        return;
    }

    printf ( "REV : [%d]\n", Sd -> Revision );
    printf ( "SBZ1 : [%d]\n", Sd -> Sbz1 );
    printf ( "CNTR: [%d]\n", Sd -> Control );
    printf ( "OWNR: [0x%p]\n", Sd -> Owner );
    printf ( "GRUP: [0x%p]\n", Sd -> Group );
    printf ( "SACL: [0x%p]\n", Sd -> Sacl );
    printf ( "DACL: [0x%p]\n", Sd -> Dacl );

}   /* _SD_SR_Dump () */

LIB_EXPORT
void
_SI_Dump ( SECURITY_INFORMATION Inf, char * Buff, size_t BuffSize )
{
    if ( Buff == NULL || BuffSize < 8 ) {
        return;
    }

    ZeroMemory ( Buff, sizeof ( char ) * BuffSize );

    Buff [0] = ( Inf & OWNER_SECURITY_INFORMATION ) == 0 ? '_' : 'O';
    Buff [1] = ( Inf & GROUP_SECURITY_INFORMATION ) == 0 ? '_' : 'G';
    Buff [2] = ( Inf & DACL_SECURITY_INFORMATION )  == 0 ? '_' : 'D';
    Buff [3] = ( Inf & SACL_SECURITY_INFORMATION )  == 0 ? '_' : 'S';
    Buff [4] = ( Inf & PROTECTED_DACL_SECURITY_INFORMATION )   == 0 ? '_' : 'd';
    Buff [5] = ( Inf & PROTECTED_SACL_SECURITY_INFORMATION )   == 0 ? '_' : 's';
    Buff [6] = ( Inf & UNPROTECTED_DACL_SECURITY_INFORMATION ) == 0 ? '_' : 'c';
    Buff [7] = ( Inf & UNPROTECTED_SACL_SECURITY_INFORMATION ) == 0 ? '_' : 't';
}   /* _SI_Dump () */

/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
/*  Here are external functions                                  */
/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

/*))
 //     NOTE!!! that does not check for length of Security Descriptor
 \\             so if buffer is unsufficient memory will be messed
 //             To obtain size of buffer use XFSSecurityDescriptorSize()
((*/
LIB_EXPORT
rc_t CC
XFSSecurityDescriptor (
                    SECURITY_INFORMATION SecInfo,
                    const char * Permissions,
                    XFSNType NodeType,
                    PSECURITY_DESCRIPTOR Descriptor,
                    ULONG DescriptorLength
)
{
    rc_t RCt;
    SECURITY_DESCRIPTOR AbsDescriptor;
    const struct XFSPerm * Perm;
    DWORD DscLen;

    RCt = 0;
    Perm = NULL;
    DscLen = DescriptorLength;


    if ( Permissions == NULL || Descriptor == NULL ) {
        return XFS_RC ( rcNull );
    }

    RCt = XFSPermMake ( Permissions, & Perm );
    if ( RCt != 0 || Perm == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = _SD_Set ( SecInfo, Perm, NodeType, & AbsDescriptor );
    if ( RCt == 0 ) {
            /*)) Checking if SD is valid
             ((*/
        if ( IsValidSecurityDescriptor ( & AbsDescriptor ) == 0 ) {
            RCt = XFS_RC ( rcInvalid );
        }
        else {
                /*))    we should convert absolute to selfrelative
                 ((*/
            if ( MakeSelfRelativeSD (
                                    & AbsDescriptor,
                                    Descriptor,
                                    & DscLen
                                    ) == 0
            ) {
                RCt = XFS_RC ( rcInvalid );
            }
        }

    }

/*
_SD_SR_Dump ( & AbsDescriptor );
_SD_AB_Dump ( Descriptor, DescriptorLength );
*/

    free ( ( struct XFSPerm * ) Perm );
    _SD_Dispose ( & AbsDescriptor );

    return RCt;
}   /* XFSSecurityDescriptor () */

LIB_EXPORT
rc_t CC
XFSSecurityDescriptorSize (
                        SECURITY_INFORMATION SecInfo,
                        const char * Permissions,
                        XFSNType NodeType,
                        ULONG * DescSize
)
{
    rc_t RCt;
    ULONG WholeSize;
    const struct XFSPerm * Perm;

    RCt = 0;
    WholeSize = 0;
    Perm = NULL;

    if ( Permissions == NULL || DescSize == NULL ) {
        return XFS_RC ( rcNull );
    }
    * DescSize = 0;

        /*) Creating permissions
         (*/
    RCt = XFSPermMake ( Permissions, & Perm );
    if ( RCt != 0 || Perm == NULL ) {
        return XFS_RC ( rcInvalid );
    }

    RCt = _SD_Size ( SecInfo, Perm, NodeType, & WholeSize );
    if ( RCt == 0 ) {
        * DescSize = WholeSize;
    }

    return RCt;
}   /* XFSSecurityDescriptorSize () */

