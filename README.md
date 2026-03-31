# Cache Simulator

Cache Simulator is a computer architecture project for exploring how different cache organizations behave under different memory-access patterns. It combines a C++ simulation backend with a lightweight browser UI, making it easy to run experiments and inspect the resulting metrics.

The project is designed to help answer questions such as:

- How do direct-mapped, fully associative, and 2-way set-associative caches differ in practice?
- How do access patterns affect hit rate, miss rate, and replacements?
- How do cache outcomes translate into a simple timing estimate?

## Overview

The simulator reports two categories of results:

- Cache behavior: hits, misses, replacements, hit rate, and miss rate
- Timing estimation: total cycles, average memory access time, and estimated execution time

This is not a full processor or pipeline simulator. The timing model is intentionally simple and is intended to provide a practical approximation rather than cycle-accurate hardware analysis.

## Features

- Simulates `DM`, `FA`, and `2W` cache organizations
- Supports `LRU`, `FIFO`, and `Random` replacement policies
- Includes multiple built-in memory-access patterns
- Exposes simulation results through a local HTTP API
- Provides a browser-based interface for interactive experimentation
- Estimates execution cost using configurable timing parameters

## Supported Configurations

### Cache Types

- `DM` — Direct Mapped
- `FA` — Fully Associative
- `2W` — 2-Way Set Associative

### Replacement Policies

- `LRU`
- `FIFO`
- `Random`

Replacement policy only affects associative caches. In direct-mapped mode, placement is determined entirely by the address mapping.

### Access Patterns

- `Random` — random accesses across the simulated memory space
- `Sequential` — sequential traversal through memory
- `FourStrike` — repeated accesses to a very small set of blocks
- `ConflictDM` — a conflict-heavy pattern that stresses direct-mapped caches
- `WorkingSet64` — repeated accesses within a limited working set
- `StrideConflict` — strided accesses intended to trigger repeated set conflicts

## How the Simulation Works

Each run generates a sequence of memory accesses and applies them to the selected cache model. For every access, the simulator determines whether it is:

- A `hit`: the required block is already in the cache
- A `miss`: the block is not present and must be brought into the cache
- A `replacement`: an existing valid line is evicted to make room for a new block

From these outcomes, the simulator computes summary statistics and timing estimates.

Important distinction:

- Every replacement happens on a miss
- Not every miss causes a replacement, because the cache may still contain invalid or empty lines

## Timing Model

The project includes a simple timing model based on three user-controlled inputs:

- `Hit Latency (cycles)` — base cost applied to every access
- `Miss Penalty (cycles)` — additional cost applied when an access misses
- `CPU Frequency (GHz)` — used to convert cycle count into nanoseconds

The formulas are:

```text
totalCycles = accesses * hitLatency + misses * missPenalty
AMAT (Average Memory Access Time) = totalCycles / accesses
estimatedTimeNs = totalCycles / cpuFrequencyGHz
```

Clarification:

- Replacements are tracked and reported as a cache metric
- Replacements do not currently add a separate cycle cost
- Their timing impact is only reflected indirectly when they occur as part of a miss

## Metrics Reported

The UI and API expose the following values:

- `hits`
- `misses`
- `hitRate`
- `replacements`
- `hitLatency`
- `missPenalty`
- `cpuFrequencyGHz`
- `totalCycles`
- `amat`
- `estimatedTimeNs`

## Project Structure

```text
cache-simulator/
├── backend/
│   ├── api.cpp
│   ├── main.cpp
│   ├── simulator.cpp
│   ├── simulator.hpp
│   ├── httplib.h
│   ├── Makefile
│   └── run_tests.sh
├── web/
│   ├── index.html
│   ├── style.css
│   ├── app.js
│   └── screenshots/
└── README.md
```

- `backend/` contains the cache models, timing logic, CLI entry point, and HTTP server
- `web/` contains the frontend interface
- The frontend communicates with `http://localhost:8080/simulate`

## Running the Project

### Requirements

- `g++` with C++17 support
- `make`
- Python 3, or any simple static file server
- A modern browser

### 1. Start the backend

```bash
cd backend
make
./server
```

The API will be available at:

```text
http://localhost:8080
```

### 2. Start the frontend

```bash
cd web
python3 -m http.server 5500
```

Then open:

```text
http://localhost:5500
```

## HTTP API

The frontend sends requests to:

```text
GET /simulate
```

Example query parameters:

```text
cacheType=FA
policy=LRU
pattern=WorkingSet64
numAccesses=200000
hitLatency=1
missPenalty=80
cpuFrequencyGHz=3.0
```

Example response fields:

- `hits`
- `misses`
- `hitRate`
- `replacements`
- `hitLatency`
- `missPenalty`
- `cpuFrequencyGHz`
- `totalCycles`
- `amat`
- `estimatedTimeNs`

## CLI Usage

The backend also includes a simple command-line test binary:

```bash
cd backend
make test
./test_simulator FA LRU Sequential 1000
```

This prints the simulation summary directly in the terminal.

## Screenshots

![Cache Simulator Screenshot 1](web/screenshots/screenshot1.png)

---

![Cache Simulator Screenshot 2](web/screenshots/screenshot2.png)

## Future Improvements

- Add configurable cache size and block size in the UI
- Support write-through and write-back policies
- Add multi-level cache simulation
- Expand automated test coverage
- Support side-by-side comparison between configurations
