TOP=$1
CIPHER_DIR=${TOP}/libs/cipher/cipher-1.7
VIRTUALENV=$(which virtualenv)

if [ "${PYTHON}" = "" ]; then
    echo "skipping python cipher test: PYTHON not defined"
    exit 0
fi

#installing cipher module into newly created virtual env
hostname=`hostname`
tmp_py_env=$(pwd)/temp_env_${hostname}

rm -fr $tmp_py_env

${PYTHON} -V
${PYTHON} -m venv $tmp_py_env
ls -ld $tmp_py_env
. $tmp_py_env/bin/activate

#now inside the virtual env, python is ${PYTHON}

pip install wheel

# The following creates "build dist .eggs" in ${CIPHER_DIR}.
# Correction: .egg is kept to speed up tests.
tmp_cur_dir=$(pwd)
cd $CIPHER_DIR
python setup.py install
cd $tmp_cur_dir
unset tmp_cur_dir

echo "Running python cipher test..."

IN=test.in${hostname}
ENC=test.enc${hostname}
OUT=test.out${hostname}

#running cipher test in py virtual env
rm -f $IN $ENC $OUT

for i in {0..10000}
do
    echo "Hello world $i" >> $IN
done

python ${CIPHER_DIR}/encrypt.py --password=password123 $IN $ENC
python ${CIPHER_DIR}/decrypt.py --password=password123 $ENC $OUT

diff $IN $OUT
exit_code=$?
rm $IN $ENC $OUT

echo "python cipher test is complete."

# cleanup
deactivate
#now outside the virtual env

rm -rf $tmp_py_env
unset tmp_py_env

cd $CIPHER_DIR
rm -r build cipher.egg-info/SOURCES.txt dist
# keep .eggs to speed up running on tcmac*

exit $exit_code
