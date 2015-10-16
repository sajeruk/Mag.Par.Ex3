#!/usr/bin/env bash

matrix_size=( 1023 1024 1025 1040 1041 1050 1100 )
cache_size_degree=( 15 18 22 )
woa=8

for r in `seq 0 1`
do
    for ms in "${matrix_size[@]}"
    do
        for deg in "${cache_size_degree[@]}"
        do
            if [ "$deg" == "22" ];
            then
                woa=16
            else
                woa=8
            fi
            #echo "./cache " $ms $deg $woa $r
            ./cache $ms $deg $woa $r
        done
    done
done

# additional test
for ms in `seq  63 65`
do
    #echo "./cache " $ms 13 8 0
    ./cache $ms 13 8 0
done

