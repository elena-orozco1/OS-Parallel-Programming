# PA1: Multithreading

This assignment processes an input message using one thread per symbol.

## What it does

- Removes previously-seen symbols for each thread's view
- Computes symbol positions and positional differences
- Compares Elias Gamma and Elias Delta bit counts

## Build

```bash
g++ -std=c++17 -pthread src/multithreading.cpp -o pa1
```

## Run

```bash
./pa1 < ../pa2-client-server/sample/input.txt
```
