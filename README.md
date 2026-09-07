# OS Lab Assignment 5 – Process Synchronization using xv6

## Overview

In this assignment, we implemented different process synchronization problems
in xv6. We worked on Peterson's solution, Producer-Consumer, Readers-Writers,
and Dining Philosophers.

For communication between processes, we used shared memory. We also implemented
our own simple semaphore system using xv6's sleep and wakeup mechanisms to
control access to shared resources.

## Design

### Q1 – Peterson's Solution
We used Peterson's algorithm to make sure that the parent and child processes
do not enter the critical section at the same time. They use shared flags and
a turn variable, along with a shared counter.

### Q2 – Producer-Consumer
The producer and consumer share a circular buffer. We used three semaphores:
`empty` for available spaces, `full` for available items, and `mutex` to make
sure only one process accesses the buffer at a time.

### Q3 – Readers-Writers
Multiple readers are allowed to read the shared data at the same time, while
writers get exclusive access. We used semaphores to protect the reader count
and shared resource. A service queue was also used to make the solution fair
to both readers and writers.

### Q4 – Dining Philosophers
Five philosophers share five forks, with each fork represented using a
semaphore. To avoid deadlock, we made every philosopher pick up the
lower-numbered fork first and then the higher-numbered fork.

## How to Build and Run

First, go to the xv6 directory and run:

    make clean
    make
    make qemu

Once xv6 starts, the programs can be run from the xv6 shell:

    peterson
    prodcons
    readwrite
    dining

The programs print their execution details in the terminal, and the output
logs are also saved in the `output_logs` folder.

## Files

The `kernel` folder contains the shared memory and semaphore changes, while
the `user` folder contains the four synchronization programs. The Makefile
contains the required build changes, and `output_logs` contains the output
from running each program.
