#!/usr/bin/env bash
prefix="/sys/devices/system/cpu/cpu0/cache/"
printf "L1 cache size: "
cat "$prefix/index1/size"
printf "L1 ways count: "
cat "$prefix/index1/ways_of_associativity"
printf "L1 line size: "
cat "$prefix/index1/coherency_line_size"
printf "L2 cache size: "
cat "$prefix/index2/size"
printf "L2 ways count: "
cat "$prefix/index2/ways_of_associativity"
printf "L2 line size: "
cat "$prefix/index2/coherency_line_size"
printf "L3 cache size: "
cat "$prefix/index3/size"
printf "L3 ways count: "
cat "$prefix/index3/ways_of_associativity"
printf "L3 line size: "
cat "$prefix/index3/coherency_line_size"