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


#define DEFAUTL_KFG \
"/config/default = \"true\"\n"                                                 \
                                                                               \
"/libs/cloud/report_instance_identity = \"false\"\n"                           \
                                                                               \
"/sra/quality_type = \"raw_scores\"\n"                                         \
                                                                               \
"/repository/user/main/public/apps/file/volumes/flat = \"files\"\n"            \
"/repository/user/main/public/apps/nakmer/volumes/nakmerFlat = \"nannot\"\n"   \
"/repository/user/main/public/apps/nannot/volumes/nannotFlat = \"nannot\"\n"   \
"/repository/user/main/public/apps/refseq/volumes/refseq = \"refseq\"\n"       \
"/repository/user/main/public/apps/sra/volumes/sraFlat = \"sra\"\n"            \
"/repository/user/main/public/apps/sraPileup/volumes/withExtFlat = \"sra\"\n"  \
"/repository/user/main/public/apps/sraRealign/volumes/withExtFlat = \"sra\"\n" \
"/repository/user/main/public/apps/wgs/volumes/wgsFlat = \"wgs\"\n"            \
                                                                               \
"strings/sdl = \"" SDL_CGI "\"\n"                                              \
                                                                               \
"/repository/remote/main/SDL.2/resolver-cgi = \"$(strings/sdl)\"\n"            \
"/repository/remote/protected/SDL.2/resolver-cgi = \"$(strings/sdl)\"\n"       \
                                                                               \
"/tools/ascp/max_rate = \"450m\"\n"

// "strings/name s = \"" RESOLVER_CGI "\"\n"
// "/repository/remote/main/CGI/resolver-cgi = \"$(strings/name s)\"\n"
// "/repository/remote/protected/CGI/resolver-cgi = \"$(strings/name s)\"\n"
