#!python3
import sys

if __name__ != '__main__':
    sys.exit(0)
if sys.platform == 'linux':
    platforms = ('linux', )
    def libname(what):
        return f"lib{what}.so"
elif sys.platform == 'darwin':
    platforms = ('mac', )
    def libname(what):
        return f"lib{what}.dylib"
else:
    sys.exit(0)

import os
import random
import re

saveSysPath = sys.path
sys.path.append(os.path.abspath('../../py_vdb'))
from vdb import *
sys.path = saveSysPath

output = 'test-data' if len(sys.argv) < 2 else sys.argv[1]

def allStandardBuildLocations(product: str, leaf: str):
    try:
        home = os.environ['HOME']
    except KeyError:
        return

    path = [home, 'ncbi-outdir', product]
    for osname in platforms:
        path.append(osname)
        for ccname in ('clang', 'gcc', ):
            path.append(ccname)
            for arch in ('x86_64', 'i386', ):
                path.append(arch)
                for btype in ('dbg', 'rel', ):
                    yield os.path.join(*path, btype, leaf)
                del path[-1]
            del path[-1]
        del path[-1]

def findLibrary(what):
    paths = []
    try:
        seen = {}
        for path in os.environ['VDB_LIBRARY_PATH'].split(':'):
            if path in seen: continue
            seen[path] = True
            if os.path.isdir(path):
                paths.append(path)
    except KeyError:
        paths = [x for x in allStandardBuildLocations('ncbi-vdb', 'lib') if os.path.isdir(x)]

    for path in paths:
        results = []
        fullpath = os.path.join(path, libname(what))
        if os.path.isfile(fullpath):
            return os.path.realpath(fullpath)

    print('fatal: Can not find ncbi-wvdb dynamic library, try setting VDB_LIBRARY_PATH')
    exit(1)

cursDef = {
    'READ': {},
    'READ_START': {},
    'READ_LEN': {},
    'READ_TYPE': {},
    'READ_FILTER': {},
    'QUALITY': {
        'expression': '(INSDC:quality:text:phred_64)QUALITY'
    },
    'RAW_NAME': {},
    'SPOT_GROUP': {},
    'PLATFORM': {}
}

data = (
    ('CTCAGGCAAAGACGCAGCGAAATCGTCATAACTCTCAGCAGGACCACCAGTCTTCTCAACCACAACCTCCCTTTTCTTTTCATCAACCTTAAAGATGACACAAATTCACTACTCTTTCATCCCTTTAAATCCTTCATACATTTTCCATGTCTTTCAGAGGTCTCAGCCGGCCAAATGCTTCATCTGGCATGGGTGTTGAT',
     [0, 100,],
     [100, 100,],
     [1, 1,],
     [0, 0,],
     r'''deecdcddebeZded_ede_cbdda\ccbddebcadWadddUKVeebe__^dZcd^dUKdd`bWYde_cddbddebbedcecad`ddeUbedaaR`Xcee_eecQ]b`ccebZb^Y]ca[acaZScdc\`ca`UdaZacd`_ddbce\bTddddcb`cdXd\aebb^b`ad`cceda]ed_eY_eS^ceedN`ddb`cO^''',
     'CL100048465L2C001R015_402436',
     '',
     0,
    ),
    ('CTTAGATATGTCGGGGGTGAAACACGGATTATTAGTATAAGAAAGGACATAAGTTGGCCGGAGCTTATGCAGAAAATATTATCGATCTATAATGAAACTCGCGAGTTTCATTATAGATCGATAATATTTTCTGCATAAGCTCCGGCCAACTTATGTCCTTTCTTATACTAATAATCCGTGTTTCACCCCCGGCATATCTA',
     [0, 100,],
     [100, 100,],
     [1, 1,],
     [0, 0,],
     r'''ffebeeefefefffffeeeeeeeeffeeeeeeedeeeeddeeeefeeeeedeee_fefffeeeeefedeffeeefeffddfdebddee_ed[dffcffdbeeeeeeeeffee`efeeeeffeeeeeeeeffffeeed`ffefefeff`efVdc`f_dffeedeededfebdee\`fed_feeee\efeefcGbS]ec_dW''',
     'CL100048465L2C001R001_5',
     '',
     0,
    ),
)

wvdb = findLibrary('ncbi-wvdb')
#print(wvdb);sys.exit(0)

mgr = manager(OpenMode.Write, wvdb)
print('info: loaded VDBManager', mgr.Version(), 'from', wvdb)
mgr.InitLogging('test-raw-name.py')
mgr.SetLogLevel(5)

schema = mgr.MakeSchema()
schema.AddIncludePath(os.path.abspath('../../interfaces'))
schema.ParseFile('sra/generic-fastq.vschema')
db = mgr.CreateDB(schema, 'NCBI:SRA:GenericFastq:db', output)
schema = None
mgr = None
tbl = db.CreateTable('SEQUENCE')
db = None
curs = tbl.CreateCursor(OpenMode.Write)
tbl = None
cols = {}
for name, define in cursDef.items():
    cols[name] = curs.AddColumn(define.get('expression', name))
curs.Open()
for col in cols.values():
    col._update()
for name, define in cursDef.items():
    try:
        cols[name].set_default(define['default'])
    except KeyError:
        pass

def writeTestData(values: list):
    assert len(values[0]) == len(values[5])
    curs.OpenRow()

    cols['READ'].write(values[0])
    cols['READ_START'].write(values[1])
    cols['READ_LEN'].write(values[2])
    cols['READ_TYPE'].write(values[3])
    cols['READ_FILTER'].write(values[4])
    cols['QUALITY'].write(values[5])
    cols['RAW_NAME'].write(values[6])
    cols['SPOT_GROUP'].write(values[7])
    cols['PLATFORM'].write(values[8])

    curs.CommitRow()
    curs.CloseRow()
    return True

good = all(map(writeTestData, data))

cols = None
if good: curs.Commit()
curs = None

assert good
