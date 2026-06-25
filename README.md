# OSN Mini Project 2: XV6 and Networking

This repository contains the implementation for the second mini-project of the OSN course. The work is split into two major parts:

- XV6 kernel extensions and scheduler changes in the [initial-xv6/src](initial-xv6/src) directory.
- Networking programs in the [networks](networks) directory.

## Project structure

- [initial-xv6/src](initial-xv6/src): xv6 source tree with system-call additions, scheduler support, and user-space test programs.
- [networks/partA](networks/partA): TCP and UDP implementations of a two-player Tic-Tac-Toe game.
- [networks/partB](networks/partB): UDP-based file transfer with chunking, sequence numbers, ACKs, and retransmission.

## What is included

1. XV6 enhancements
   - Added system calls for syscall counting and alarm-style handling.
   - Implemented scheduler options for lottery-based scheduling and MLFQ.
   - Added user-space utilities for testing the new behavior.

2. Networking applications
   - A TCP-based Tic-Tac-Toe server and client.
   - A UDP-based Tic-Tac-Toe server and client.
   - A UDP file-transfer program that splits data into chunks and retransmits missing ones.

## Quick start

### XV6

Navigate to [initial-xv6/src](initial-xv6/src) and build the kernel with a chosen scheduler:

```sh
make clean
make qemu SCHEDULER=MLFQ
```

You can also build with `SCHEDULER=LBS`.

### Networking part A

Go to [networks/partA](networks/partA) and compile the required programs manually:

```sh
gcc -Wall -g -o server_tcp server_tcp.c
gcc -Wall -g -o client_tcp client_tcp.c
gcc -Wall -g -o server_udp server_udp.c
gcc -Wall -g -o client_udp client_udp.c
```

### Networking part B

Compile the file-transfer program:

```sh
gcc -Wall -g -o partb code_partb.c
```

## Notes

- The xv6 component is documented in more detail in [initial-xv6/README.md](initial-xv6/README.md).
- The TCP/UDP game implementation is documented in [networks/partA/README.md](networks/partA/README.md).
- The UDP file-transfer implementation is documented in [networks/partB/README.md](networks/partB/README.md).
- A detailed report describing the implementation and results should be added alongside these files when preparing the final submission.
