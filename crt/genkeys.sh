#!/bin/bash

#openssl req -out CSR.csr -new -newkey rsa:2048 -nodes -keyout server.key
#openssl req -x509 -sha256 -nodes -days 365 -newkey rsa:2048 -keyout server.key -out server.crt

openssl pkcs12 -export -name ca -in ca.crt -inkey ca.key -out ca.p12
keytool -importkeystore -destkeystore ca.jks -srckeystore ca.p12 -srcstoretype pkcs12 -alias ca


openssl pkcs12 -export -name mateusz-Lenovo-IdeaPad-Y580 -in mateusz-Lenovo-IdeaPad-Y580.crt -inkey mateusz-Lenovo-IdeaPad-Y580.key -out server.p12
keytool -importkeystore -destkeystore mateusz-Lenovo-IdeaPad-Y580.jks -srckeystore server.p12 -srcstoretype pkcs12 -alias mateusz-Lenovo-IdeaPad-Y580