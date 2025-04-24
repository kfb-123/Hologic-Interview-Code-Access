# Disk-Scheduler-and-Thread-Library
This includes both a simple concurrent program that schedules disk requests, and a thread library.


## Project Overview

This project is composed of two parts:

### 1. Disk Scheduler (`disk.cc`)
Implements a multi-threaded disk scheduler that simulates handling disk I/O requests using requester and service threads. The scheduler:
- Spawns one requester thread per input file, each submitting a series of disk track requests.
- Services requests using **SSTF (Shortest Seek Time First)** scheduling.
- Enforces a maximum queue size (`max_disk_queue`), blocking requesters when full.
- Synchronizes threads using monitors to avoid race conditions.
- Ensures output is properly serialized using a shared lock.

**Run Instructions:**
g++ disk.cc thread.o libinterrupt.a -ldl -o disk
./disk <max_disk_queue> disk.in0 disk.in1 ...
2. Thread Library (thread.cc)
Implements a user-level thread library using ucontext that supports:

Thread creation and cooperative scheduling (thread_create, thread_yield)

Mesa-style monitor synchronization (thread_lock, thread_unlock, thread_wait, thread_signal, thread_broadcast)

Interrupt disabling/enabling for atomic operations using the provided libinterrupt.a

Robust error handling and assertion-based debugging

The library ensures FIFO scheduling across all queues (ready, lock, condition), and handles corner cases such as lock misuse and improper initialization.

