#!/bin/bash

mount ${1} /mnt/nucleo
cp ${2} /mnt/nucleo
umount /mnt/nucleo
