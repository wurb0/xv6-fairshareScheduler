# Fair-Share Scheduler for xv6 (RISC-V)

This project implements a fair-share CPU scheduler for the xv6 operating system. Instead of round-robin scheduling, processes receive CPU time proportional to a configurable share (weight), ensuring long-term fairness while preventing starvation.

This work was completed as part of CMPT 332 – Operating Systems and focuses on kernel-level scheduling, synchronization, and CPU accounting.

---

## Overview

Each runnable process tracks:
- cpu_used: total CPU ticks consumed
- share: weight representing how much CPU time the process should receive

At each scheduling decision, the scheduler selects the runnable process with the lowest score:

score = cpu_used / share

This ensures that processes with higher shares receive more CPU time over time, while CPU-deprived processes naturally catch up.

---

## Key Features

- Fair-share scheduling based on historical CPU usage
- Multi-core safe runnable queue
- Per-process CPU accounting using kernel ticks
- Kernel-level setshare() system call
- Starvation-free scheduling
- User-space test program to validate fairness

---

## Files Included

This repository contains only the modified and relevant files:

kernel/
  proc.c        Scheduler implementation and runnable queue
  proc.h        Process structure extensions

user/
  scheduler_test.c   User-space scheduler test

README.md

The full xv6 kernel is not included.

---

## Kernel Changes

### Process Structure (proc.h)

The following fields were added to struct proc:

- cpu_used: total CPU ticks consumed
- share: CPU share weight (minimum 1)

These fields are used by the scheduler to compute fairness.

---

## Runnable Queue

- A circular runnable queue is maintained separately from the process table
- Protected by runnables_lock
- Scheduler decisions additionally protected by sched_lock
- Designed to be safe under multi-core execution

---

## Scheduler Logic

High-level scheduling flow:
1. Scan runnable queue
2. Compute cpu_used / share for each RUNNABLE process
3. Select the process with the lowest score
4. Context switch and measure execution time
5. Update cpu_used
6. Reinsert the process if still runnable

If no runnable processes exist, the CPU enters a wait-for-interrupt (wfi) state.

---

## System Call: setshare(int share)

A custom system call allows user processes to dynamically adjust their CPU share.

- Minimum share enforced (share >= 1)
- Protected by the process lock
- Takes effect immediately

This syscall must be registered through the standard xv6 syscall path:
sysproc.c, syscall.c, syscall.h, user.h, and usys.pl.

---

## Testing the Scheduler

The scheduler_test program:
- Forks multiple CPU-bound child processes
- Runs identical workloads
- Demonstrates proportional CPU allocation via output frequency

Expected behavior:
- Processes with higher shares make more visible progress
- All processes continue executing (no starvation)

---

## Build and Run

1. Apply these changes to an xv6 RISC-V tree
2. Add scheduler_test.c to UPROGS in the Makefile
3. Build and run xv6:
   make qemu
4. Run the test:
   scheduler_test

---

## Notes

- CPU usage is measured using kernel ticks during context switches
- cpu_used is periodically scaled down to prevent overflow
- Priority fields are present but unused in the final design
- Emphasis was placed on correctness and clarity over micro-optimizations

---

## Concepts Demonstrated

- Operating system scheduling policies
- Kernel synchronization with spinlocks
- CPU accounting
- Multi-core scheduling
- System call implementation
- Low-level C systems programming

---

## Author

Mustafa Saqib  
B.Sc. Computer Science  
University of Saskatchewan
