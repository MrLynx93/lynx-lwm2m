#!/bin/bash
# $1 - clients number
# $2 - servers number
# $3 - 0/1 tls
# $4 - times
make test
cd bin/lib/x64

for i in $(seq 1 $1); do
    ./test local_test_${i} $3 localhost:8883 $(($2*$4)) &
done
echo "Started all"