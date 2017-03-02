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
* ===========================================================================
*
*/
#include <kapp/extern.h>
#include <kapp/main.h>
#include <klib/debug.h>
#include <klib/log.h>
#include <klib/out.h>
#include <klib/status.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <kxml/xml.h>
#include <kfs/file.h>
#include <kfs/kfs-priv.h>
#include <kfs/directory.h>
#include <klib/rc.h>
#include <kapp/log-xml.h>

#include <os-native.h>

#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#define DBG(msg) DBGMSG(DBG_LOADLIB,DBG_FLAG(DBG_LOADLIB_XLOG), msg)

enum OptDefIndex {
    eopt_file = 0,
    eopt_fd
};

const char* XMLLogger_file_usage[] = {"produce XML-formatted log file", NULL};
const char* XMLLogger_fd_usage[] = {NULL, "produce XML-formatted output to given file descriptor", NULL};

const OptDef XMLLogger_Args[] =
{
    {"xml-log", "z", NULL, XMLLogger_file_usage, 1, true, false},
    {"xml-log-fd", NULL, NULL, XMLLogger_fd_usage, 1, true, false}
};
const size_t XMLLogger_ArgsQty = sizeof(XMLLogger_Args) / sizeof(XMLLogger_Args[0]);

typedef struct XMLFormatterFile
{
    KFile* file;
    uint64_t pos;
} XMLFormatterFile;

typedef struct XMLFormatterData
{
    XMLFormatterFile* file;
    KFmtHandler fmt;
    KWrtHandler wrt;
} XMLFormatterData;

struct XMLLogger {
    XMLFormatterFile file;
    XMLFormatterData log;
    XMLFormatterData loglib;
    XMLFormatterData sts;
    XMLFormatterData stslib;
};

LIB_EXPORT
rc_t CC XMLLogger_Encode(const char* src, char *dst, size_t dst_sz, size_t *num_writ)
{
    rc_t rc = 0;

    if( src == NULL || dst == NULL || num_writ == NULL ) {
        rc = RC(rcApp, rcArgv, rcParsing, rcParam, rcNull);
    } else {
        *num_writ = 0;
        do {
            size_t bytes;
            const char* p;
            switch(*src) {
                case '\'':
                    bytes = 6;
                    p = "&apos;";
                    break;
                case '"':
                    bytes = 6;
                    p = "&quot;";
                    break;
                case '&':
                    bytes = 5;
                    p = "&amp;";
                    break;
                case '<':
                    bytes = 4;
                    p = "&lt;";
                    break;
                case '>':
                    bytes = 4;
                    p = "&gt;";
                    break;
                default:
                    bytes = 1;
                    p = src;
                    break;
            }
            if( (*num_writ + bytes) > dst_sz ) {
                rc = RC(rcApp, rcLog, rcEncoding, rcBuffer, rcInsufficient);
                break;
            } else {
                memmove(dst, p, bytes);
                *num_writ = *num_writ + bytes;
                dst += bytes;
            }
        } while(*src++ != '\0');
        *num_writ = *num_writ - 1;
    }
    return rc;
}

static
rc_t CC LoaderXMLFormatter( void* data, KWrtHandler* writer,
                            size_t argc, const wrt_nvp_t args[],
                            size_t envc, const wrt_nvp_t envs[])

{
    rc_t rc = 0;
    size_t i, remaining, num_writ = 0;
    XMLFormatterData* self = (XMLFormatterData*)data;
    char buffer[4096];
    const char* severity, *msg_val = NULL;
    bool severity_std = false;
    char* pbuffer;
    const char* const tag_nvp_name = "severity";
    const wrt_nvp_t* msg_nvp = NULL;
    static const char* tags[] = {
        "fatal",
        "system",
        "internal",
        "error",
        "warning",
        "info"
    };

    msg_nvp = wrt_nvp_find(envc, envs, "message");
    if( msg_nvp != NULL ) {
        msg_val = msg_nvp->value;
    }
    severity = wrt_nvp_find_value(argc, args, tag_nvp_name);
    if( severity == NULL ) {
        severity = wrt_nvp_find_value(envc, envs, tag_nvp_name);
        if( severity != NULL ) {
            severity_std = true;
            /* translate std severity name to full name */
            for(i = 0; i < sizeof(tags)/sizeof(tags[0]); i++) {
                if( strncmp(severity, tags[i], strlen(severity)) == 0 ) {
                    severity = tags[i];
                    break;
                }
            }
        }
    }
    if( severity == NULL ) {
        severity = "status";
    }

#define FIX_UP() if(rc != 0){break;} remaining -= num_writ; pbuffer += num_writ
    
    pbuffer = buffer;
    remaining = sizeof(buffer);
    do {
        size_t k, pq = envc;
        const wrt_nvp_t* p = envs;
        const char* subst = NULL;

        rc = LogInsertSpace("<", pbuffer, remaining, &num_writ);
        FIX_UP();
        rc = LogInsertSpace(severity, pbuffer, remaining, &num_writ);
        FIX_UP();
        /* print env first and than args! */
        for(k = 0; rc == 0 && k < 2; k++) {
            for(i = 0; i < pq; i++ ) {
                if( strcmp(p[i].name, tag_nvp_name) == 0 ) {
                    continue;
                }
                if( p == args ) {
                    if( i == 0 && msg_nvp != NULL ) {
                        /* grab args attr buffer start */
                        subst = pbuffer;
                    }
                    if( severity_std ) {
                        /* allow only specific attributes from message into xml log
                           for LOG calls with standard severity */
                        int x, yes = 0;
                        static const char* allowed_attr[] = {
                            "file", "line", "offset",
                            "spot", "spot_name", "spotname"
                        };
                        for(x = 0; x < sizeof(allowed_attr)/sizeof(allowed_attr[0]); x++) {
                            if( strcasecmp(p[i].name, allowed_attr[x]) == 0 ) {
                                yes = 1;
                                break;
                            }
                        }
                        if( !yes ) {
                            continue;
                        }
                    }
                }
                rc = LogInsertSpace(" ", pbuffer, remaining, &num_writ);
                FIX_UP();
                rc = XMLLogger_Encode(p[i].name, pbuffer, remaining, &num_writ);
                FIX_UP();
                rc = LogInsertSpace("=\"", pbuffer, remaining, &num_writ);
                FIX_UP();
                rc = XMLLogger_Encode(p[i].value, pbuffer, remaining, &num_writ);
                FIX_UP();
                rc = LogInsertSpace("\"", pbuffer, remaining, &num_writ);
                FIX_UP();
            }
            p = args;
            pq = argc;
        }
        if( subst != NULL && subst[0] != '\0' ) {
            /* hack 'message' to print curr argv to std log as text attr="value" */
            ((wrt_nvp_t*)msg_nvp)->value = subst + 1; /* \0 terminated pre LogInsertSpace behavior */
            if( (rc = self->fmt.formatter(self->fmt.data, &self->wrt, 0, NULL, envc, envs)) != 0 ) {
                break;
            }
            ((wrt_nvp_t*)msg_nvp)->value = msg_val;
        }
        rc = LogInsertSpace("/>\n", pbuffer, remaining, &num_writ);
        FIX_UP();
    } while(false);

    if( self->file->file != NULL ) {
        if( rc != 0 ) {
            pbuffer = buffer;
            remaining = sizeof(buffer);
            rc = string_printf(pbuffer, remaining, & num_writ, "<error severity=\"err\" message=\"XML log failed: %R\"/>\n", rc);
            pbuffer += num_writ <= remaining ? num_writ : 0;
        }
        rc = KFileWrite(self->file->file, self->file->pos, buffer, pbuffer - buffer, &num_writ);
        self->file->pos += num_writ;
    }
    rc = self->fmt.formatter(self->fmt.data, &self->wrt, argc, args, envc, envs);
    return rc;
}

LIB_EXPORT rc_t CC XMLLogger_Make(const XMLLogger** cself, KDirectory* dir, const Args *args)
{
    rc_t rc = 0;
    const char* path = NULL, *sfd = NULL;
    int fd = -1;
    uint32_t count;

    if( (rc = ArgsOptionCount(args, XMLLogger_Args[eopt_file].name, &count)) != 0 || count > 1 ) {
        rc = rc ? rc : RC(rcApp, rcArgv, rcParsing, rcParam, rcExcessive);
    } else if( count > 0 && (rc = ArgsOptionValue(args, XMLLogger_Args[eopt_file].name, 0, (const void **)&path)) != 0 ) {

    } else if( (rc = ArgsOptionCount(args, XMLLogger_Args[eopt_fd].name, &count)) != 0 || count > 1 ) {
        rc = rc ? rc : RC(rcApp, rcArgv, rcParsing, rcParam, rcExcessive);
    } else if( count > 0 && (rc = ArgsOptionValue(args, XMLLogger_Args[eopt_fd].name, 0, (const void **)&sfd)) != 0 ) {

    } else {
        do {
            long val = 0;
            char* end = NULL;

            if( sfd != NULL ) {
                if( path != NULL ) {
                    rc = RC(rcApp, rcArgv, rcParsing, rcParam, rcExcessive);
                    break;
                }
                errno = 0;
                val = strtol(sfd, &end, 10);
                if( errno != 0 || sfd == end || *end != '\0' || val < 0 || val > INT_MAX ) {
                    rc = RC(rcApp, rcArgv, rcReading, rcParam, rcInvalid);
                    break;
                }
                fd = val;
            }
            rc = XMLLogger_Make2(cself, dir, path, fd);
        } while( false );
    }
    return rc;
}

LIB_EXPORT rc_t CC XMLLogger_Make2(const XMLLogger** self, KDirectory* dir, const char* logpath, const int fd)
{
    rc_t rc = 0;
    XMLLogger* obj;
    KDirectory* my_dir = NULL;

    const uint32_t lopt = klogFmtTimestamp | klogFmtSeverity |
                          klogFmtMessage | klogFmtAppName | klogFmtAppVersion | klogFmtReasonShort;

    const uint32_t sopt = kstsFmtTimestamp | kstsFmtMessage | kstsFmtAppName | kstsFmtAppVersion;

    KLogFmtFlagsSet(lopt);
    KLogLibFmtFlagsSet(lopt);
    KStsFmtFlagsSet(sopt);
    KStsLibFmtFlagsSet(sopt);

    obj = calloc(1, sizeof(*obj));
    if( obj == NULL ) {
        rc = RC(rcApp, rcLog, rcAllocating, rcMemory, rcExhausted);
    } else if( fd < 0 && dir == NULL && (rc = KDirectoryNativeDir(&my_dir)) != 0 ) {
    } else if( fd >= 0 && (rc = KFileMakeFDFileWrite(&obj->file.file, false, fd)) != 0 ) {
    } else if( logpath != NULL && fd < 0 && (rc = KDirectoryCreateFile(dir ? dir : my_dir, &obj->file.file, false, 0644, kcmInit, "%s", logpath)) != 0 ) {
    } else {
    
        obj->file.pos = 0;
        obj->log.file = &obj->file;
        obj->log.fmt.formatter = KLogFmtWriterGet();
        obj->log.fmt.data = KLogFmtDataGet();
        obj->log.wrt.writer = KLogWriterGet();
        obj->log.wrt.data = KLogDataGet();

        obj->loglib.file = &obj->file;
        obj->loglib.fmt.formatter = KLogLibFmtWriterGet();
        obj->loglib.fmt.data = KLogLibFmtDataGet();
        obj->loglib.wrt.writer = KLogLibWriterGet();
        obj->loglib.wrt.data = KLogLibDataGet();

        obj->sts.file = &obj->file;
        obj->sts.fmt.formatter = KStsFmtWriterGet();
        obj->sts.fmt.data = KStsFmtDataGet();
        obj->sts.wrt.writer = KStsWriterGet();
        obj->sts.wrt.data = KStsDataGet();

        obj->stslib.file = &obj->file;
        obj->stslib.fmt.formatter = KStsLibFmtWriterGet();
        obj->stslib.fmt.data = KStsLibFmtDataGet();
        obj->stslib.wrt.writer = KStsLibWriterGet();
        obj->stslib.wrt.data = KStsLibDataGet();

        if( (rc = KLogFmtHandlerSet(LoaderXMLFormatter, lopt, &obj->log)) == 0 &&
            (rc = KLogLibFmtHandlerSet(LoaderXMLFormatter, lopt, &obj->loglib)) == 0 &&
            (rc = KStsFmtHandlerSet(LoaderXMLFormatter, sopt, &obj->sts)) == 0 &&
            (rc = KStsLibFmtHandlerSet(LoaderXMLFormatter, sopt, &obj->stslib)) == 0 ) {
            /* make log valid XML */
            if( obj->file.file != NULL ) {
                size_t num_writ = 0;
                rc = KFileWrite(obj->file.file, obj->file.pos, "<Log>\n", 6, &num_writ);
                obj->file.pos += num_writ;
            }
        }
    }
    KDirectoryRelease(my_dir);
    if( rc == 0 ) {
        *self = obj;
        if( fd >= 0 ) {
            DBG(("XML Log file set to handle %d\n", fd));
        } else if( logpath != NULL) {
            DBG(("XML Log file set to %s\n", logpath));
        } 
    } else {
        XMLLogger_Release(obj);
        *self = NULL;
    }
    return rc;
}

LIB_EXPORT void CC XMLLogger_Usage(void)
{
    size_t i;
    for(i = 0; i < XMLLogger_ArgsQty; i++ ) {
        if( XMLLogger_Args[i].help[0] != NULL ) {
            HelpOptionLine(XMLLogger_Args[i].aliases, XMLLogger_Args[i].name, "logfile", XMLLogger_Args[i].help);
        }
    }
}

LIB_EXPORT void CC XMLLogger_Release(const XMLLogger* cself)
{
    if( cself != NULL ) {
        XMLLogger* self = (XMLLogger*)cself;
        KLogFmtHandlerSet(self->log.fmt.formatter, 0, self->log.fmt.data);
        KLogLibFmtHandlerSet(self->loglib.fmt.formatter, 0, self->loglib.fmt.data);
        KLogHandlerSet(self->log.wrt.writer, self->log.wrt.data);
        KLogLibHandlerSet(self->loglib.wrt.writer, self->loglib.wrt.data);
        KStsFmtHandlerSet(self->sts.fmt.formatter, 0, self->sts.fmt.data);
        KStsLibFmtHandlerSet(self->stslib.fmt.formatter, 0, self->stslib.fmt.data);
        KStsHandlerSet(self->sts.wrt.writer, self->sts.wrt.data);
        KStsLibHandlerSet(self->stslib.wrt.writer, self->stslib.wrt.data);
        /* make log valid XML */
        if( self->file.file != NULL ) {
            if( self->file.pos > 0 ) {
                KFileWrite(self->file.file, self->file.pos, "</Log>\n", 7, NULL);
            }
            KFileRelease(self->file.file);
        }
        free(self);
    }
}
