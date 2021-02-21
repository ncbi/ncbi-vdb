TOP=$1
CIPHER_DIR=${TOP}/libs/cipher/cipher-1.7

if [ "${PYTHON}" = "" ]; then
    echo "skipping python cipher test: PYTHON not defined"
    exit 0
fi

#installing cipher module into newly created virtual env
hostname=`hostname`
hostname | grep -q '^tcmac01$' && KEEP=1
KEEP=   # force virtual env. removal
#KEEP=1 # force saving the virtual env. to speed up tests
if [ "$KEEP" == "" ] ; then
    tmp_py_env=$(pwd)/temp_env_${hostname}
else
    tmp_py_env=$HOME/ncbi-vdb_test-cipher-env_${hostname}
fi

if [ "$KEEP" == "" ] ; then
    rm -fr $tmp_py_env
fi

if [ -d "$tmp_py_env" ] ; then
    echo found old virtual env dir
    FOUND=1
else
    echo creating new virtual env dir
fi

echo ${PYTHON}
${PYTHON} -V
${PYTHON} -m venv $tmp_py_env
ls -ld $tmp_py_env
. $tmp_py_env/bin/activate

#now inside the virtual env, python is ${PYTHON}

#echo pip install wheel...
pip install wheel

# The following creates "build dist .eggs" in ${CIPHER_DIR}.
if [ "$FOUND" == "" ] ; then
    tmp_cur_dir=$(pwd)
    cd $CIPHER_DIR
#   echo python setup.py install...
    python setup.py install
    cd $tmp_cur_dir
    unset tmp_cur_dir
else
    echo old virtual env found - skipped setup.py install
fi

echo "Running python cipher test..."

IN=test.in.${hostname}
ENC=test.enc.${hostname}
OUT=test.out.${hostname}

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

if [ "$KEEP" == "" ] ; then
    echo removing virtual env dir
    rm -rf $tmp_py_env
else
    echo keeping virtual env dir
fi

unset tmp_py_env

cd $CIPHER_DIR
rm -r build cipher.egg-info/SOURCES.txt dist

#ls -a .eggs
rm -fr .eggs/README.txt
rm -f .eggs/glob2-0.7-py3.*.egg
rm -fr .eggs/packit-0.27.1-py3.*.egg
rm -fr .eggs/packaging-20.9-py3.*.egg
rm -fr .eggs/pyparsing-3.0.0b2-py3.*.egg
rm -fr .eggs/pbr-5.5.1-py3.*.egg
rm -fr .eggs

exit $exit_code
