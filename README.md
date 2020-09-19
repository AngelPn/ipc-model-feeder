# IPC model
IPC architecture that includs a feeder (producer) process, a number of consumer processes (their number is given by the command line) and a shared memory sector accessible to both feeder and consumers.

The feeder holds a matrix of random integers of magnitude M (given by the command line) and uses the shared memory domain to replicate that table with exactly the same composition in n consumer processes. The feeder writes to memory from one integer at a time along with its time stamp and then waits until this integer is read from all consumer processes to reactivate and write a new value.

Consumers use the shared memory domain to read the integer that is registered by the feeder and calculate the moving average time delay each time they derive information from the shared memory.

Shared memory can store up to an integer and a time stamp. This time stamp is set at the time the feeder registers a value from its table in the shared memory domain. In doing so, consumers are asked one by one to read the shared memory information by storing the integer and counting each time the new average delay time.

The above procedure is repeated until the feeder process table is reproduced in the same order in all consumer processes, and the last completed user process prints its table and the average delay time to an output file that is common to all processes.

# Compile and Execution
```sh
$ make
```
```sh
$ ./ex <number of values> <number of processes>
```
number of values must be >3000
