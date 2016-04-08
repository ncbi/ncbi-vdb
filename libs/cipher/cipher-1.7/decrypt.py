#!/usr/bin/env python
import cipher
import getpass
from optparse import OptionParser
import shutil
import sys


if __name__ == '__main__':
    version = '%prog 1.0'
    parser = OptionParser(usage='''usage: %prog [options] INFILE OUTFILE
    Decrypts file''', version=version)
    parser.add_option('-P', '--password', dest='password', default=None)
    args, values = parser.parse_args()
    if len(values) < 2:
        parser.print_usage()
        sys.exit(2)
    in_file, out_file = values
    if not args.password:
        args.password = getpass.getpass()

    with open(in_file, 'rb') as in_f:
        with open(out_file, 'wb') as out_f:
            dec = cipher.Decryptor(args.password.encode('ascii'), in_f.fileno(), close=False)
            shutil.copyfileobj(dec, out_f)
