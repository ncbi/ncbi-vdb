/* ===========================================================================
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
#extern "C" {
#endif

typedef struct c_SRAFileReader c_SRAFileReader;
typedef struct c_SRASpotEnumerator c_SRASpotEnumerator;
typedef struct c_SRAReferenceList c_SRAReferenceList;
typedef struct c_SRAAlignmentEnumerator c_SRAAlignmentEnumerator;

void c_SRAFileReaderSetDbgFlag(char const *flag);

c_SRAFileReader *c_SRAFileReaderCreate(char const accession[], char const **errmsg);
void c_SRAFileReaderFree(c_SRAFileReader *);

char const *c_SRAFileReaderAccession(c_SRAFileReader const *self);


c_SRASpotEnumerator *c_SRAFileReaderCreateSpotEnumerator(c_SRAFileReader *self, char const **errmsg);
void c_SRASpotEnumeratorFree(c_SRASpotEnumerator *self);
bool c_SRASpotEnumeratorNext(c_SRASpotEnumerator *self);

int64_t         c_SRASpotEnumeratorReadID            (c_SRASpotEnumerator const *self);
int32_t         c_SRASpotEnumeratorPlatform          (c_SRASpotEnumerator const *self);
char     const *c_SRASpotEnumeratorReadGroup         (c_SRASpotEnumerator const *self, uint32_t *length);
uint32_t        c_SRASpotEnumeratorNumberOfReads     (c_SRASpotEnumerator const *self);
uint8_t  const *c_SRASpotEnumeratorReadTypeArray     (c_SRASpotEnumerator const *self, uint32_t *length);
uint8_t  const *c_SRASpotEnumeratorReadFilterArray   (c_SRASpotEnumerator const *self, uint32_t *length);
uint32_t const *c_SRASpotEnumeratorReadLengthArray   (c_SRASpotEnumerator const *self, uint32_t *length);
int32_t  const *c_SRASpotEnumeratorReadStartArray    (c_SRASpotEnumerator const *self, uint32_t *length);
char     const *c_SRASpotEnumeratorReadSequenceArray (c_SRASpotEnumerator const *self, uint32_t *length);
uint8_t  const *c_SRASpotEnumeratorReadQualityArray  (c_SRASpotEnumerator const *self, uint32_t *length);
int64_t  const *c_SRASpotEnumeratorAlignIDArray      (c_SRASpotEnumerator const *self, uint32_t *length);


c_SRAReferenceList const *c_SRAFileReaderGetReferenceList(c_SRAFileReader *self, char const **errmsg);
void c_SRAReferenceListFree(c_SRAReferenceList *self);
uint32_t    c_SRAReferenceListGetCount(c_SRAReferenceList const *self);
int32_t     c_SRAReferenceListGetIndexForName(c_SRAReferenceList const *self, char const *name);
int32_t     c_SRAReferenceListGetIndexForRowID(c_SRAReferenceList const *self, int64_t rowid);

char const *c_SRAReferenceListGetName      (c_SRAReferenceList const *self, uint32_t i);
char const *c_SRAReferenceListGetAccession (c_SRAReferenceList const *self, uint32_t i);
uint32_t    c_SRAReferenceListGetBaseCount (c_SRAReferenceList const *self, uint32_t i);
bool        c_SRAReferenceListGetIsCircular(c_SRAReferenceList const *self, uint32_t i);


c_SRAAlignmentEnumerator *c_SRAFileReaderCreateAlignmentByIDEnumerator(c_SRAFileReader *self, int64_t id,
                                                                       char const **const errmsg);

c_SRAAlignmentEnumerator *c_SRAFileReaderCreateAlignmentEnumerator(c_SRAFileReader *self,
                                                                   bool primary,
                                                                   bool secondary,
                                                                   char const **const errmsg);

c_SRAAlignmentEnumerator *c_SRAFileReaderCreateOrderedAlignmentEnumerator(c_SRAFileReader *self,
                                                                          bool primary,
                                                                          bool secondary,
                                                                          char const **const errmsg);

void c_SRAAlignmentEnumeratorFree(c_SRAAlignmentEnumerator *self);
bool c_SRAAlignmentEnumeratorNext(c_SRAAlignmentEnumerator *self);
bool c_SRAAlignmentEnumeratorSeek(c_SRAAlignmentEnumerator *self,
                                  c_SRAReferenceList const *refList,
                                  char const name[], int32_t position,
                                  bool endingAfter);

int64_t         c_SRAAlignmentEnumeratorReadID              (c_SRAAlignmentEnumerator *self);
char const *    c_SRAAlignmentEnumeratorReadGroup           (c_SRAAlignmentEnumerator *self, uint32_t *length);
int32_t         c_SRAAlignmentEnumeratorPlatform            (c_SRAAlignmentEnumerator *self);
int32_t         c_SRAAlignmentEnumeratorReadNumber          (c_SRAAlignmentEnumerator *self);
int32_t         c_SRAAlignmentEnumeratorReadLength          (c_SRAAlignmentEnumerator *self);
char const *    c_SRAAlignmentEnumeratorReadSequence        (c_SRAAlignmentEnumerator *self, uint32_t *length);
uint8_t const * c_SRAAlignmentEnumeratorReadQuality         (c_SRAAlignmentEnumerator *self, uint32_t *length);

int64_t         c_SRAAlignmentEnumeratorRefRowID            (c_SRAAlignmentEnumerator *self);
int32_t         c_SRAAlignmentEnumeratorAlignmentStart      (c_SRAAlignmentEnumerator *self);
int32_t         c_SRAAlignmentEnumeratorReferenceLength     (c_SRAAlignmentEnumerator *self);
uint32_t const *c_SRAAlignmentEnumeratorCigar               (c_SRAAlignmentEnumerator *self, uint32_t *length);
int32_t         c_SRAAlignmentEnumeratorMappingQuality      (c_SRAAlignmentEnumerator *self);

bool            c_SRAAlignmentEnumeratorReadPaired          (c_SRAAlignmentEnumerator *self);
bool            c_SRAAlignmentEnumeratorProperPair          (c_SRAAlignmentEnumerator *self);
bool            c_SRAAlignmentEnumeratorReadNegativeStrand  (c_SRAAlignmentEnumerator *self);
bool            c_SRAAlignmentEnumeratorNotPrimaryAlignment (c_SRAAlignmentEnumerator *self);
bool            c_SRAAlignmentEnumeratorReadFailsVendorQC   (c_SRAAlignmentEnumerator *self);
bool            c_SRAAlignmentEnumeratorReadIsDuplicate     (c_SRAAlignmentEnumerator *self);
bool            c_SRAAlignmentEnumeratorMateUnmapped        (c_SRAAlignmentEnumerator *self);
bool            c_SRAAlignmentEnumeratorMateNegativeStrand  (c_SRAAlignmentEnumerator *self);

int64_t         c_SRAAlignmentEnumeratorMateRefRowID        (c_SRAAlignmentEnumerator *self);
int32_t         c_SRAAlignmentEnumeratorMateAlignmentStart  (c_SRAAlignmentEnumerator *self);
int32_t         c_SRAAlignmentEnumeratorTemplateLength      (c_SRAAlignmentEnumerator *self);

#ifdef __cplusplus
}
#endif
