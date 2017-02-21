#!/bin/bash
# $1 - clients number
# $2 - servers number
# $3 - 0/1 tls
# $4 - times
# $5 - broker
make test
cd bin/lib/x64

for i in $(seq 1 $1); do
    ulimit -c unlimited
    ./test local_test_${i} $3 $5 $(($2*$4)) > local_test_${i}.log &2>1 &
done
echo "Started all"
