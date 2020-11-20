#!/bin/sh

file_name=output
current_time=$(date "+%Y.%m.%d.%H.%M.%S")
file_ext=.txt
final_name=$file_name.$current_time$file_ext

alloc_size=20

echo Starting graph_analysis
echo Allocation Size: $alloc_size GB
echo Output File: $final_name

./graph_analysis $alloc_size > $final_name

echo Done
