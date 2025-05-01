TOP=$1
CIPHER_DIR=${TOP}/libs/cipher/cipher-1.7
VIRTUALENV=$(which virtualenv)

if [ "${PYTHON}" = "" ]; then
    echo "skipping python cipher test: PYTHON not defined"
    exit 0
fi
if [ "${VIRTUALENV}" = "" ]; then
    echo "skipping python cipher test: no virtualenv"
    exit 0
fi

#installing cipher module into newly created virtual env
tmp_py_env=$(pwd)/temp_env

${PYTHON} -V
${VIRTUALENV} -p ${PYTHON} $tmp_py_env || exit 1
. $tmp_py_env/bin/activate             || exit 2

#now inside the virtual env, python is ${PYTHON}

# the following creates "build dist .eggs" in $CIPHER_DIR
tmp_cur_dir=$(pwd)
cd $CIPHER_DIR          || exit 3
python -m pip install . || exit 4
cd $tmp_cur_dir         || exit 5
unset tmp_cur_dir

echo "Running python cipher test..."

#running cipher test in py virtual env
rm -f test.in test.enc test.out

for i in {0..10000}
do
    echo "Hello world $i" >> test.in
done

python ${CIPHER_DIR}/encrypt.py --password=password123 test.in test.enc ||exit 6
python ${CIPHER_DIR}/decrypt.py --password=password123 test.enc test.out||exit 7

diff test.in test.out
exit_code=$?
rm test.in test.enc test.out

echo "python cipher test is complete."

# cleanup
deactivate || exit 8
#now outside the virtual env

rm -rf $tmp_py_env
unset tmp_py_env

cd $CIPHER_DIR || exit 9
rm -rf packit-0.18-py2.7.egg pbr-3.1.1-py2.7.egg wheel-0.30.0-py2.7.egg
rm -vf glob2-0.4.1-py2.7.egg

exit $exit_code
