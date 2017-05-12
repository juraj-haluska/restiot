#!/bin/bash
CERT="CERT"
KEY="KEY"

openssl req -x509 -newkey rsa:2014 -keyout $KEY.pem -out $CERT.pem -days 365 -nodes

NEW=$CERT.h
touch $NEW

echo "const char $CERT [] =" > $NEW

while IFS='' read -r line || [[ -n "$line" ]]; do
    echo "    \"$line\\n\"" >> $NEW
done < $CERT.pem

echo "$(cat $NEW);" > $NEW

NEW=$KEY.h
touch $NEW

echo "const char $KEY [] =" > $NEW

while IFS='' read -r line || [[ -n "$line" ]]; do
    echo "    \"$line\\n\"" >> $NEW
done < $KEY.pem

echo "$(cat $NEW);" > $NEW

rm CERT.pem KEY.pem

echo "done, generated files:"
echo -e "\tcert => $CERT.h"
echo -e "\tkey => $KEY.h"