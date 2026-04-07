# PA2: Client-Server (Sockets)

This assignment splits computation between a multithreaded client and a fork-based server.

## What it does

- Client creates one thread per symbol
- Each thread sends request data to server via TCP
- Server computes filtered message, positions/differences, and Elias bit counts
- Client aggregates and prints per-symbol output plus totals

## Build

```bash
g++ -std=c++17 src/server.cpp -o server
g++ -std=c++17 -pthread src/client.cpp -o client
```

## Run

In terminal 1:

```bash
./server 5000
```

In terminal 2:

```bash
./client localhost 5000 < sample/input.txt
```
