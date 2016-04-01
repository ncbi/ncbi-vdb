TOP=$1

#installing cipher module into newly created virtual env
tmp_py_env=$(mktemp -d)
virtualenv -p $(which python2) $tmp_py_env
. $tmp_py_env/bin/activate
tmp_cur_dir=$(pwd)
cd $TOP/libs/cipher/cipher-1.7/
$tmp_py_env/bin/python setup.py install
cd $tmp_cur_dir
unset tmp_cur_dir


#running cipher test in py virtual env
rm -f test.in test.enc test.out

for i in {0..10000}
do
    echo "Hello world $i" >> test.in
done

python $TOP/libs/cipher/cipher-1.7/encrypt.py --password=password123 test.in test.enc
python $TOP/libs/cipher/cipher-1.7/decrypt.py --password=password123 test.enc test.out

diff test.in test.out
exit_code=$?
rm test.in test.enc test.out

# cleanup
deactivate
rm -rf $tmp_py_env
unset tmp_py_env

exit $exit_code
