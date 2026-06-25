# Networking Part B: UDP-Based Reliable-ish File Transfer

This folder contains a small networking application that demonstrates a simplified TCP-like transfer mechanism over UDP. The program splits a file into fixed-size chunks, assigns each chunk a sequence number, sends it to a receiver, and waits for acknowledgements.

## What the program does

The implementation in [networks/partB/code_partb.c](networks/partB/code_partb.c) provides:

- Chunking of a file into fixed-size data packets.
- Sequence numbers for each chunk.
- ACK packets from the receiver.
- Retransmission of unacknowledged chunks after a timeout.
- Reassembly of the received chunks into the final file.

## Packet structure

The code uses two simple packet formats:

- `data_packet`: stores the sequence number, total chunk count, and chunk data.
- `ack_packet`: stores the acknowledgement sequence number.

This mimics the basic idea of sequencing and acknowledgment used by TCP, but over UDP.

## How it works

1. The sender reads a file and divides it into `CHUNK_SIZE` blocks.
2. Each block is wrapped in a data packet and sent to the receiver.
3. The receiver stores arriving chunks by sequence number and sends ACKs back.
4. If an ACK is missing, the sender retransmits the corresponding chunk.
5. Once all chunks arrive, the receiver writes the reconstructed file to `received_data.txt`.

## Build and run

Compile the program:

```sh
gcc -Wall -g -o partb code_partb.c
```

### Receiver side

```sh
./partb server receive
```

### Sender side

```sh
./partb client send sample.txt 127.0.0.1
```

Replace `sample.txt` with the file you want to transfer.

## Notes

- The current implementation intentionally drops some ACKs every third packet to demonstrate retransmission behavior.
- The receiving side writes the reconstructed output to `received_data.txt` in the current working directory.
- This is a simplified educational implementation and is not a full TCP stack.