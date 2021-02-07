Persistent Memory Benchmark
===

This project conducts an experiment of a graph algorithm's performance on persistent memory machines.

Dependencies
---
___
Linux OS \
C++11 compliant compiler. (g++ 7.5.0 was used for this project) \
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

The script run.sh contains the input parameters for the program.

Output
---
___
The output of the program will be printed to the console as well as saved in a directory. A directory under ./output/ will be created based on the time the script was started. Then, all output will be saved to output.txt. Each test will save a csv of the metrics and append a command for gnuplot. From the root directory, running the generated script will produce pngs for each csv file in the output directory using GNUPlot.

Clean
---
___
To delete all built files, run:
```console
user@dir:~$ make clean
```