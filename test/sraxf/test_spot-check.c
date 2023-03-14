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
#include "../../libs/sraxf/spot-check.c"

/* test for success */
static void check_check_quality1(self_t const *self) {
    int32_t const start[] = { 0, 75 };
    uint32_t const len[] = { 75, 75 };
    uint8_t const type[] = { SRA_READ_TYPE_BIOLOGICAL|SRA_READ_TYPE_FORWARD
        ,SRA_READ_TYPE_BIOLOGICAL|SRA_READ_TYPE_REVERSE };
    uint8_t qual[75+75];
    int result;

    memset(qual, 20, 150);
    result = check_quality(self, 2, start, len, type, qual);
    assert(result == notRejected);
}

/* test short biological read fails  */ 
static void check_check_quality2(self_t const *self) {
    int32_t const start[] = { 0, 9 };
    uint32_t const len[] = { 9, 75 };
    uint8_t const type[] = { SRA_READ_TYPE_BIOLOGICAL|SRA_READ_TYPE_FORWARD
        ,SRA_READ_TYPE_BIOLOGICAL|SRA_READ_TYPE_REVERSE };
    uint8_t qual[75+75];
    int result;

    memset(qual, 20, 150);
    result = check_quality(self, 2, start, len, type, qual);
    assert(result == tooShort);
}

/* test short technical read is okay */
static void check_check_quality3(self_t const *self) {
    int32_t const start[] = { 0, 9 };
    uint32_t const len[] = { 9, 75 };
    uint8_t const type[] = { SRA_READ_TYPE_TECHNICAL|SRA_READ_TYPE_FORWARD
        ,SRA_READ_TYPE_BIOLOGICAL|SRA_READ_TYPE_REVERSE };
    uint8_t qual[75+75];
    int result;

    memset(qual, 20, 150);
    result = check_quality(self, 2, start, len, type, qual);
    assert(result == notRejected);
}

/* test bad==50% doesn't fail */
static void check_check_quality4(self_t const *self) {
    int32_t const start[] = { 0, 75 };
    uint32_t const len[] = { 75, 75 };
    uint8_t const type[] = { SRA_READ_TYPE_BIOLOGICAL|SRA_READ_TYPE_FORWARD
        ,SRA_READ_TYPE_BIOLOGICAL|SRA_READ_TYPE_REVERSE };
    uint8_t qual[75+75];
    int result;

    memset(qual, 20, 150);
    for (result = 0; result < 150; ++result) {
        if ((result + 1) % 2 == 0)
            qual[result] = 5;
    }
    result = check_quality(self, 2, start, len, type, qual);
    assert(result == notRejected);
}

/* test >50% bad fails */
static void check_check_quality5(self_t const *self) {
    int32_t const start[] = { 0, 75 };
    uint32_t const len[] = { 75, 75 };
    uint8_t const type[] = { SRA_READ_TYPE_BIOLOGICAL|SRA_READ_TYPE_FORWARD
        ,SRA_READ_TYPE_BIOLOGICAL|SRA_READ_TYPE_REVERSE };
    uint8_t qual[75+75];
    int result;

    memset(qual, 20, 150);
    for (result = 0; result < 150; ++result) {
        if ((result + 1) % 2 == 0)
            qual[result] = 5;
    }
    qual[20] = 5;
    result = check_quality(self, 2, start, len, type, qual);
    assert(result == tooManyBadQualValues);
}

/* test 9 bad on leading edge doesn't fail */
static void check_check_quality6(self_t const *self) {
    int32_t const start[] = { 0, 75 };
    uint32_t const len[] = { 75, 75 };
    uint8_t const type[] = { SRA_READ_TYPE_BIOLOGICAL|SRA_READ_TYPE_FORWARD
        ,SRA_READ_TYPE_BIOLOGICAL|SRA_READ_TYPE_REVERSE };
    uint8_t qual[75+75];
    int result;

    memset(qual, 20, 150);
    for (result = 0; result < 9; ++result) {
        qual[result] = 5;
    }
    result = check_quality(self, 2, start, len, type, qual);
    assert(result == notRejected);
}

/* test 10 bad on leading edge fails */
static void check_check_quality7(self_t const *self) {
    int32_t const start[] = { 0, 75 };
    uint32_t const len[] = { 75, 75 };
    uint8_t const type[] = { SRA_READ_TYPE_BIOLOGICAL|SRA_READ_TYPE_FORWARD
        ,SRA_READ_TYPE_BIOLOGICAL|SRA_READ_TYPE_REVERSE };
    uint8_t qual[75+75];
    int result;

    memset(qual, 20, 150);
    for (result = 0; result < 10; ++result) {
        qual[result] = 5;
    }
    result = check_quality(self, 2, start, len, type, qual);
    assert(result == badQualValueFirstM);
}

/* test 9 bad on trailing edge doesn't fail */
static void check_check_quality8(self_t const *self) {
    int32_t const start[] = { 0, 75 };
    uint32_t const len[] = { 75, 75 };
    uint8_t const type[] = { SRA_READ_TYPE_BIOLOGICAL|SRA_READ_TYPE_FORWARD
        ,SRA_READ_TYPE_BIOLOGICAL|SRA_READ_TYPE_REVERSE };
    uint8_t qual[75+75];
    int result;

    memset(qual, 20, 150);
    for (result = 66; result < 75; ++result) {
        qual[result] = 5;
    }
    result = check_quality(self, 2, start, len, type, qual);
    assert(result == notRejected);
}

/* test 10 bad on trailing edge fails */
static void check_check_quality9(self_t const *self) {
    int32_t const start[] = { 0, 75 };
    uint32_t const len[] = { 75, 75 };
    uint8_t const type[] = { SRA_READ_TYPE_BIOLOGICAL|SRA_READ_TYPE_FORWARD
        ,SRA_READ_TYPE_BIOLOGICAL|SRA_READ_TYPE_REVERSE };
    uint8_t qual[75+75];
    int result;

    memset(qual, 20, 150);
    for (result = 65; result < 75; ++result) {
        qual[result] = 5;
    }
    result = check_quality(self, 2, start, len, type, qual);
    assert(result == badQualValueFirstM);
}

static void check_check_quality() {
    self_t self = {
        NULL
        , 10 /* lead-in/lead-out length */
        , 20 /* the low quality threshold */
        , -1
    };
    check_check_quality1(&self);
    check_check_quality2(&self);
    check_check_quality3(&self);
    check_check_quality4(&self);
    check_check_quality5(&self);
    check_check_quality6(&self);
    check_check_quality7(&self);
    check_check_quality8(&self);
    check_check_quality9(&self);
}

int main(int argc, char *argv[]) {
    check_check_quality();
}
