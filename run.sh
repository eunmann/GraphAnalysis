#!/bin/bash

file_name=output
current_time=$(date "+%Y.%m.%d.%H.%M.%S")
file_ext=.txt
final_name=$file_name.$current_time$file_ext

# Size of the memory test buffer
export alloc_size=1000000000

# Number of vertices to generate for the graph test
export num_vertices=1000000

echo Starting graph_analysis
echo Output File: $final_name

./graph_analysis | tee $final_name

echo Done
