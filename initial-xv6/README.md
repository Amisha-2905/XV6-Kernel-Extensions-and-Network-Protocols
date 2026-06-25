# XV6 Kernel Extensions and Scheduling

This folder contains the xv6 kernel modifications developed for the project. The implementation focuses on two main areas:

1. New system calls and user-space utilities.
2. Scheduler support for two alternative policies: lottery-based scheduling and MLFQ.

## Overview of the implementation

### 1. System calls

The xv6 kernel has been extended with the following user-facing features:

- `getsyscount`: counts how many times a selected system call is invoked by a process tree.
- `sigalarm` and `sigreturn`: support periodic user-level alarms that run a handler and then resume execution.
- `settickets`: lets a process change its number of lottery scheduler tickets.

These features are implemented through the syscall entry points in [initial-xv6/src/kernel/sysproc.c](initial-xv6/src/kernel/sysproc.c), the syscall dispatch table in [initial-xv6/src/kernel/syscall.c](initial-xv6/src/kernel/syscall.c), and the user programs in [initial-xv6/src/user](initial-xv6/src/user).

### 2. Scheduling policies

The project supports compilation-time selection of a scheduler using the `SCHEDULER` flag:

- `SCHEDULER=LBS` for lottery-based scheduling.
- `SCHEDULER=MLFQ` for a multi-level feedback queue scheduler.

The scheduler logic is implemented in [initial-xv6/src/kernel/proc.c](initial-xv6/src/kernel/proc.c), with related process state definitions in [initial-xv6/src/kernel/proc.h](initial-xv6/src/kernel/proc.h) and timer-related behavior in [initial-xv6/src/kernel/trap.c](initial-xv6/src/kernel/trap.c).

## Important source files

- [initial-xv6/src/kernel/sysproc.c](initial-xv6/src/kernel/sysproc.c): system-call implementations.
- [initial-xv6/src/kernel/syscall.c](initial-xv6/src/kernel/syscall.c): syscall registration and dispatch.
- [initial-xv6/src/kernel/proc.c](initial-xv6/src/kernel/proc.c): scheduler implementation.
- [initial-xv6/src/kernel/proc.h](initial-xv6/src/kernel/proc.h): process state and scheduler-related fields.
- [initial-xv6/src/user/syscount.c](initial-xv6/src/user/syscount.c): user-space syscall counting utility.
- [initial-xv6/src/user/alarmtest.c](initial-xv6/src/user/alarmtest.c): tests for alarm handling.
- [initial-xv6/src/user/schedulertest.c](initial-xv6/src/user/schedulertest.c): scheduler stress/performance testing.

## Build and run

From [initial-xv6/src](initial-xv6/src), use:

```sh
make clean
make qemu SCHEDULER=MLFQ
```

To test the system calls and scheduler behavior, run the relevant user programs from the xv6 shell, such as:

```sh
syscount 32768 grep hello README.md
alarmtest
schedulertest
```

## Notes

- The implementation is designed to be compiled with one scheduler at a time.
- The MLFQ version includes queue-based priority movement and boosting logic, while the lottery version uses ticket-based selection.
- A written report with analysis and comparison results should accompany the final submission.