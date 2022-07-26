#!/bin/bash


#cd ./klsm
#make clean && make

sudo modprobe cpufreq_userspace

ITERS=10

FREQS=(1200 1500 2000 2500 2800 )  		#List of frequencies to run the test at

cd klsm

for freq in "${FREQS[@]}"; do
    
    sudo cpupower frequency-set --governor userspace
    sudo cpupower frequency-set --freq "$freq"000

    taskset -c 0-31 ./bench.py -a klsm128,klsm256 -p 1 -r $ITERS -o ./results/${freq}/results_1.csv
    taskset -c 0-31 ./bench.py -a klsm128,klsm256 -p 8 -r $ITERS -o ./results/${freq}/results_8.csv
    taskset -c 0-31 ./bench.py -a klsm128,klsm256 -p 16 -r $ITERS -o ./results/${freq}/results_16.csv
    taskset -c 0-31 ./bench.py -a klsm128,klsm256 -p 24 -r $ITERS -o ./results/${freq}/results_24.csv
    taskset -c 0-31 ./bench.py -a klsm128,klsm256 -p 32 -r $ITERS -o ./results/${freq}/results_32.csv

done
