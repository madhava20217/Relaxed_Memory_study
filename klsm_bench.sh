#!/bin/bash

cd ./klsm
make clean && make && ./bench.py -a klsm128,klsm256 -p 1,2,3,5,10,15,20 -r 5 -o results.csv
Rscript misc/plot_throughput.R results.csv