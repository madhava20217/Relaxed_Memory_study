#!/bin/bash

cd ./Energy_metrics

gcc -o cpu_monitoring pmc_monitoring.c
sudo ./cpu_monitoring