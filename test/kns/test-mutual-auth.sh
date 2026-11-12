#!/bin/sh

cd test-mutual-auth

CLIENT=client
SSL_LOG=ssl.log

# original instructions are in VDB-4465

#TODO: error checking everywhere

#Prepare keys and certificates
# Prepare certificate authority, server’s key and certificate, client’s key

#    create a certificate authority (CA), ca.key and ca.crt:
openssl req -new -x509 -nodes -days 365 -subj '/CN=my-ca' -keyout ca.key -out ca.crt 2>$SSL_LOG
#    create the server’s key server.key:
openssl genrsa -out server.key 2048 2>$SSL_LOG
#    create a Certificate Signing Request (CSR) with the Common Name (CN) localhost, server.csr:
openssl req -new -key server.key -subj '/CN=localhost' -out server.csr 2>$SSL_LOG
#    create the signed server certificate, server.crt:
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -days 365 -out server.crt 2>$SSL_LOG
#    creating the client’s key, client.key:
openssl genrsa -out client.key 2048 2>$SSL_LOG

#Create client’s certificate:

mkdir -p $CLIENT
cd $CLIENT
ln -f -s ../client.key pk_key
openssl req -new -key pk_key -subj '/CN=my-client' -out client.csr 2>$SSL_LOG
openssl x509 -req -in client.csr -CA ../ca.crt -CAkey ../ca.key -CAcreateserial -days 365 -out own_cert 2>$SSL_LOG

cd ..
node mtls.js &
#TODO: kill when done

# curl test
curl --cacert ca.crt --key $CLIENT/pk_key --cert $CLIENT/own_cert https://localhost:3001

# client test
echo "$1/test-kns-mutual-authentication $CLIENT https://localhost:3001"
$1/test-kns-mutual-authentication $CLIENT https://localhost:3001

exit $?
