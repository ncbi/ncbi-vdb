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

#include <kfs/extern.h>
#include <magic.h>
#include <klib/rc.h>
#include <kfs/file.h>
#include <klib/text.h>
#include <klib/log.h>
#include <klib/debug.h>
#include <klib/container.h>
#include <kfs/directory.h>
#include <kfs/fileformat.h>
#include <kfg/config.h>
#include <sysalloc.h>
#include "fileformat-priv.h"
struct KMagicFileFormat;
#define KFILEFORMAT_IMPL struct KMagicFileFormat
#include "impl.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <os-native.h>
#include <stdio.h>

typedef
struct KMagicNode
{
    BSTNode		node;
    atomic32_t 		refcount;
    KFileFormatType	typeid;
    size_t		kfflen;
    size_t		magiclen;
    char *		magicdescr;
    char		kffdescr [1];
} KMagicNode;

static
rc_t KMagicNodeDestroy (const KMagicNode * cself)
{
    rc_t rc;
    KMagicNode * self;

    FUNC_ENTRY();

    rc = 0;
    self = (KMagicNode*)cself;

    if (self == NULL)
    {
	rc = RC (rcFF, rcIndex, rcDestroying, rcSelf, rcNull);
	LOGERR (klogErr, rc, "KMagicNodeDestroy: self == NULL");
    }
    else
    {
	free (self);
    }
    return rc;
}

static
rc_t KMagicNodeRelease (const KMagicNode * cself)
{
    rc_t rc = 0;

    FUNC_ENTRY();

    if (cself != NULL)
    {
        KMagicNode *self = (KMagicNode*)cself;
        if (atomic32_dec_and_test (&self->refcount))
            return  KMagicNodeDestroy (cself);
    }
    return rc;
}
/* not used at this time */
#if 0
static
rc_t KMagicNodeAddRef (const KMagicNode * self)
{
    rc_t rc = 0;

    FUNC_ENTRY();

    if (self != NULL)
        atomic32_inc (&((KFileFormat*)self)->refcount);
    return rc;
}
#endif
static
rc_t KMagicNodeMake (KMagicNode ** kmmp, const KFFTables * tables,
		     const char * magicdescr, size_t magiclen,
		     const char * kffdescr, size_t kfflen)

{
    KMagicNode * self;
    rc_t rc = 0;

    FUNC_ENTRY();

    if (magicdescr == NULL)
    {
	rc = RC (rcFF, rcIndex, rcConstructing, rcParam, rcNull);
	LOGERR (klogErr, rc, "KMagicNodeMake: magicdescr == NULL");
    }
    else if (kffdescr == NULL)
    {
	rc = RC (rcFF, rcIndex, rcConstructing, rcParam, rcNull);
	LOGERR (klogErr, rc, "KKffNodeMake: kffdescr == NULL");
    }
    else
    {
	if (magiclen > DESCRLEN_MAX)
	{
	    rc = RC (rcFF, rcIndex, rcConstructing, rcParam, rcInvalid);
	    LOGERR (klogErr, rc, "KMagicNodeMake: magicdescr too long");
	}
	else if (magiclen == 0)
	{
	    rc = RC (rcFF, rcIndex, rcConstructing, rcParam, rcEmpty);
	    LOGERR (klogErr, rc, "KMagicNodeMake: magicdescr too short");
	}
	else if (kfflen > DESCRLEN_MAX)
	{
	    rc = RC (rcFF, rcIndex, rcConstructing, rcParam, rcInvalid);
	    LOGERR (klogErr, rc, "KKffNodeMake: kffdescr too long");
	}
	else if (kfflen == 0)
	{
	    rc = RC (rcFF, rcIndex, rcConstructing, rcParam, rcEmpty);
	    LOGERR (klogErr, rc, "KKffNodeMake: kffdescr too short");
	}
	else
	{
	    KFileFormatType typeid;
	    rc = KFFTablesGetTypeId (tables, kffdescr, &typeid, NULL);
	    if (rc == 0)
	    {
		self = malloc (sizeof (*self) + magiclen + kfflen + 1);
		if (self == NULL)
		{
		    rc = RC (rcFF,  rcIndex, rcConstructing, rcMemory, rcExhausted);
		    LOGERR (klogErr, rc, "KMagicNodeMake: self could not be allocated");
		}
		else
		{
		    atomic32_set (&self->refcount,1);
		    self->typeid = typeid;
		    self->kfflen = kfflen;
		    self->magiclen = magiclen;
		    self->magicdescr = self->kffdescr + kfflen + 1;
		    memmove (self->kffdescr, kffdescr, kfflen);
		    memmove (self->magicdescr, magicdescr, magiclen);
		    self->kffdescr[self->kfflen] = '\0';
		    self->magicdescr[self->magiclen] = '\0';
		    *kmmp = self;
		    return 0;
		}
	    }
	}
    }
    return rc;
}


/* -------------------------
 *
 */
typedef
struct KMagicTable
{
    atomic32_t 	refcount;
    BSTree	tree;
} KMagicTable;

static
void KMagicNodeWhack (BSTNode * n, void * ignored)
{
    FUNC_ENTRY();

    (void)KMagicNodeRelease((KMagicNode*)n);
}
static
rc_t KMagicTableDestroy (KMagicTable * cself)
{
    rc_t rc;
    KMagicTable * self;

    FUNC_ENTRY();

    rc = 0;
    self = (KMagicTable*)cself;
    if (self == NULL)
    {
	rc = RC (rcFF, rcTable, rcDestroying, rcSelf, rcNull);
	LOGERR (klogErr, rc, "KMagicTableDestroy: self == NULL");
    }
    else
    {
	BSTreeWhack (&self->tree, KMagicNodeWhack, NULL);
	free (self);
    }
    return rc;
}
static
rc_t KMagicTableRelease (const KMagicTable * cself)
{
    rc_t rc = 0;

    FUNC_ENTRY();

    if (cself != NULL)
    {
	KMagicTable *self = (KMagicTable*)cself;
        if (atomic32_dec_and_test (&self->refcount))
	    return  KMagicTableDestroy (self);
    }
    return rc;
}
#if 0
static /* not used at this time */
rc_t KMagicTableAddRef (const KMagicTable * self)
{
    rc_t rc = 0;

    FUNC_ENTRY();

    if (self != NULL)
        atomic32_inc (& ((KFileFormat*) self)->refcount);
    return rc;
}
#endif
static
rc_t KMagicTableMake (KMagicTable ** kmmtp)
{
    KMagicTable * self;
    rc_t rc = 0;

    FUNC_ENTRY();

    self = malloc (sizeof *self);
    if (self == NULL)
    {
        rc = RC (rcFF, rcTable, rcConstructing, rcParam, rcNull);
        LOGERR (klogErr, rc, "KMagicTableMake: self could not be allocated");
    }
    else
    {
        atomic32_set (&self->refcount,1);
        BSTreeInit (&self->tree);
        *kmmtp = self;
    }
    return rc;
}

static
int64_t KMagicNodeCmp (const void* item, const BSTNode * n)
{
    size_t len;
    KMagicNode * mn = (KMagicNode *)n;

    FUNC_ENTRY();

    /* -----
     * we only check this many characters of the comparison item
     * we need only this part to match and ignore characters after
     * this in the comparison string
     */
    len = mn->magiclen;
    return strncmp (item, mn->magicdescr, len);
}

static
rc_t KMagicTableFind (KMagicTable * self, KMagicNode ** node, const char * str)
{
    rc_t rc = 0;

    FUNC_ENTRY();

    *node = (KMagicNode*)BSTreeFind (&self->tree, str, KMagicNodeCmp);
    if (*node == NULL)
    {
/* 	rc = RC (rcFF, rcTable, rcSearching, rcNode, rcNotFound); */
        KFF_DEBUG (("%s: Could not find %s\n", __func__, str));
    }
    return rc;
}
/* maxlen includes the terminating NUL */
#if 0 /* not in use at this time */
static
rc_t KMagicTableFindKFFDescr (KMagicTable * self, const char * str, char * kff, size_t maxlen)
{
    rc_t rc;
    KMagicNode * np;

    FUNC_ENTRY();

    if (self == NULL)
    {
	rc = RC (rcFF, rcFileFormat, rcSearching, rcSelf, rcNull);
	LOGERR (klogErr, rc, "KMagicTableFindKFFDecr:self == NULL");
	return rc;
    }
    if (str == NULL)
    {
	rc = RC (rcFF, rcFileFormat, rcSearching, rcParam, rcNull);
	LOGERR (klogErr, rc, "KMagicTableFindKFFDecr: searchstring is NULL");
	return rc;
    }
    if (kff == NULL)
    {
	rc = RC (rcFF, rcFileFormat, rcSearching, rcParam, rcNull);
	LOGERR (klogErr, rc, "KMagicTableFindKFFDecr: found storage is NULL");
	return rc;
    }
    rc = KMagicTableFind (self, &np, str);
    if (maxlen <= np->kfflen) /* kfflen does not include NUL */
    {
	rc = RC (rcFF, rcFileFormat, rcSearching, rcParam, rcTooLong);
	LOGERR (klogErr, rc, "KMagicTableFindKFFDecr: found storage is NULL");
	return rc;
    }
    memmove (kff, np->kffdescr, np->kfflen);
    kff[np->kfflen] = '\0';
    return rc;
}
#endif
static
int64_t KMagicNodeSort (const BSTNode* item, const BSTNode * n)
{
    const char * str1;
    const char * str2;

    FUNC_ENTRY();

    str1 = ((KMagicNode *)item)->magicdescr;
    str2 = ((KMagicNode *)n)->magicdescr;
    return strcmp (str1, str2);
}
static
rc_t KMagicTableInsert (KMagicTable * self, KMagicNode *node)
{
    FUNC_ENTRY();

    return (BSTreeInsert (&self->tree, &node->node, KMagicNodeSort));
}
/* not is use at this time */
#if 0

static
rc_t KMagicTableBufferRead (KMagicTable * self, KFFTables * tables,
			    const char * buff, size_t bufflen)
{
    rc_t rc = 0;
    const char * kff;
    const char * magic;
    const char * nl;
    size_t kfflen;
    size_t magiclen;

    FUNC_ENTRY();


    /* -----
     * until we get all the way through the buffer
     * which by this coding could actually be all blank
     */
    while (bufflen)
    {
	magic = buff;
	/* -----
	 * allow leading white space including blank lines
	 */
	if (isspace (*magic))	
	{
	    buff++;
	    bufflen --;
	    continue;
	}
	/* -----
	 * not a comment line so find the tab splitting the sections
	 */
	kff = memchr (magic, '\t', bufflen);
	if (kff == NULL)
	{
	    /* couldn't find it so blae the document and quit */
	bad_line:
	    rc = RC (rcFF, rcBuffer, rcParsing, rcFormat, rcCorrupt);
	    /* log error */
	    bufflen = 0;
	    continue;
	}
	/* -----
	 * the magic portion of the line is from the first non-white space
	 * through the character before the tab.
	 */
	magiclen = kff - magic;
	bufflen -= magiclen + 1;
	kff++; /* point past the tab */
	while (bufflen) /* skip white space */
	{
	    if (*kff == '\n') /* end of line now is a format error */
	    {
		goto bad_line;
	    }
	    if (!isspace (*kff)) /* break at non shite space character */
		break;
	    bufflen --;
	    kff ++;
	}
	if (bufflen == 0) /* no kff descr */
	    goto bad_line;
	nl = memchr (kff, '\n', bufflen);
	if (nl == NULL) /* no EOL but last line in buffer */
	{
	    kfflen = bufflen;
	    bufflen = 0;
	}
	else /* not last unfinished line */
	{
	    kfflen = nl - magic;
	    bufflen -= kfflen + 1;
	    buff = nl + 1;
	}
	{
	    KMagicNode * np;
	    rc = KMagicNodeMake (&np, tables, magic, magiclen,
				 kff, kfflen);
	    if (rc != 0)
	    {
		/* LOG ERR */
		break;
	    }
	}
    }
    ( break;
	    }
	}
    }
    return rc;
}
#endif

/* not used at this time */
#if 0
LIB_EXPORT rc_t CC KMagicTableRead (KMagicTable * self, const KFile * file))
{
    rc_t rc = 0;
    /* setup KMMap */
    /* call KMagicTableBufferRead */
    return rc;
}

LIB_EXPORT rc_t CC KMagicTableWrite (const KMagicTable * self, KFile * file)
{
    rc_t rc = 0;

    return rc;
}
#endif

/* -----
 * format is
 * whitechar := {' '|'\f'|'\t'|'\v'}
 * whitespace := whitechar*
 * magic-str = !whitespace!{'\t'|'\n'}*
 * kff-str = !whitespace!{'\t'|'\n'}*
 *
 * A line is 
 * [<whitespace>]#<comment line skipped>\n
 * Or 
 * [<whitespace>]<magic-str>\t[<whitespace>]<kff-str>\n
 * Or
 * [<whitespace>]\n
 *
 * NOTE: whitespace at the right end of the two strings is included in the strings
 * NOTE: We do not look for '\v'|'\f' within the strings though we maybe should
 * NOTE: the magic string definitely allows white space and punctuation
 */
static
rc_t KMagicTableInit (KMagicTable * self, const KFFTables * tables, const char * buffer, size_t len)
{
    rc_t rc;
    const char * magic;
    const char * kff;
    const char * tab;
    const char * newline;
    const char * line;
    const char * limit;
    KMagicNode * node;

    FUNC_ENTRY();

    rc = 0;

    /* -----
     * we try to go all the way through the buffer line by line
     * which by this coding could actually be all blank
     */
    limit = buffer + len;
    for (line = buffer; line < limit; line = newline+1)
    {
	for (magic = line; isspace (*magic); magic++, len--)
	{
	    if (len == 0) /* last of the file was all whitespace so quit */
		break;
	}
	newline = memchr (magic, '\n', len);
	if (newline == NULL)
	    newline = magic + len;
	/* -----
	 * If the first character on the line is #
	 * we treat it as a comment (matches sh/bash/libmagic/etc.
	 */
	if (*magic == '#')
	{
	    /* -----
	     * skip this line
	     */
	    len -= newline+1 - magic;
	    continue;
	}
	
	tab = memchr (magic, '\t', len);
	if (tab == NULL)
	{
	    rc = RC (rcFF, rcFileFormat, rcConstructing, rcFile, rcInvalid);
	    LOGERR (klogFatal, rc, "No <TAB> between magic and kff");
	    break;
	}
	kff = tab + 1;
	for (len -= kff - magic;
	     isspace (*kff);
	     len--, kff++)
	{
	    if (len == 0)
	    {
		rc = RC (rcFF, rcFileFormat, rcConstructing, rcFile, rcInvalid);
		LOGERR (klogFatal, rc, "No kff after <TAB>");
		break;
	    }
	}
	if (newline == kff)
	{
	    rc = RC (rcFF, rcFileFormat, rcConstructing, rcFile, rcInvalid);
	    LOGERR (klogFatal, rc, "No kff after whitespace");
	    break;
	}
	len -= newline+1 - kff;
 	rc = KMagicNodeMake (&node, tables, magic, tab-magic, kff, newline-kff);
	if (rc != 0)
	{
	    LOGERR (klogFatal, rc, "Failure to make node");
	    break;
	}
	rc = KMagicTableInsert(self, node);
	if (rc != 0)
	{
	    LOGERR (klogFatal, rc, "Failure to insert node");
	    break;
	}
    }
    if (rc == 0)
    {
	char unknown[] = "Unknown";

	rc = KMagicNodeMake (&node, tables, unknown, sizeof (unknown) - 1, unknown, sizeof (unknown) - 1);
	if (rc != 0)
	{
	    LOGERR (klogFatal, rc, "Failure to make node");
	}
	else
	{
	    rc = KMagicTableInsert(self, node);
	    if (rc != 0)
	    {
		LOGERR (klogFatal, rc, "Failure to insert node");
	    }
	}
    }
    return rc;
}


/*--------------------------------------------------------------------------
 * KMagicFileFormat
 *  a file content (format) categorizer
 */

typedef
struct KMagicFileFormat
{
    KFileFormat	  dad;
    magic_t 	  cookie;
    KMagicTable * table;
} KMagicFileFormat;

static rc_t KMagicFileFormatDestroy (KMagicFileFormat *self);
static rc_t KMagicFileFormatGetTypeBuff (const KMagicFileFormat *self,
					 const void * buff, size_t buff_len,
					 KFileFormatType * type,
					 KFileFormatClass * class,
					 char * description,
					 size_t descriptionmax,
					 size_t * descriptionlength);
static rc_t KMagicFileFormatGetTypePath (const KMagicFileFormat *self,
					 const KDirectory * dir,
					 const char * path,
					 KFileFormatType * type,
					 KFileFormatClass * class,
					 char * description,
					 size_t descriptionmax,
					 size_t * descriptionlength);
static
KFileFormat_vt_v1 vt_v1 =
{
    1, 1, /* maj, min */
    KMagicFileFormatDestroy,
    KMagicFileFormatGetTypeBuff,
    KMagicFileFormatGetTypePath
};



/* Destroy
 *  destroy FileFormat
 */
static
rc_t KMagicFileFormatDestroy (KMagicFileFormat *self)
{
    FUNC_ENTRY();

    rc_t rc = KMagicTableRelease (self->table);
    magic_close (self->cookie);
    {
        rc_t rc2 = KFFTablesRelease (self->dad.tables);
        if ( rc == 0 )
            rc = rc2;
    }
    free (self);
    return rc;
}

/* Type
 *  returns a KMagicFileFormatDesc
 *  [OUT] rc_t               return
 *  [IN]  const KMagicFileFormat *  self         
 *  [IN]  void **            buffer       buffer to hold returned description
 *  [IN]  size_t             buffer_size  size of the buffer
 *  [OUT] char **            descr        text description of file type
 *  [IN]  size_t             descr_max    maximum size of string descr can hold
 *  [OUT] size_t *           descr_len    length of returned descr (not including NUL
 */
static
rc_t KMagicFileFormatGetTypePath (const KMagicFileFormat *self, const KDirectory * dir, const char * path,
				  KFileFormatType * type, KFileFormatClass * class,
				  char * descr, size_t descr_max, size_t *descr_len)
{
    rc_t rc = 0;
    uint8_t	buff	[8192];
    size_t	bytes_read;

    rc = KDirectoryAddRef (dir);
    if (rc == 0)
    {
	const KFile * file;
	rc = KDirectoryOpenFileRead (dir, &file, "%s", path);
	if (rc == 0)
	{
	    rc = KFileRead (file, 0, buff, sizeof buff, &bytes_read);
	    {
		rc = KMagicFileFormatGetTypeBuff (self,buff, bytes_read,
						  type, class, descr,
						  descr_max, descr_len);
	    }
	    KFileRelease (file);
	}
	KDirectoryRelease (dir);
    }
    return rc;
}
static
rc_t KMagicFileFormatGetTypeBuff (const KMagicFileFormat *self, const void * buff, size_t buff_len,
				  KFileFormatType * type, KFileFormatClass * class,
				  char * descr, size_t descr_max, size_t *descr_len)
{
    rc_t rc = 0;
    const char * b;

    FUNC_ENTRY();

    if (type != NULL)
	*type = kfftError;
    if (class != NULL)
	*class = kffcError;
    b = magic_buffer (self->cookie, buff, buff_len);
    if (b == NULL)
	rc = RC (rcFF, rcFileFormat, rcParsing, rcFormat, rcUnrecognized);
    else
    {
	KMagicNode * node;
	size_t c;

        KFF_DEBUG (("magic_buffer returned %s\n", b));
#if 1
 	rc = KMagicTableFind (self->table, &node, b);
	if (rc == 0)
	{
	    KFileFormatClass cid;
	    KFileFormatType tid;
#define TABLES self->dad.tables
	    if (node == NULL)
		rc = KMagicTableFind (self->table, &node, "Unknown");
	    if (rc == 0)
		rc = KFFTablesGetTypeId (TABLES, node->kffdescr, &tid, &cid);
	    if (rc == 0)
	    {
		c = node->kfflen;
		if (c > descr_max)
		    c = descr_max-1;
		if (descr)
		    string_copy(descr, descr_max, node->kffdescr, c);
		descr[c] = '\0';
		if (descr_len)
		    *descr_len = c;
		if (type)
		    *type = tid;
		if (class)
		    *class = cid;
	    }
	}
	if (rc != 0)
	{
	    if (descr_len != NULL)
		*descr_len = 0;
	    if (type != NULL)
		*type = kfftNotFound;
	    if (class != NULL)
		*class = kffcNotFound;
	}	


#else
    size_t size;
	size_t l = string_measure(b, &size);
	if (descr != NULL)
	{
	    string_copy (descr, descr_max, b, l);
	    if (descr_max < l)
		descr[descr_max-1] = 0;
	}
	if (descr_len != NULL)
	    *descr_len = strlen (b);
	if (type != NULL)
	    *type = kfftUnknown;
	if (class != NULL)
	    *class = kffcUnknown;
#endif
    }
    return rc;
}


LIB_EXPORT rc_t CC KMagicFileFormatMake (KFileFormat ** pft, const char * magic_path,
			   const char* magic, size_t magiclen,
			   const char * typeAndClass, size_t tclen)
{
    rc_t rc = 0;
    KMagicFileFormat * self;

    FUNC_ENTRY();

    DBGMSG (DBG_KFS, DBG_FLAG(DBG_KFS_KFF),
            ("%s: load path as called %s\n", __func__, magic_path));

    self = malloc (sizeof * self);
    if (self == NULL)
    {
        rc = RC (rcFF, rcFileFormat, rcAllocating, rcMemory, rcExhausted);
        LOGERR (klogFatal, rc, "Failed to allocate for KMagicFileFormat");
    }
    else
    {
        rc = KFileFormatInit (&self->dad, (const KFileFormat_vt *)&vt_v1, typeAndClass, tclen);
        if (rc == 0)
        {
            rc = KMagicTableMake (&self->table);
            if (rc == 0)
            {
                rc = KMagicTableInit (self->table, self->dad.tables, magic, magiclen);
                if (rc == 0)
                {
                    self->cookie = magic_open (MAGIC_PRESERVE_ATIME);
        /* 		    self->cookie = magic_open (MAGIC_PRESERVE_ATIME|MAGIC_DEBUG|MAGIC_CHECK); */
                    if (self->cookie == NULL)
                    {
                        rc = RC (rcFF, rcFileFormat, rcConstructing, rcResources, rcNull);
                        LOGERR (klogFatal, rc, "Unable to obtain libmagic cookie");
                    }
                    else
                    {
                        KConfig * kfg;
                        static const char kfg_magic_path[] = "kfs/kff/magic";
                        char * magic_path_cursor;
                        char kfg_magic_path_buff [1024];
                        char magic_path_buff [4096];
                        size_t z;
                        int load_code;

                        magic_path_cursor = magic_path_buff;

                        if (magic_path)
                        {
                            z = strlen (magic_path);
                            if (magic_path_cursor != magic_path_buff)
                                *magic_path_cursor++ = ':';
                            memmove (magic_path_cursor, magic_path, z);
                            magic_path_cursor += z;
                            *magic_path_cursor = '\0';
                            DBGMSG (DBG_KFS, DBG_FLAG(DBG_KFS_KFF),
                                    ("%s: 1 magic_path_buff %s\n", __func__, magic_path_buff));
                        }

                        /* see if we have a configured alternate magic path */
                        rc = KConfigMake (&kfg, NULL);
                        if (rc == 0)
                        {
                            const KConfigNode * node;
                            rc = KConfigOpenNodeRead (kfg, &node, kfg_magic_path);
                            if (rc == 0)
                            {
                                size_t remaining;
                                rc = KConfigNodeRead (node, 0, kfg_magic_path_buff,
                                                      sizeof kfg_magic_path_buff-1,
                                                      &z, &remaining);
                                if (rc == 0)
                                {
                                    if (remaining != 0)
                                    {
                                        rc = RC (rcFF, rcFileFormat, rcLoading, rcString, rcExcessive);
                                        DBGMSG (DBG_KFS, DBG_FLAG(DBG_KFS_KFF),
                                                ("%s: failed KConfigOpenNodeRead remaining %zu %R\n",
                                                __func__, remaining, rc));
                                    }
                                    else if (z)
                                    {
                                        kfg_magic_path_buff[z] = '\0';
                                        if (magic_path_cursor != magic_path_buff)
                                            *magic_path_cursor++ = ':';
                                        memmove (magic_path_cursor,kfg_magic_path_buff, z);
                                        magic_path_cursor += z;
                                        *magic_path_cursor = '\0';
                                        DBGMSG (DBG_KFS, DBG_FLAG(DBG_KFS_KFF),
                                                ("%s: 3 magic_path_buff %s\n", __func__, magic_path_buff));
                                    }
                                }
                                else
                                    DBGMSG (DBG_KFS, DBG_FLAG(DBG_KFS_KFF),
                                        ("%s: failed KConfigNodeRead %R\n", __func__, rc));
                                        
                                {
                                    rc_t rc2 = KConfigNodeRelease(node);
                                    if (rc == 0)
                                        rc = rc2;
                                }
                            }
                            else
                            {   /* no magic path konfigured; report but this is not an error */
                                DBGMSG (DBG_KFS, DBG_FLAG(DBG_KFS_KFF),
                                        ("%s: failed KConfigOpenNodeRead %R\n", __func__, rc));
                                rc = 0;
                            }
                                    
                            {
                                rc_t rc2 = KConfigRelease(kfg);
                                if (rc == 0)
                                    rc = rc2;
                            }
                        }
                        else 
                            DBGMSG (DBG_KFS, DBG_FLAG(DBG_KFS_KFF),
                                    ("%s: failed KConfigMake %R\n", __func__, rc));
        
                        if (rc == 0)
                        {
/*VDB-2911: remove use of system magic file; do not want to break when magic file format changes */
#if 0                            
                            KDirectory * pwd;
                            rc = KDirectoryNativeDir (&pwd);
                            if (rc == 0)
                            {
                                static const char unix_magic_path[] = "/usr/share/file/magic";
                                KPathType kpt = KDirectoryPathType (pwd, unix_magic_path);
                                rc = KDirectoryRelease(pwd);
                                DBGMSG (DBG_KFS, DBG_FLAG(DBG_KFS_KFF),
                                        ("%s: %s %x\n", __func__, unix_magic_path, kpt));
                                if ((kpt & ~ kptAlias) == kptFile)
                                {
                                    z = strlen (unix_magic_path);
                                    if (magic_path_cursor != magic_path_buff)
                                        *magic_path_cursor++ = ':';
                                    memmove (magic_path_cursor, unix_magic_path, z);
                                    magic_path_cursor += z;
                                    *magic_path_cursor = '\0';
                                    DBGMSG (DBG_KFS, DBG_FLAG(DBG_KFS_KFF),
                                            ("%s: 2 magic_path_buff %s\n", __func__, magic_path_buff));
                                }
                            }
                            else
                                LOGERR (klogErr, rc, "Failed to open NativeDir for Magic");
#endif
                            DBGMSG (DBG_KFS, DBG_FLAG(DBG_KFS_KFF),
                                    ("%s: loading path %s\n", __func__, magic_path_buff));

                            load_code = magic_load (self->cookie, magic_path_buff);
                            if (load_code != 0) /* defined as 0 success and -1 as fail */
                            {
                                KFF_DEBUG (("%s: magic_load() failed with load code %d(%s)\n", __func__, load_code, magic_error (self->cookie) ));
                                rc = RC (rcFF, rcFileFormat, rcLoading, rcLibrary, rcUnexpected);
                            }
                            else
                            {
                                *pft = &self->dad;
                                KFF_DEBUG (("%s Success\n", __func__));
                                return 0;
                            }
                        }
                        magic_close (self->cookie);
                    }
                }
                else
                    LOGERR (klogErr, rc, "Fail from KMagicTableInit");
                    
                KMagicTableRelease (self->table);
            }
            else
                LOGERR (klogErr, rc, "Fail from KMagicTableMake");
        }
        else
            LOGERR (klogErr, rc, "Fail from KFileFormatInit");
        free (self);
    }
    return rc;
}

