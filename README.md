# xv6 Fair-Share Scheduler 

This repo shows my xv6-riscv fair-share scheduler work from an Operating Systems course.
Scheduler goal: pick the RUNNABLE process with the smallest score = cpu_used / share so that
processes with higher share get more CPU over time and starved processes catch up.

What’s included here:
- proc.c (my modified version)
- README (setup + what to change)

Note: I’m not publishing the full xv6 tree for academic integrity reasons. This README explains
exactly what to change in a clean xv6 checkout.

------------------------------------------------------------
1) Build & Run (xv6-riscv)
------------------------------------------------------------

Prereqs (typical):
- RISC-V toolchain (riscv64-unknown-elf-*)
- qemu-system-riscv64
- make

Steps:
1. Get xv6-riscv from your course or the official repo (same version I used).
2. Replace the kernel/proc.c with the proc.c in this repo.
3. Build and run:
   make clean
   make qemu

If your setup uses a different folder layout, replace the proc.c in the same file location
that contains the kernel scheduler() and process table code.

------------------------------------------------------------
2) Design Summary
------------------------------------------------------------

Each process tracks:
- p->cpu_used : total CPU ticks consumed
- p->share    : weight controlling how much CPU it should get

Scheduling rule:
- for each RUNNABLE process, compute score = cpu_used / share
- run the process with the smallest score

CPU accounting:
- measure ticks before and after swtch() and add the delta to cpu_used
- apply decay when cpu_used gets huge (prevents overflow / keeps ratios meaningful)

Concurrency:
- runnables_lock protects the runnable queue (circular array)
- p->lock protects per-process state transitions
- sched_lock prevents multi-core scheduling races while selecting a “best” process

Idle behavior:
- if no runnable processes exist, CPU executes wfi (wait for interrupt)

------------------------------------------------------------
3) Files / Changes Required (beyond proc.c)
------------------------------------------------------------

My proc.c references new fields + a syscall-style setter for share. To make this compile/work,
you need a few small changes in the standard xv6 files below.

A) kernel/proc.h  (struct proc fields)
Add fields to struct proc:
- int share;
- uint64 cpu_used;

(You also have p->priority in your proc.c; if you kept it, ensure proc.h includes it too.)

B) Add a way to set share from user space (recommended)
You already wrote:
  int setshare(int share)

Expose it as a syscall so you can test from user programs.

Typical syscall wiring in xv6:

1) kernel/sysproc.c
Add a syscall handler:
  uint64
  sys_setshare(void)
  {
    int share;
    if(argint(0, &share) < 0) return -1;
    return setshare(share);
  }

2) kernel/syscall.h
Add a syscall number:
  #define SYS_setshare  <next available number>

3) kernel/syscall.c
Add the function to the syscall table:
  extern uint64 sys_setshare(void);
  [SYS_setshare]  sys_setshare,

4) user/user.h
Add user-space prototype:
  int setshare(int);

5) user/usys.pl
Add a stub:
  entry("setshare");

C) (Optional but useful) A user test program
Create something like user/sharetest.c that:
- forks a few CPU-bound loops
- calls setshare(1), setshare(2), setshare(8) per process
- prints runtime / progress so you can observe fairness

Then add it to the build:
- user/Makefile: add sharetest to UPROGS

------------------------------------------------------------
4) How to Test Quickly
------------------------------------------------------------

Inside xv6 shell:
- Run your test program (sharetest)
- Or run multiple CPU-bound programs in parallel and compare how fast they progress
- Increase one process’s share and confirm it gets more CPU time over the run

------------------------------------------------------------
5) Known Notes / Assumptions
------------------------------------------------------------

- share must be >= 1 (I clamp it).
- score uses integer division (cpu_used/share). That’s fine for fairness, but you can
  also use scaled math if you want smoother behavior (e.g., cpu_used * K / share).
- runnable queue is a circular array; locks matter on multicore.
