#!/bin/bash


#cd ./klsm
#make clean && make

sudo modprobe cpufreq_userspace

ITERS=5

FREQS=( 1200000 1500000 2000000 2500000 2800000 )  		#List of frequencies to run the test at

cd klsm

for freq in "${FREQS[@]}"; do
    echo "TESTING FREQUENCY $freq"   
 
    sudo cpupower frequency-set --governor userspace
    sudo cpupower frequency-set --freq "$freq"

    taskset -c 0-31 ./bench.py -a klsm128,klsm256 -p 1 -r $ITERS -o ./results/${freq}/results_1.csv
    taskset -c 0-31 ./bench.py -a klsm128,klsm256 -p 8 -r $ITERS -o ./results/${freq}/results_8.csv
    taskset -c 0-31 ./bench.py -a klsm128,klsm256 -p 16 -r $ITERS -o ./results/${freq}/results_16.csv
    taskset -c 0-31 ./bench.py -a klsm128,klsm256 -p 24 -r $ITERS -o ./results/${freq}/results_24.csv
    taskset -c 0-31 ./bench.py -a klsm128,klsm256 -p 32 -r $ITERS -o ./results/${freq}/results_32.csv

done
