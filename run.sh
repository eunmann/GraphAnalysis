#!/bin/bash

dir_name=$(date "+%Y.%m.%d.%H.%M.%S")
file_name=output.txt
out_dir=output/${dir_name}/
final_name=${out_dir}${file_name}
mkdir ${out_dir}

# Output directory
export out_dir

# Size of the memory test buffer
export mem_alloc_size=1000000000

# Number of vertices to generate for the graph test
export graph_num_vertices=100000

# Minimum and Maximum degree for vertice
export graph_min_degree=20
export graph_max_degree=200

# Minimum and Maximum value for an edge's weight
export graph_min_value=1
export graph_max_value=2

# Page Rank Parameters
export page_rank_iterations=100
export page_rank_num_dampening_factors=16

# Number of iterations for each test
export test_iterations=3

# OMP environment variables
export OMP_DISPLAY_ENV=true
export OMP_PROC_BIND=true

if [ $HOSTNAME = "Evan-Ubuntu" ]; then
	export OMP_PLACES="{0}:24:1"
	export OMP_NUM_THREADS=24
else
	export OMP_PLACES="{0}:36:1"
	export OMP_NUM_THREADS=36
fi

echo Starting pmem_benchmark
echo Output Directory: ${out_dir}
echo Output File: ${final_name}

./pmem_benchmark | tee $final_name

echo Done
