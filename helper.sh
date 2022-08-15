#!/bin/bash

# Takes four arguments
# 1) Frequency
# 2) Iterations
# 3) Cores
# 4) Sampling

FREQ=$1                             #testing_frequency
ITER=$2                             #iterations to run the test for
CORE=$3                             #total number of cores
MONITOR_CORE=$CORE                  #core to monitor power at
SAMPLES=$4
CORE=$(($CORE-1))

echo $FREQ $ITER $CORE $MONITOR_CORE

#clear csv file
truncate -s 0 ./results/${FREQ}/results_${MONITOR_CORE}.csv


#setting frequency and changing governor
sudo cpupower frequency-set --governor userspace
sudo cpupower frequency-set --freq "$FREQ"

#executing task, setting seed as 1520043 (after -k argument)
taskset -c 0-${CORE} ./bench.py -a spray -p ${MONITOR_CORE} -r ${ITER} -o ./results/${FREQ}/results_${MONITOR_CORE}.csv & sudo taskset -c ${MONITOR_CORE} ../Energy_metrics/cpu_monitoring ./results/${FREQ}/power_${MONITOR_CORE}.csv ${SAMPLES}


wait