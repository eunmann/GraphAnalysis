Graph Analysis
===

This project conducts an experiment of a graph algorithm's performance on persistent memory machines.

Dependencies
---
___
C++17 compliant compiler. (g++ 9.2.0 was used for this project) \
[PDMK](https://github.com/pmem/pmdk/) \
Make sure to modify the include and link paths in the makefile for your system.

Build
---
___
To build the program:
The command to build is:
```console
user@dir:~$ make
```

<span style="color:red">Windows does not compile at the moment! There are linking errors.<span>


To build in debug mode:
```console
user@dir:~$ make debug
```

Make sure to clean when switching from release to debug builds

To build the assembly:
```console
user@dir:~$ make assembly
```

Run
---
___
To run the program linux, run:
```console
user@dir:~$ ./main
```

To run the program Windows, run:
```console
user@dir:~$ ./main.exe
```

Clean
---
___
To delete all built files, run:
```console
user@dir:~$ make clean
```