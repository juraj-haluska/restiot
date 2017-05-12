#!/bin/bash
DIR=".board"

mkdir $DIR
mount ${1} $DIR
cp ${2} $DIR
umount $DIR
rm -rf $DIR
