#!/bin/bash

packages=("cmake" "libgsl-dev" "libhwloc-dev" "valgrind" 
           "doxygen" "libatomic-ops-dev", "linux-tools-common")

for pkg in ${packages[@]}; do
    sudo apt install $pkg
done
