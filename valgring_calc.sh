#!/usr/bin/env bash

matrix_size=( 1023 1024 1025 1040 1041 1050 1100 )

for ms in "${matrix_size[@]}"
do
    echo "valgrind --tool=cachegrind ./cache $ms > l1l3.data 2>&1"
    valgrind --tool=cachegrind ./cache $ms > l1l3.data 2>&1
done

for ms in "${matrix_size[@]}"
do
    echo "valgrind --tool=cachegrind --LL=262144,8,64 ./cache $ms > l2.data 2>&1"
    valgrind --tool=cachegrind --LL=262144,8,64 ./cache $ms > l2.data 2>&1
done
