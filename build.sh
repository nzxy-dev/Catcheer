#!/bin/bash

set -e

printf "\033[48:2:255:165:0m%s\033[m\n" "=== Catcheer Linux Build ==="

mkdir -p build
cd build

rm -f CMakeCache.txt

# generate build files with gcc

printf "\033[48:2:255:165:0m%s\033[m\n" "=== genrating build files con GCC ==="
cmake -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ ..

# Compilar
printf "\033[48:2:255:165:0m%s\033[m\n" "=== compile Catcheer ==="
cmake --build .

printf "\033[48:2:255:165:0m%s\033[m\n" "=== compile completed  ==="
