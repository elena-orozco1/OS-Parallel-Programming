# C++ Parallel Programming Assignments

C++ programming assignments covering multithreading, thread synchronization, and client-server socket communication with Elias Gamma/Delta encoding analysis.

## Repository Structure

- `pa1-multithreading/` — one-thread-per-symbol local processing
- `pa2-client-server/` — distributed client/server processing over TCP sockets
- `pa3-thread-synchronization/` — multithreading with synchronization for ordered output

## Quick Start

### PA1

```bash
g++ -std=c++17 -pthread pa1-multithreading/src/multithreading.cpp -o multithreading
./pa1 < pa2-client-server/sample/input.txt
```

### PA2

```bash
g++ -std=c++17 pa2-client-server/src/server.cpp -o server
g++ -std=c++17 -pthread pa2-client-server/src/client.cpp -o client
./server 5000
./client localhost 5000 < pa2-client-server/sample/input.txt
```

### PA3

```bash
g++ -std=c++17 -pthread pa3-thread-synchronization/src/thread_synchronization.cpp -o thread_synchronization
./pa3 < pa2-client-server/sample/input.txt
```
# OS-Parallel-Programming
