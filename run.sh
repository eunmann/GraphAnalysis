#!/bin/bash


dir_name=$(date "+%Y.%m.%d.%H.%M.%S")
file_name=output.txt
out_dir=output/${dir_name}/
final_name=${out_dir}${file_name}
mkdir ${out_dir}

# Output directory
export out_dir

# Size of the memory test buffer
export mem_alloc_size=10000000000

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
export page_rank_num_dampening_factors=8

# Number of iterations for each test
export test_iterations=10

# OMP environment variables
export OMP_DISPLAY_ENV=true
export OMP_PROC_BIND=true

# Allocates threads to hardware threads (using hyperthreading)
#export OMP_PLACES={0}:36
#export OMP_NUM_THREADS=36

# Allocates threads to cores
export OMP_PLACES={0}:18:2
export OMP_NUM_THREADS=18

echo Starting pmem_benchmark
echo Output Directory: ${out_dir}
echo Output File: ${final_name}

#./pmem_benchmark
./pmem_benchmark | tee $final_name

echo Done
