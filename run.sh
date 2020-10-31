#!/bin/sh

file_name=output
current_time=$(date "+%Y.%m.%d-%H:%M:%S")
file_ext=.txt
final_name=$file_name.$current_time$file_ext

alloc_size=1000000000

echo Starting main
echo Allocation Size: $alloc_size
echo Output File: $final_name

./main $alloc_size > $final_name

echo Done
