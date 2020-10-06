Graph Analysis
===

This project conducts an experiment of a graph algorithm's performance on persistent memory machines.

Build
---
___
To build the program, you will need a g++ 9.2.0 and make.
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
To clean the directories of files from building, run:
```console
user@dir:~$ make clean
```
Note that in the makefile that it uses the windows command "del". This will need to be changed if running on a Unix machine.