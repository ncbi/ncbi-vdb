rm -f test.in test.enc test.out

for i in {0..10000}
do
    echo "Hello world $i" >> test.in
done

python encrypt.py --password=password123 test.in test.enc
python decrypt.py --password=password123 test.enc test.out

diff test.in test.out
exit_code=$?
rm test.in test.enc test.out
exit $exit_code
