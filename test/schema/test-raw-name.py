#!python3
import sys

if __name__ != '__main__':
    sys.exit(0)
if sys.platform != 'linux' and sys.platform != 'darwin':
    sys.exit(0)

import os
import random
import re

saveSysPath = sys.path
sys.path.append(os.path.abspath('../../py_vdb'))
from vdb import *
sys.path = saveSysPath

output = 'test-data' if len(sys.argv) < 2 else sys.argv[1]

def findLibrary(what):
    paths = []
    try:
        paths = os.environ['VDB_LIBRARY_PATH'].split(':')
    except:
        pass

    for path in paths:
        results = []
        for file in os.listdir(path):
            if re.search(what, file) == None:
                continue
            if os.path.isfile(os.path.join(path, file)):
                results.append(file)
        if len(results) != 0:
            return os.path.realpath(os.path.join(path, sorted(results)[-1]))

    print('fatal: Can not find ncbi-wvdb dynamic library, try setting VDB_LIBRARY_PATH')
    exit(1)

cursDef = {
     'RAW_NAME': {},
}

wvdb = findLibrary(r'ncbi-wvdb')
#print(wvdb);sys.exit(0)
mgr = manager(OpenMode.Write, wvdb)
print('info: loaded VDBManager', mgr.Version(), 'from', wvdb)
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

def writeTestData(value:str):
    curs.OpenRow()

    cols['RAW_NAME'].write(value)

    curs.CommitRow()
    curs.CloseRow()
    return True

good = all(map(writeTestData, ('FOO', 'GOO', 'BAR', 'BAZZOOOOOM')))

cols = None
if good: curs.Commit()
curs = None

assert good
