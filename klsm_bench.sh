#!/bin/bash


#cd ./klsm
#make clean && make

sudo modprobe cpufreq_userspace
sudo modprobe msr
sudo chmod +rw /dev/cpu/*/msr

ITERS=5

FREQS=( 1200000 1500000 1800000 2100000 2300000 )  	#List of frequencies to run the test at
CORES=( 1 8 16 24 )
cd klsm

for freq in "${FREQS[@]}"; do
    echo "TESTING FREQUENCY $freq"
 
    # sudo cpupower frequency-set --governor userspace
    # sudo cpupower frequency-set --freq 
    for core in "${CORES[@]"; do
    	echo "TESTING CORE $core"
	../helper.sh $freq $ITERS $core 20
    done
done
