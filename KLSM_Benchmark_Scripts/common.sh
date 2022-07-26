#!/bin/bash


sudo modprobe cpufreq_userspace
sudo modprobe msr
sudo chmod +rw /dev/cpu/*/msr