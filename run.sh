#!/bin/sh

file_name=output
current_time=$(date "+%Y.%m.%d.%H.%M.%S")
file_ext=.txt
final_name=$file_name.$current_time$file_ext

# Number of GB to allocate for the memory test buffer
alloc_size=20

# Number of vertices to generate for the graph (in Millions)
num_vertices=100

echo Starting graph_analysis
echo Allocation Size: $alloc_size GB
echo Number of Vertices: $num_vertices M
echo Output File: $final_name

./graph_analysis $alloc_size $num_vertices > $final_name

echo Done
