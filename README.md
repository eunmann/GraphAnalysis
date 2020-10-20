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
To run the program, run:
```console
user@dir:~$ main.exe
```

Clean
---
___
To delete all built files, run:
```console
user@dir:~$ make clean
```
Note that in the makefile that it uses the windows command "del". This will need to be changed if running on a Unix machine.