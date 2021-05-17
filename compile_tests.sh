#!/bin/bash
CC=gcc
CFLAGS="-fsanitize=address -Wall -Werror -std=gnu11"

inputs=(testcases/src/*)
for infile in "${inputs[@]}";
do
    # set outfile name
    outfile=${infile/src/bin}
    outfile=${outfile/.c}
    echo $outfile

    $CC $CFLAGS ./$infile -o $outfile
done
