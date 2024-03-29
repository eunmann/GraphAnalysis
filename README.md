Performance Analytics of Graph Algorithms using Intel Optane DC Persistent Memory
===

This project conducts a benchmark of memory and graph algorithms. It compares DRAM against persistent memory, PMEM.

Dependencies
---
___
Linux OS \
C++14 compliant compiler. (g++ 7.5.0 was used for this project) \
[PDMK](https://github.com/pmem/pmdk/) (libpmem 1.1) 

Build
---
___
To build the program:
```console
user@dir:~$ make
```

To build in debug mode:
```console
user@dir:~$ make debug
```

Make sure to clean when switching between release and debug

To build the assembly:
```console
user@dir:~$ make assembly
```

Run
---
___
To run the program, run:
```console
user@dir:~$ ./run.sh
```

The script run.sh contains the input parameters for the program. The machined used for experiementation required certain OMP environment variables to be set, please modify as required.

Output
---
___
The output of the program will be printed to the console as well as saved in a directory. A directory under ./output/ will be created based on the time the script was started. Then, all output will be saved to output.txt. The program will also generate csv files containing the metrics for each benchmark. The average results are reported. For the graph benchmarks, the units are TEPS in Millions. For the memory benchmarks, the units are specified in the file.

Clean
---
___
To delete all built files, run:
```console
user@dir:~$ make clean
```