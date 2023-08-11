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
 * Testing functions from <vdb/schema-priv.h>
 * ========================================================================== */

#include <kapp/args.h> // ArgsWhack
#include <kfg/config.h> /* KConfigDisableUserSetting */
#include <klib/vector.h> // VectorForEach
#include <ktst/unit_test.hpp> /* TEST_SUITE_WITH_ARGS_HANDLER */
#include <vdb/database.h> // VDatabaseRelease
#include <vdb/manager.h> // VDBManagerRelease
#include <vdb/schema.h> // VSchemaRelease
#include <vdb/schema-priv.h> // VSchemaGetDb

static rc_t argsHandler(int argc, char *argv[]) {
    Args *args = NULL;
    rc_t rc = ArgsMakeAndHandle(&args, argc, argv, 0, NULL, 0);
    ArgsWhack(args);
    return rc;
}
TEST_SUITE_WITH_ARGS_HANDLER(SchemaPrivSuite, argsHandler)

static void OnDb(void *item, void *data) {
    assert(item && data);
    const struct SDatabase *self(static_cast<const struct SDatabase*>(item));
    rc_t *aRc = static_cast<rc_t*>(data);
    if (*aRc != 0)
        return;

    // SDatabaseMakeKSymbolName
    KSymbolName *sn(NULL);
    rc_t rc(SDatabaseMakeKSymbolName(self, &sn));
    if (rc != 0)
        *aRc = rc;
    else if (sn->version != 0x01030000)
        *aRc = 1;
    else if (sn->name->next == NULL)
        *aRc = 2;
    else {
#ifdef SHOW_RESULTS
        std::cerr << "DB-NAME: " <<
            std::string(sn->name->name->addr, sn->name->name->size) << std::endl;
#endif
        String name;
        CONST_STRING(&name, "NCBI");
        if (!StringEqual(&name, sn->name->name))
            *aRc = 2;
    }

    // SDatabaseGetDad
    const struct SDatabase *dd(NULL);
    rc = SDatabaseGetDad(self, &dd);
    if (rc != 0)
        *aRc = rc;
    if (rc != 0)
        *aRc = rc;
    if (rc != 0)
        *aRc = rc;
   
    // KSymbolNameWhack
    rc = KSymbolNameWhack(sn);
    if (rc != 0)
        *aRc = rc;
}

static void OnTbl(void *item, void *data) {
    assert(item && data);
    const struct STable *self(static_cast<const struct STable*>(item));
    rc_t *aRc = static_cast<rc_t*>(data);
    if (*aRc != 0)
        return;

    // STableMakeKSymbolName
    KSymbolName *sn(NULL);
    rc_t rc(STableMakeKSymbolName(self, &sn));
    assert(sn->name->name);
    if (rc != 0)
        *aRc = rc;
    else if (sn->name->next == NULL)
        *aRc = 10;
    else if (sn->name->name->size == 0)
        *aRc = 20;
#ifdef SHOW_RESULTS
    else
        std::cerr << "TABLE-NAME: " <<
           std::string(sn->name->name->addr, sn->name->name->size) << std::endl;
#endif
}

TEST_CASE(TestSchemaPriv) {
    const VDBManager *m(NULL);
    REQUIRE_RC(VDBManagerMakeRead(&m, NULL));
  
    const VDatabase *db(NULL);
    REQUIRE_RC(VDBManagerOpenDBRead(m, &db, NULL, "db/VDB-3418.sra"));
    
    const VSchema *schema(NULL);
    REQUIRE_RC(VDatabaseOpenSchema(db, &schema));
   
    // VSchemaGetDb
    const Vector *v(NULL);
    REQUIRE_RC(VSchemaGetDb(schema, &v));
    REQUIRE_NOT_NULL(v);
    rc_t rc(0);
    VectorForEach(v, false, OnDb, &rc);

    // VSchemaGetView
    v = NULL;
    REQUIRE_RC(VSchemaGetView(schema, &v));
    REQUIRE_NOT_NULL(v);
    REQUIRE_NULL(VectorFirst(v));
    // SViewGetParents, SViewMakeKSymbolName:
    // cannot find existing run with views in schema

    // VSchemaGetTbl
    v = NULL;
    REQUIRE_RC(VSchemaGetTbl(schema, &v));
    REQUIRE_NOT_NULL(v);
    VectorForEach(v, false, OnTbl, &rc);
 
    REQUIRE_RC(rc);
   
    REQUIRE_RC(VSchemaRelease(schema));
   
    REQUIRE_RC(VDatabaseRelease(db));
   
    REQUIRE_RC(VDBManagerRelease(m));
}

extern "C" {
    rc_t CC KMain(int argc, char *argv[]) {
        KConfigDisableUserSettings();
        return SchemaPrivSuite(argc, argv);
    }
    ver_t CC KAppVersion(void) { return 0; }
    const char UsageDefaultName[]("Test_VDB_schema_priv");
    rc_t CC Usage(const Args *args) { return 0; }
    rc_t CC UsageSummary(const char *progname) { return 0; }
}
