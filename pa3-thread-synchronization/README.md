# PA3: Thread Synchronization

This assignment extends multithreading with synchronization primitives to coordinate thread output order.

## What it does

- Uses `pthread_mutex_t` and `pthread_cond_t`
- Enforces deterministic printing order by thread ID
- Computes same Elias Gamma/Delta metrics as PA1

## Build

```bash
g++ -std=c++17 -pthread src/thread_synchronization.cpp -o thread_synchronization
```

## Run

```bash
./thread_synchronization < ../pa2-client-server/sample/input.txt
```
