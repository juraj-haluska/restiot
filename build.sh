#!/bin/bash

BOARD=nucleo_f767zi
TARGET=GCC_ARM

mbed compile -m ${BOARD} -t ${TARGET} -DMBED_HEAP_STATS_ENABLED=1
