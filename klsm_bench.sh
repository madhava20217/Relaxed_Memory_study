#!/bin/bash


#cd ./klsm
#make clean && make

sudo modprobe cpufreq_userspace
sudo modprobe msr
sudo chmod +rw /dev/cpu/*/msr

ITERS=5

FREQS=( 1200000 1500000 2000000 2500000 2800000 )  	#List of frequencies to run the test at

cd klsm

for freq in "${FREQS[@]}"; do
    echo "TESTING FREQUENCY $freq"
 
    # sudo cpupower frequency-set --governor userspace
    # sudo cpupower frequency-set --freq "$freq"

    ../helper.sh $freq $ITERS 1 20
    ../helper.sh $freq $ITERS 8 20
    ../helper.sh $freq $ITERS 16 20
    ../helper.sh $freq $ITERS 24 20
    ../helper.sh $freq $ITERS 32 20
done
