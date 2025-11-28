# Market Data Streamer and Order Book Engine

High-throughput C++ system that replays historical market data over TCP to multiple clients, which reconstruct limit order books and track best bid/offer (BBO) in real time.

## At a Glance

* *High throughput*: ~1M messages/second (single client) and ~2.5M messages/second aggregate (five clients).

* *Order book engine*: builds order books from MBO (message-by-order) data using exchange-style semantics (e.g. modify changes order priority).

* *Modern C++ implementation*: uses `std::unordered_map` and tracked best bid/ask for speed, batched TCP sends, and a simple thread pool for scaling.

* *Tests and benchmarks included*: Catch2 unit and integration tests and a benchmark script that runs with a user-selected number of clients.

![Class diagram](/docs/system-diag.png)

## Benchmarks and Tests

**Benchmark script** (`run_bench.sh`) runs system with user-selected number of clients and reports:
* Messages sent
* Messages received
* Error rates
* Throughput (messages/second)

**Catch2 tests:**
* Unit tests for order book engine
* Unit tests for TCP mechanics
* Integration tests covering entire pipeline

## Performance

**Single Client**
~1M messages/second

**Five Clients** ~500k messages/second per client

Benchmarks were run on i7 dual-core laptop with Ubuntu 22.04.


## Getting Started

*Prerequisites*
* Linux environment (tested on Ubuntu 22.04)
* CMake 3.24 or later
* A modern C++ compiler (supporting C++17 or later)

*Build Instructions*

```
git clone https://github.com/ab2163/market-data-streamer.git
cd market-data-streamer
cmake -B build
cmake --build build
```

*Running Benchmarks*

```
./run_bench.sh [num_clients]
