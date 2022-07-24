#!/bin/bash

cd ./klsm
make clean && make
taskset -c 0-31 ./bench.py -a klsm128,klsm256 -p 1,8,16,24,32 -r 5 -o results.csv
Rscript misc/plot_throughput.R results.csv
