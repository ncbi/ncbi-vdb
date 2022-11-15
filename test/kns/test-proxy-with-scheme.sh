#!/bin/bash

test_proxy_with_scheme=$1

echo test-proxy-with-scheme: BEGIN ...

# no proxy specified: fail
unset http_proxy
output=$($test_proxy_with_scheme)
res=$?
if [ "$res" != "0" ];
	then echo "test-proxy-with-scheme: no proxy specified FAILED: should return 0, actual res=$res output=$output" && exit 1;
fi

# bad proxy in configuration: fail
unset http_proxy
output=$($test_proxy_with_scheme =BAD.proxy.ncbi.nlm.nih.gov)
res=$?
if [ "$res" != "0" ];
	then echo "test-proxy-with-scheme: bad proxy in configuration: should return 0: res=$res output=$K" && exit 1;
fi

# good proxy in configuration: success
output=$(ping -c1 webproxy > /dev/null)
res=$?
if [ "$res" == "0" ]
then
	output=$($test_proxy_with_scheme =webproxy.ncbi.nlm.nih.gov  SUCCESS);
	res=$?
	if [ "$res" != 0 ]
		then echo "good proxy in configuration FAILED: res=$res, output=$output"
		exit 1
	fi
else
	echo "skipped good proxy in configuration: res=$res, output=$output"
fi

# bad proxy in environment: fail
http_proxy=BAD.proxy.ncbi.nlm.nih.gov $test_proxy_with_scheme
res=$?
if [ "$res" != 0 ]
	then echo "bad proxy in environment FAILED: res=$res, output=$output"
	exit 1
fi


# good proxy in environment: success
output=$(ping -c1 webproxy > /dev/null)
res=$?
if [ "$res" == "0" ]
then
	output=$(all_proxy=webproxy.ncbi.nlm.nih.gov $test_proxy_with_scheme SUCCESS)
	res=$?
	if [ "$res" != 0 ]
		then echo "good proxy in environment: FAILED: res=$res, output=$output"
		exit 1
	fi
else
	echo skipped good proxy in environment: success
fi


# good proxy with schema in environment: success
output=$(ping -c1 webproxy > /dev/null)
res=$?
if [ "$res" == "0" ]
then
	output=$(ALL_PROXY=http://webproxy.ncbi.nlm.nih.gov  $test_proxy_with_scheme SUCCESS)
	res=$?
	if [ "$res" != 0 ]
		then echo "good proxy with schema in environment: FAILED: res=$res, output=$output"
		exit 1
	fi
else
	echo skipped good proxy with schema in environment: success
fi

# good proxy with schema and port in environment: success
output=$(ping -c1 webproxy > /dev/null)
res=$?
if [ "$res" == "0" ]
then
	output=$(http_proxy=http://webproxy.ncbi.nlm.nih.gov:3128  $test_proxy_with_scheme SUCCESS)
	res=$?
	if [ "$res" != 0 ]
		then echo "good proxy with schema and port in environment: FAILED: res=$res, output=$output"
		exit 1
	fi
else
	echo skipped good proxy with schema and port in environment: success
fi

# good proxy with any schema in environment: success
output=$(ping -c1 webproxy > /dev/null)
res=$?
if [ "$res" == "0" ]
then
	output=$(HTTP_PROXY=anySchemaWillBeIgnored://webproxy.ncbi.nlm.nih.gov $test_proxy_with_scheme SUCCESS)
	res=$?
	if [ "$res" != 0 ]
		then echo "good proxy with any schema in environment: FAILED: res=$res, output=$output"
		exit 1
	fi
else
	echo skipped good proxy with any schema in environment: success
fi

# bad proxy port in environment: fail
output=$(http_proxy=http://webproxy.ncbi.nlm.nih.gov:3 $test_proxy_with_scheme)
res=$?
if [ "$res" != 0 ]
	then echo "bad proxy port in environment: FAILED: res=$res, output=$output"
	exit 1
fi

# bad proxy specifications in environment: fail
output=$(http_proxy=h            $test_proxy_with_scheme)
res=$?
if [ "$res" != 0 ]
	then echo "bad proxy specifications in environment: FAILED: res=$res, output=$output"
	exit 1
fi

output=$(http_proxy=:            $test_proxy_with_scheme)
res=$?
if [ "$res" != 0 ]
	then echo "bad proxy specifications in environment: FAILED: res=$res, output=$output"
	exit 1
fi

output=$(http_proxy=:a           $test_proxy_with_scheme)
res=$?
if [ "$res" != 0 ]
	then echo "bad proxy specifications in environment: FAILED: res=$res, output=$output"
	exit 1
fi

output=$(http_proxy=:2           $test_proxy_with_scheme)
res=$?
if [ "$res" != 0 ]
	then echo "bad proxy specifications in environment: FAILED: res=$res, output=$output"
	exit 1
fi

output=$(http_proxy=h:/w.g:3128  $test_proxy_with_scheme)
res=$?
if [ "$res" != 0 ]
	then echo "bad proxy specifications in environment: FAILED: res=$res, output=$output"
	exit 1
fi

output=$(http_proxy=http://      $test_proxy_with_scheme)
res=$?
if [ "$res" != 0 ]
	then echo "bad proxy specifications in environment: FAILED: res=$res, output=$output"
	exit 1
fi

output=$(http_proxy=http://:3128 $test_proxy_with_scheme)
res=$?
if [ "$res" != 0 ]
	then echo "bad proxy specifications in environment: FAILED: res=$res, output=$output"
	exit 1
fi

# VDB-3329: http_proxy specification with trailing slash
output=$(ping -c1 webproxy > /dev/null)
res=$?
if [ "$res" == "0" ]
then
	output=$(http_proxy=http://webproxy.ncbi.nlm.nih.gov:3128/ $test_proxy_with_scheme SUCCESS)
	res=$?
	if [ "$res" != 0 ]
		then echo "VDB-3329: http_proxy specification with trailing slash: FAILED: res=$res, output=$output"
		exit 1
	fi
else
	echo skipped VDB-3329: http_proxy specification with trailing slash
fi


output=$(ping -c1 webproxy > /dev/null)
res=$?
if [ "$res" == "0" ]
then
	output=$(http_proxy=http://webproxy.ncbi.nlm.nih.gov:3128/// $test_proxy_with_scheme SUCCESS)
	res=$?
	if [ "$res" != 0 ]
		then echo "VDB-3329: http_proxy specification with trailing 3 slash: FAILED: res=$res, output=$output"
		exit 1
	fi
else
	echo skipped VDB-3329: http_proxy specification with trailing 3 slash
fi

output=$(ping -c1 webproxy > /dev/null)
res=$?
if [ "$res" == "0" ]
then
	output=$(http_proxy=http://webproxy.ncbi.nlm.nih.gov:3128/: $test_proxy_with_scheme SUCCESS)
	res=$?
	if [ "$res" != 0 ]
		then echo "VDB-3329: http_proxy specification with trailing slash colon: FAILED: res=$res, output=$output"
		exit 1
	fi
else
	echo skipped VDB-3329: http_proxy specification with trailing slash colon
fi

output=$(ping -c1 webproxy > /dev/null)
res=$?
if [ "$res" == "0" ]
then
	output=$(http_proxy=http://webproxy.ncbi.nlm.nih.gov:3128???XY $test_proxy_with_scheme SUCCESS)
	res=$?
	if [ "$res" != 0 ]
		then echo "VDB-3329: http_proxy specification with trailing trash: FAILED: res=$res, output=$output"
		exit 1
	fi
else
	echo skipped VDB-3329: http_proxy specification with trailing trash
fi

output=$(http_proxy=http://webproxy.ncbi.nlm.nih.gov/X $test_proxy_with_scheme)
res=$?
if [ "$res" != 0 ]
	then echo "http_proxy=http://webproxy.ncbi.nlm.nih.gov/X: FAILED: res=$res, output=$output"
	exit 1
fi

output=$(http_proxy=http://webproxy.ncbi.nlm.nih.gov?X $test_proxy_with_scheme)
res=$?
if [ "$res" != 0 ]
	then echo "http_proxy=http://webproxy.ncbi.nlm.nih.gov?X: FAILED: res=$res, output=$output"
	exit 1
fi

echo ... test-proxy-with-scheme: END

exit 0
