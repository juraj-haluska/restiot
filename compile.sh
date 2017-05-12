#!/bin/bash

BOARD=nucleo_f767zi
TARGET=GCC_ARM

mbed compile -m ${BOARD} -t ${TARGET}
