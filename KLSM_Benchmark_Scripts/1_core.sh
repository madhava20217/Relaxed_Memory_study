#!/bin/bash

./common.sh



ITERS=5

FREQS=( 1200000 1500000 2000000 2500000 2800000 )  		#List of frequencies to run the test at

cd ..
cd klsm

for freq in "${FREQS[@]}"; do
    echo "TESTING FREQUENCY $freq"   c
 
    sudo cpupower frequency-set --governor userspace
    sudo cpupower frequency-set --freq "$freq"

    taskset -c 0 ./bench.py -a klsm128,klsm256 -p 1 -r $ITERS -o ./results/${freq}/results_1.csv & sudo taskset -c 32 ../Energy_metrics/cpu_monitoring ./results/${freq}/power_1.csv 

done