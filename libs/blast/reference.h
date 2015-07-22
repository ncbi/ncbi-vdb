#ifndef _h_libs_blast_reference_
#define _h_libs_blast_reference_

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

#ifdef __cplusplus
extern "C" {
#endif

struct Core4na;
struct References;
struct RunSet;

uint64_t _Core2naReadRef(struct Core2na *self, VdbBlastStatus *status,
    uint64_t *read_id, uint8_t *buffer, size_t buffer_size);

uint32_t _Core2naDataRef(struct Core2na *self,
    Data2na *data, VdbBlastStatus *status,
    Packed2naRead *buffer, uint32_t buffer_length);

size_t _Core4naReadRef(struct Core4na *self, const struct RunSet *runs,
    uint32_t *status, uint64_t read_id, size_t starting_base,
    uint8_t *buffer, size_t buffer_length);

const uint8_t* _Core4naDataRef(struct Core4na *self, const struct RunSet *runs,
    uint32_t *status, uint64_t read_id, size_t *length);

const struct References* _RunSetMakeReferences
    (struct RunSet *self, VdbBlastStatus *status);
void _ReferencesWhack(const struct References *self);
uint64_t _ReferencesGetNumSequences
    (const struct References *self, VdbBlastStatus *status);
uint64_t _ReferencesGetTotalLength
    (const struct References *self, VdbBlastStatus *status);

#ifdef __cplusplus
}
#endif

#endif /* _h_libs_blast_reference_ */
