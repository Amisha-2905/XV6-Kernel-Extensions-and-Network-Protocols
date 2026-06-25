# Networking Part A: Tic-Tac-Toe over TCP and UDP

This folder contains a simple multiplayer Tic-Tac-Toe game implemented twice using different socket types:

- TCP version: [networks/partA/server_tcp.c](networks/partA/server_tcp.c) and [networks/partA/client_tcp.c](networks/partA/client_tcp.c)
- UDP version: [networks/partA/server_udp.c](networks/partA/server_udp.c) and [networks/partA/client_udp.c](networks/partA/client_udp.c)

## Overview

The server manages the shared game board, receives moves from the players, checks for winning conditions, and sends updated board status back to both clients. Each client displays the board and lets the user choose a move.

## How the game works

1. A client connects to the server and sends its player name.
2. The server waits for the second player.
3. Both players are assigned marks, `X` and `O`.
4. The server alternates turns and sends the current turn information.
5. Each player enters a row and column for their move.
6. The server validates the move, updates the board, and checks for a winner or draw.
7. The game can be restarted by both players if they agree to play again.

## TCP implementation

The TCP files use stream sockets (`SOCK_STREAM`) and rely on `connect`, `accept`, `read`, and `write` for communication. This version is easier to reason about because the connection is persistent between client and server.

## UDP implementation

The UDP files use datagram sockets (`SOCK_DGRAM`) and rely on `sendto` and `recvfrom`. The server and client exchange messages more explicitly, and the board state is sent as a small string representation of the board.

## Build and run

### TCP

```sh
gcc -Wall -g -o server_tcp server_tcp.c
gcc -Wall -g -o client_tcp client_tcp.c
```

Start the server:

```sh
./server_tcp
```

Start two clients in separate terminals:

```sh
./client_tcp <server_ip>
```

### UDP

```sh
gcc -Wall -g -o server_udp server_udp.c
gcc -Wall -g -o client_udp client_udp.c
```

Start the server:

```sh
./server_udp
```

Start two clients in separate terminals:

```sh
./client_udp <server_ip>
```

## Notes

- Both implementations are educational examples of basic networked game design.
- The server keeps the rules of the game, while the clients are responsible for collecting user input.
- The code is intentionally simple so that the networking concepts are easy to follow.
