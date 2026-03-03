# ===========================================================================
#
#                            PUBLIC DOMAIN NOTICE
#               National Center for Biotechnology Information
#
#  This software/database is a "United States Government Work" under the
#  terms of the United States Copyright Act.  It was written as part of
#  the author's official duties as a United States Government employee and
#  thus cannot be copyrighted.  This software/database is freely available
#  to the public for use. The National Library of Medicine and the U.S.
#  Government have not placed any restriction on its use or reproduction.
#
#  Although all reasonable efforts have been taken to ensure the accuracy
#  and reliability of the software and data, the NLM and the U.S.
#  Government do not and cannot warrant the performance or results that
#  may be obtained by using this software or data. The NLM and the U.S.
#  Government disclaim all warranties, express or implied, including
#  warranties of performance, merchantability or fitness for any particular
#  purpose.
#
#  Please cite the author in any work or product based on this material.
#
# ===========================================================================

TOP=$1
CIPHER_DIR=${TOP}/libs/cipher/cipher-1.7
VIRTUALENV=$(which virtualenv)

if [ "${PYTHON}" = "" ]; then
    echo "skipping python cipher test: PYTHON not defined"
    exit 0
fi

#installing cipher module into newly created virtual env
tmp_py_env=$(pwd)/temp_env_${OS}
rm -f $tmp_py_env

if [ "${VIRTUALENV}" != "" ]; then
   virtualenv -p ${PYTHON} $tmp_py_env || exit 1
else
   ${PYTHON} -m venv $tmp_py_env       || exit 1
fi
. $tmp_py_env/bin/activate             || exit 2

#now inside the virtual env, python is python
if [ "${OS}" = "mac" ]; then
  pip install --upgrade pip setuptools wheel
fi

# the following creates "build dist .eggs" in $CIPHER_DIR
tmp_cur_dir=$(pwd)
cd $CIPHER_DIR                       || exit 3
if [ "${OS}" = "mac" ]; then
  PFX="python $CIPHER_DIR/"
  python -m pip install --no-build-isolation . || exit 4
else
  PFX=
  lsb_release=`lsb_release -is`
  if [ "$lsb_release" = "Ubuntu" ]; then
    python -m pip install              . || exit 4
  else
    python -m pip install --use-pep517 . || exit 4
  fi
fi
cd $tmp_cur_dir                      || exit 5
unset tmp_cur_dir

echo "Running python cipher test..."

#running cipher test in py virtual env
rm -f test.in test.enc test.out
if [ "${OS}" = "mac" ]; then
    rm -f libncbi-vdb.dylib
    ln -s $LD_LIBRARY_PATH/libncbi-vdb.dylib || exit 6
fi

for i in {0..10000}
do
    echo "Hello world $i" >> test.in
done

${PFX}encrypt.py --password=password123 test.in  test.enc || exit 7
${PFX}decrypt.py --password=password123 test.enc test.out || exit 8

diff test.in test.out
exit_code=$?
rm test.in test.enc test.out
if [ "${OS}" = "mac" ]; then rm libncbi-vdb.dylib; fi

echo "...python cipher test is complete."

# cleanup
deactivate || exit 9
#now outside the virtual env

rm -rf $tmp_py_env
unset tmp_py_env

cd $CIPHER_DIR || exit 10
rm -rf packit-0.18-py2.7.egg pbr-3.1.1-py2.7.egg wheel-0.30.0-py2.7.egg
rm -vf glob2-0.4.1-py2.7.egg

exit $exit_code
