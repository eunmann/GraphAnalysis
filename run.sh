#!/bin/bash

dir_name=$(date "+%Y.%m.%d.%H.%M.%S")
file_name=output.txt
out_dir=output/$dir_name/
final_name=$out_dir$file_name
mkdir $out_dir

# Output directory
export out_dir

# Size of the memory test buffer
export alloc_size=1000000000

# Number of vertices to generate for the graph test
export num_vertices=1000000

# Minimum and Maximum degree for vertice
export min_degree=20
export max_degree=200

# Minimum and Maximum value for an edge's weight
export min_value=1
export max_value=2

# Page Rank Parameters
export page_rank_iterations=100
export page_rank_dampening_factor=0.8

# Number of iterations for each test
export test_iterations=10


echo Starting graph_analysis
echo Output Directory: $out_dir
echo Output File: $final_name

./graph_analysis | tee $final_name

echo Done
