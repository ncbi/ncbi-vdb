#!/bin/sh
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

# client mutial authentication test

CA=ca
CLIENT=client
SERVER=server
TEST_LOG=log.txt
KFG=cert.kfg

rm -f ${TEST_LOG}
rm -f ${KFG}

# original instructions are in VDB-4465

#Prepare keys and certificates
# Prepare certificate authority, server’s key and certificate, client’s key

mkdir -p ${CA}
#    create a certificate authority (CA), ca.key and ca.crt:
openssl req -new -x509 -nodes -days 365 -subj '/CN=my-ca' -keyout ${CA}/ca.key -out ${CA}/ca.crt 2>$TEST_LOG || { echo "****** openssl req ca.crt failed" >&2; exit 1; }

mkdir -p ${SERVER}
#    create the server’s key server.key:
openssl genrsa -out ${SERVER}/server.key 2048 2>>$TEST_LOG || { echo "****** openssl genrsa failed" >&2; exit 1; }
#    create a Certificate Signing Request (CSR) with the Common Name (CN) localhost, server.csr:
openssl req -new -key ${SERVER}/server.key -subj '/CN=localhost' -out ${SERVER}/server.csr 2>>$TEST_LOG || { echo "****** openssl req server.csr failed" >&2; exit 1; }
#    create the signed server certificate, server.crt:
openssl x509 -req -in ${SERVER}/server.csr -CA ${CA}/ca.crt -CAkey ${CA}/ca.key -CAcreateserial -days 365 -out ${SERVER}/server.crt 2>>$TEST_LOG || { echo "****** openssl req server.crt failed" >&2; exit 1; }

mkdir -p ${CLIENT}
#    create the client’s key, client.key:
openssl genrsa -out ${CLIENT}/client.key 2048 2>>$TEST_LOG || { echo "****** openssl genrsa client.key failed" >&2; exit 1; }

# Create client’s certificate:
openssl req -new -key ${CLIENT}/client.key -subj '/CN=my-client' -out ${CLIENT}/client.csr 2>>$TEST_LOG || { echo "****** openssl req client.csr failed" >&2; exit 1; }
openssl x509 -req -in ${CLIENT}/client.csr -CA ${CA}/ca.crt -CAkey ${CA}/ca.key -CAcreateserial -days 365 -out ${CLIENT}/signed_cert 2>>$TEST_LOG || { echo "****** openssl req signed_cert failed" >&2; exit 1; }

# start the https server
node mtls.js >>$TEST_LOG 2>&1 &
SERVER_PID=$!
sleep 1 # give the server time to initialize
#echo server started, PID=${SERVER_PID}

# curl test
#echo "curl -s --cacert ${CA}/ca.crt --key ${CLIENT}/pk_key --cert ${CLIENT}/signed_cert https://localhost:3000 >>$TEST_LOG"
curl -s --cacert ${CA}/ca.crt --key ${CLIENT}/client.key --cert ${CLIENT}/signed_cert https://localhost:3000 >>$TEST_LOG || { echo "****** curl test failed" >&2; kill ${SERVER_PID}; exit 1; }

# add certificate authority's certificate to test konfig as trusted
echo -n "/tls/ca.crt/test = \"" > ${KFG}
# convert into one-line double quoted string
cat ${CA}/ca.crt | sed ':a;N;$!ba;s/\n/\\n/g' | sed '$s/$/\"/' >> ${KFG}

#echo "$1/test-kns-mutual-authentication ${CLIENT} https://localhost:3000"
VDB_CONFIG=. $1/test-kns-mutual-authentication ${CLIENT} https://localhost:3000 #-+KNS -vvvv

RC=$?
#echo RC=${RC}

kill ${SERVER_PID}

exit $RC
