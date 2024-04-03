#!/bin/sh

test_proxy=$1

echo test-proxy BEGIN ...

# no proxy specified: fail
unset http_proxy
output=$($test_proxy)
res=$?
if [ "$res" != "0" ];
	then echo "test-proxy failed, res=$res output=$output"
	exit 1
fi

echo ... test-proxy: END

exit 0
