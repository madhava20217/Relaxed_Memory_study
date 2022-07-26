#!/bin/bash


#cd ./klsm
#make clean && make

sudo modprobe cpufreq_userspace
sudo modprobe msr
sudo chmod +rw /dev/cpu/*/msr

ITERS=5

FREQS=( 1200000 1500000 2000000 2500000 2800000 )  		#List of frequencies to run the test at

cd klsm

for freq in "${FREQS[@]}"; do
    echo "TESTING FREQUENCY $freq"   c
 
    sudo cpupower frequency-set --governor userspace
    sudo cpupower frequency-set --freq "$freq"



    # (echo taskset -c 0-31 ./bench.py -a klsm128,klsm256 -p 1 -r $ITERS -o ./results/${freq}/results_1.csv; echo taskset -c 32 ../Energy_metrics/cpu_monitoring > ./results/${freq}/power_1.csv) | parallel
    # (echo taskset -c 0-31 ./bench.py -a klsm128,klsm256 -p 8 -r $ITERS -o ./results/${freq}/results_8.csv; echo taskset -c 32 ../Energy_metrics/cpu_monitoring > ./results/${freq}/power_8.csv) | parallel
    # (echo taskset -c 0-31 ./bench.py -a klsm128,klsm256 -p 16 -r $ITERS -o ./results/${freq}/results_16.csv; echo taskset -c 32 ../Energy_metrics/cpu_monitoring > ./results/${freq}/power_16.csv) | parallel
    # (echo taskset -c 0-31 ./bench.py -a klsm128,klsm256 -p 24 -r $ITERS -o ./results/${freq}/results_24.csv; echo taskset -c 32 ../Energy_metrics/cpu_monitoring > ./results/${freq}/power_24.csv) | parallel
    # (echo taskset -c 0-31 ./bench.py -a klsm128,klsm256 -p 32 -r $ITERS -o ./results/${freq}/results_32.csv; echo taskset -c 32 ../Energy_metrics/cpu_monitoring > ./results/${freq}/power_32.csv) | parallel
    pids = ""
    taskset -c 0-31 ./bench.py -a klsm128,klsm256 -p 1 -r $ITERS -o ./results/${freq}/results_1.csv & 
        pids="$pids $!" & 
    sudo taskset -c 32 ../Energy_metrics/cpu_monitoring ./results/${freq}/power_1.csv 7 & 
        pids="$pids $!";
        
    wait $pids;
    pids = ""


    taskset -c 0-31 ./bench.py -a klsm128,klsm256 -p 1 -r $ITERS -o ./results/${freq}/results_8.csv & 
        pids="$pids $!" & 
    sudo taskset -c 32 ../Energy_metrics/cpu_monitoring ./results/${freq}/power_8.csv 7
        pids="$pids $!";

    wait $pids;
    pids = ""


    taskset -c 0-31 ./bench.py -a klsm128,klsm256 -p 1 -r $ITERS -o ./results/${freq}/results_16.csv & 
        pids="$pids $!" & 
    sudo taskset -c 32 ../Energy_metrics/cpu_monitoring ./results/${freq}/power_16.csv 7
        pids="$pids $!";

    wait $pids;
    pids = ""


    taskset -c 0-31 ./bench.py -a klsm128,klsm256 -p 1 -r $ITERS -o ./results/${freq}/results_24.csv & 
        pids="$pids $!" & 
    sudo taskset -c 32 ../Energy_metrics/cpu_monitoring ./results/${freq}/power_24.csv 7
        pids="$pids $!";

    wait $pids;
    pids = ""


    taskset -c 0-31 ./bench.py -a klsm128,klsm256 -p 1 -r $ITERS -o ./results/${freq}/results_32.csv & 
        pids="$pids $!" & 
    sudo taskset -c 32 ../Energy_metrics/cpu_monitoring ./results/${freq}/power_32.csv 7
        pids="$pids $!";

    wait $pids;
    pids = ""
done
