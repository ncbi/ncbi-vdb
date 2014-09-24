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
 */

#include <kfs/extern.h>
#include <klib/log.h>
#include <klib/debug.h>
#include <klib/rc.h>
#include <kfs/file.h>
#include <kfs/countfile.h>
#include <sysalloc.h>

#include <assert.h>
#include <stdlib.h>

/* ======================================================================
 * KCounterFile
 */

/* -----
 * define the specific types to be used in the templatish/inheritancish
 * definition of vtables and their elements
 */
#define KFILE_IMPL struct KCounterFile
#include <kfs/impl.h>

/* clear out possible macro definitions of FS and GS,
 * which show up on Solaris/x86 when building with GCC (per sys/ucontext.h)
 */
#ifdef FS
#  undef FS
#endif
#ifdef GS
#  undef GS
#endif

static rc_t CC KCounterFileDestroy (KCounterFile *self);
static struct KSysFile *CC KCounterFileGetSysFile (const KCounterFile *self,
					    uint64_t *offset);
static rc_t CC KCounterFileRandomAccess (const KCounterFile *self);
static uint32_t CC KCounterFileType (const KCounterFile *self);
static rc_t CC KCounterFileSize (const KCounterFile *self, uint64_t *size);
static rc_t CC KCounterFileSetSize (KCounterFile *self, uint64_t size);
static rc_t CC KCounterFileRead (const KCounterFile *self, uint64_t pos,
                                 void *buffer, size_t bsize, size_t *num_read);
static rc_t CC KCounterFileWrite (KCounterFile *self, uint64_t pos, const void *buffer,
			       size_t size, size_t *num_writ);


static const KFile_vt_v1 vtKCounterFile =
{
    /* version */
    1, 1,

    /* 1.0 */
    KCounterFileDestroy,
    KCounterFileGetSysFile,
    KCounterFileRandomAccess,
    KCounterFileSize,
    KCounterFileSetSize,
    KCounterFileRead,
    KCounterFileWrite,

    /* 1.1 */
    KCounterFileType
};

/*-----------------------------------------------------------------------
 * KCounterFile
 */
typedef uint8_t lstate;
enum elstate
{
    ENOTHING,
    EASCII,
    ECR,
    ELF,
    ECRLF,
    ELFCR,
    EBINARY
};
struct KCounterFile
{
    KFile	dad;
    uint64_t	max_position;
    KFile *	original;
    uint64_t *	bytecounter; /* we do not trust reads from bytecounter */
    uint64_t *	linecounter; /* we do not trust reads from linecounter */
    lstate      line_state;
    bool	size_allowed;
    bool	force;
};

/* ----------------------------------------------------------------------
 * KCounterFileMake
 *  create a new file object
 */

static
rc_t KCounterFileMake (KCounterFile ** pself,
		       KFile * original,
		       uint64_t * bytecounter,
		       uint64_t * linecounter,
                       bool force)
{
    uint64_t		fsize;
    rc_t		rc;
    KCounterFile *	self;

    /* -----
     * we can not accept any of the three pointer parameters as NULL
     */
    assert (pself != NULL);
    assert (original != NULL);
    assert (bytecounter != NULL);

    /* -----
     * get space for the object
     */
    self = malloc (sizeof (KCounterFile));
    if (self == NULL)	/* allocation failed */
    {
	/* fail */
	rc = RC (rcFS, rcFile, rcConstructing, rcMemory, rcExhausted);
    }
    else
    {
	rc = KFileInit (&self->dad,			/* initialize base class */
			(const KFile_vt*)&vtKCounterFile,/* VTable for KCounterFile */
            "KCounterFile", "no-name",
			original->read_enabled,
			original->write_enabled);
	if (rc == 0)
	{
            for (;;)
            {
                if (force) /* all sizes come from actual reads */
                {
                    fsize = 0;
                    self->force = true;
                    self->size_allowed = false;
                }
                else
                {
                    rc = KFileSize(original,&fsize);
                    if (GetRCState(rc) == rcUnsupported)
                    {
                        force = true;
                        continue;
                    }
                    else if (rc)
                        break;
                    self->force = false;
                    self->size_allowed = true;
                }
                self->max_position = fsize;
                self->original = original;
                self->bytecounter = bytecounter;
                self->linecounter = linecounter;
                if (linecounter)
                {
                    self->line_state = ENOTHING;
                    *linecounter = 0;
                }
                else
                {
                    self->line_state = EBINARY;
                }
                *bytecounter = fsize;
                *pself = self;
                return 0;
            }
	}
	/* fail */
	free (self);
    }
    *pself = NULL;
    return rc;
}

LIB_EXPORT rc_t CC KFileMakeCounterRead (const KFile ** self, const KFile * original,
                                         uint64_t * bytecounter, uint64_t * linecounter,
                                         bool force_reads)
{
    return KCounterFileMake ((KCounterFile **)self, (KFile*)original, bytecounter, linecounter, force_reads);
}
LIB_EXPORT rc_t CC KFileMakeCounterUpdate (KFile ** self, KFile * original,
                                           uint64_t * bytecounter, uint64_t * linecounter,
                                           bool force_reads)
{
    return KCounterFileMake ((KCounterFile **)self, original, bytecounter, linecounter, force_reads);
}
LIB_EXPORT rc_t CC KFileMakeCounterWrite (KFile ** self, KFile * original,
                                          uint64_t * bytecounter, uint64_t * linecounter,
                                          bool force_reads)
{
    return KCounterFileMake ((KCounterFile **)self, original, bytecounter, linecounter, force_reads);
}

enum ecc
{
    NUL,
    SOH,
    STX,
    ETX,
    EOT,
    ENQ,
    ACK,
    BEL,
    BS,
    TAB,
    LF,
    VT,
    FF,
    CR,
    SO,
    SI,
    DLE,
    DC1,
    DC2,
    DC3,
    DC4,
    NAK,
    SYN,
    ETB,
    CAN,
    EM,
    SUB,
    ESC,
    FS,
    GS,
    RS,
    US,
    DEL = 127
};

static
void check_state (KCounterFile * self, void * _buffer, size_t size)
{
    uint8_t * buffer = _buffer;
    size_t idx;

    if (self->linecounter == NULL)
        return;

    if (self->line_state == EBINARY)
        return;

    for (idx = 0; idx < size; ++idx)
    {
        if (buffer[idx] >= DEL)
        {
            self->line_state = EBINARY;
        }
        else
        {
            switch (buffer[idx])
            {
            case NUL:
            case SOH:
            case STX:
            case ETX:
            case EOT:
            case ENQ:
            case ACK:
            case BEL:
            case BS:
            case SO:
            case SI:
            case DLE:
            case DC1:
            case DC2:
            case DC3:
            case DC4:
            case NAK:
            case SYN:
            case ETB:
            case CAN:
            case EM:
            case SUB:
            case ESC:
            case FS:
            case GS:
            case RS:
            case US:
            case DEL:
                /* leave line count mode */
                self->line_state = EBINARY;
                *self->linecounter = 0;
                return;

            case 128: /* not in use yet - a pseudo-EOL */
                /* FF might go here? */
                ++*self->linecounter;
                self->line_state = EASCII;
                break;
                

            case LF:
                switch (self->line_state)
                {
                case ENOTHING:
                case ELF:
                case ECRLF:
                case ELFCR:
                    ++*self->linecounter;
                case EASCII:
                    self->line_state = ELF;
                    break;
                case ECR:
                    self->line_state = ECRLF;
                    break;
                }
                break;
            case CR:
                switch (self->line_state)
                {
                case ENOTHING:
                case ECR:
                case ECRLF:
                case ELFCR:
                    ++*self->linecounter;
                case EASCII:
                    self->line_state = ECR;
                    break;
                case ELF:
                    self->line_state = ELFCR;
                    break;
                }
                break;

            case TAB:
            case VT:
            case FF:
            default:
                if (self->line_state != EASCII)
                {
                    ++*self->linecounter;
                }
                self->line_state = EASCII;
                break;
            }
        }       
    }
}

static
rc_t KCounterFileSeek (KCounterFile * self, uint64_t pos)
{
    uint64_t	max_position;
    rc_t	rc = 0;
    size_t	num_read = 0;
    size_t	to_read;
    uint8_t	ignored[64*1024];

    assert (self->max_position < pos);
    for (max_position = self->max_position; max_position < pos; max_position += num_read)
    {
            to_read = (size_t)( pos - max_position );
            if (to_read > sizeof ignored)
                to_read = sizeof ignored;
            rc = KFileRead (self->original, max_position, ignored, to_read, &num_read);
            if (rc || (num_read == 0))
                break;
            check_state (self, ignored, num_read);
    }
    self->max_position = max_position;
    *self->bytecounter = max_position;
    return rc;
}

/* ----------------------------------------------------------------------
 * Destroy
 *
 */
static
rc_t CC KCounterFileDestroy (KCounterFile *self)
{
    rc_t rc = 0;
    uint64_t size;

    assert (self != NULL);
    assert (self->bytecounter != NULL);

    if (self->force || ! self->size_allowed)
    {
        size_t	num_read = 0;
        uint8_t	ignored[64*1024];
        
        size = self->max_position;
        if (self->dad.read_enabled)
            do
            {
                rc = KFileRead (self->original, size,
                                ignored, sizeof ignored, &num_read);
                size += num_read;
                DBGMSG(DBG_KFS,DBG_FLAG(DBG_KFS_COUNTER),
                       ("%s: size '%lu' num_read '%lu'\n", __func__, size, num_read));
                if (rc != 0)
                    break;
                check_state (self, ignored, num_read);
            } while (num_read != 0);
    }
    else
    {
        rc = KFileSize (self->original, &size);
        DBGMSG(DBG_KFS,DBG_FLAG(DBG_KFS_COUNTER),
               ("%s: lazy way size '%lu'\n", __func__, size));
    }
    *self->bytecounter = size;
    if (rc == 0)
    {
        rc = KFileRelease (self->original);
        free (self);
    }
    return rc;
}

/* ----------------------------------------------------------------------
 * GetSysFile
 *  returns an underlying system file object
 *  and starting offset to contiguous region
 *  suitable for memory mapping, or NULL if
 *  no such file is available.
 *
 * bytes could not be counted if memory mapped so this is disallowed
 */

static
struct KSysFile *CC KCounterFileGetSysFile (const KCounterFile *self, uint64_t *offset)
{
    /* parameters must be non-NULL */
    assert (self != NULL);
    assert (offset != NULL);

    return NULL;
}

/* ----------------------------------------------------------------------
 * RandomAccess
 *
 *  returns 0 if random access, error code otherwise
 */
static
rc_t CC KCounterFileRandomAccess (const KCounterFile *self)
{
    assert (self != NULL);
    assert (self->original != NULL);
    return KFileRandomAccess (self->original);
}

/* ----------------------------------------------------------------------
 * Type
 *  returns a KFileDesc
 *  not intended to be a content type,
 *  but rather an implementation class
 */
static
uint32_t CC KCounterFileType (const KCounterFile *self)
{
    assert (self != NULL);
    assert (self->original != NULL);

    return KFileType (self->original);
}

/* ----------------------------------------------------------------------
 * Size
 *  returns size in bytes of file
 *
 *  "size" [ OUT ] - return parameter for file size
 */
static
rc_t CC KCounterFileSize (const KCounterFile *self, uint64_t *size)
{
    rc_t	rc;
    uint64_t	fsize;

    assert (self != NULL);
    assert (self->original != NULL);
    assert (size != NULL);

    rc = KFileSize (self->original, &fsize);

    if (rc == 0)
    {
	/* success: refreshing the value */
	*size = ((KCounterFile *)self)->max_position = fsize;
    }
    /* pass along RC value */
    return rc;
}

/* ----------------------------------------------------------------------
 * SetSize
 *  sets size in bytes of file
 *
 *  "size" [ IN ] - new file size
 */
static
rc_t CC KCounterFileSetSize (KCounterFile *self, uint64_t size)
{
    rc_t rc;

    rc = KFileSetSize (self->original, size);
    if (rc == 0)
	self->max_position = size;
    return rc;
}

/* ----------------------------------------------------------------------
 * Read
 *  read file from known position
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT, NULL OKAY ] - optional return parameter
 *  giving number of bytes actually read
 */
static
rc_t CC KCounterFileRead	(const KCounterFile *cself,
				 uint64_t pos,
				 void *buffer,
				 size_t bsize,
				 size_t *num_read)
{
    KCounterFile * 	self;
    uint64_t	max_position;
    uint64_t	temp_max_position;
    rc_t	rc;


    /* -----
     * self and buffer were validated as not NULL before calling here
     *
     * So get the KTTOCNode type: chunked files and contiguous files 
     * are read differently.
     */
    assert (cself != NULL);
    assert (cself->original != NULL);
    assert (buffer != NULL);
    assert (num_read != NULL);
    assert (bsize != 0);

    self = (KCounterFile*)cself;
    max_position = cself->max_position;

    if ((pos > max_position) && (! self->size_allowed))
    {
	rc = KCounterFileSeek (self, pos);
	if (rc != 0)
	    return rc;
        /* if seek failed */
        if (pos > self->max_position)
        {
            *num_read = 0;
            return 0;
        }
    }
    rc = KFileRead (self->original, pos, buffer, bsize, num_read);
    temp_max_position = pos + *num_read;
    if (temp_max_position > max_position)
    {
        uint32_t new_bytes = (uint32_t)(temp_max_position - max_position);

        check_state (self, ((char *)buffer) + (*num_read) - new_bytes, new_bytes);

	*self->bytecounter = self->max_position = temp_max_position;
    }
    return rc;
}

/* ----------------------------------------------------------------------
 * Write
 *  write file at known position
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ IN ] and "size" [ IN ] - data to be written
 *
 *  "num_writ" [ OUT, NULL OKAY ] - optional return parameter
 *  giving number of bytes actually written
 *
 * Unsupported as we now treat archives as READ ONLY
 */
static
rc_t CC KCounterFileWrite (KCounterFile *self, uint64_t pos,
			   const void *buffer, size_t bsize,
			   size_t *num_writ)
{
    uint64_t	max_position;
    uint64_t	temp_max_position;
    rc_t	rc;


    /* -----
     * self and buffer were validated as not NULL before calling here
     *
     * So get the KTTOCNode type: chunked files and contiguous files 
     * are read differently.
     */
    assert (self != NULL);
    assert (self->original != NULL);
    assert (buffer != NULL);
    assert (num_writ != NULL);
    assert (bsize != 0);

    max_position = self->max_position;

    if ((self->dad.read_enabled) && (pos > max_position) && (! self->size_allowed))
    {
	rc = KCounterFileSeek (self, pos);
	if (rc != 0)
	    return rc;
    }

    rc = KFileWrite (self->original, pos, buffer, bsize, num_writ);
    temp_max_position = pos + *num_writ;

    if (temp_max_position > max_position)
    {
	*self->bytecounter = self->max_position = temp_max_position;
    }
    return rc;
}


/* end of file countfile.c */

