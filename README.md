# TRACE Trade Ingestion Pipeline

A high-performance FINRA TRACE bond trade ingestion system in C++17. Multiple TCP feeds stream simulated trade data through a **lockless multi-producer/multi-consumer (MPMC) queue** into PostgreSQL, with real-time issuer enrichment. Designed to demonstrate low-latency concurrent data processing for financial market data.

![Build Status](https://github.com/AsymptoticEpiphany/finra-trace-pipeline/actions/workflows/ci_cd_jobs.yml/badge.svg)

## Architecture

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│  TRACE Feed     │     │  TRACE Feed     │     │  TRACE Feed     │
│  (TCP :5555)    │     │  (TCP :5556)    │     │  (TCP :5557)    │
└────────┬────────┘     └────────┬────────┘     └────────┬────────┘
         │                       │                       │
         │    JSON trade msgs    │                       │
         ▼                       ▼                       ▼
┌────────────────────────────────────────────────────────────────┐
│                     Producer Threads (3)                       │
│  • Parse JSON trade data from TCP streams                     │
│  • Enrich with issuer rating/industry from in-memory cache    │
│  • Enqueue onto lockless MPMC queue                           │
└────────────────────────────┬───────────────────────────────────┘
                             │
                             ▼
              ┌──────────────────────────┐
              │    Lockless MPMC Queue   │
              │  • Fixed-size ring buffer│
              │  • Cache-line aligned    │
              │  • Zero heap allocation  │
              │  • Lock-free CAS ops     │
              └──────────────┬───────────┘
                             │
                ┌────────────┴────────────┐
                ▼                         ▼
     ┌──────────────────┐     ┌──────────────────┐
     │  Consumer 1      │     │  Consumer 2      │
     │  → PostgreSQL    │     │  → PostgreSQL    │
     └──────────────────┘     └──────────────────┘
```

The system simulates a realistic bond trade data environment: the Python-based TRACE feed generator produces trade messages with valid CUSIP check digits, proper buy/sell trade pairing, execution and report timestamps with late-trade modifier detection (FINRA's 15-minute reporting window), and configurable throughput with burst and jitter modes.

## Lockless MPMC Queue — Design Rationale

The core data structure is a bounded, lock-free multi-producer/multi-consumer queue based on Dmitry Vyukov's classic design. The implementation makes deliberate choices about memory ordering, layout, and synchronization that are worth explaining.

### How It Works

Each slot in the ring buffer carries a **sequence number** alongside the data. The sequence number acts as both a state flag and a synchronization mechanism:

- **Enqueue:** A producer loads the current `tail` position, checks if the slot's sequence number matches that position (meaning the slot is free), and atomically claims the position via `compare_exchange_weak` on `tail`. After writing data, it advances the slot's sequence number to signal that data is available.
- **Dequeue:** A consumer does the mirror operation on `head` — it checks if the slot's sequence equals `pos + 1` (meaning data has been written), claims the position, reads the data, and advances the sequence by `Capacity` to mark the slot as free for reuse.

The key insight is the **separation of claiming and acting**. The CAS on head/tail only claims a position — the actual data read/write happens after the claim, and the sequence number update is what publishes the result to other threads.

### Memory Ordering Choices

Every atomic operation in the queue uses the minimum memory ordering required for correctness:

- **`memory_order_relaxed` on the `compare_exchange_weak` for head/tail:** The CAS itself only needs to be atomic — it doesn't need to order any other memory operations. The *sequence number* is what provides the inter-thread happens-before relationship, not the head/tail update.
- **`memory_order_acquire` on loading the sequence number:** This ensures that when a consumer sees a sequence value indicating data is ready, all preceding writes by the producer (including the data itself) are visible. Without acquire here, a consumer could observe the updated sequence but read stale data.
- **`memory_order_release` on storing the sequence number after write:** This ensures that the data write is visible to any thread that subsequently loads this sequence with acquire semantics. The release-acquire pair on the sequence number is the actual synchronization edge.

Using `seq_cst` everywhere would be correct but unnecessarily expensive — it would insert full memory barriers on every operation. The relaxed/acquire/release combination gives the same correctness guarantees with significantly less overhead on x86-64 (where acquire/release map to plain loads/stores) and substantially less overhead on ARM (where they avoid unnecessary `dmb` barriers).

### Cache-Line Isolation

False sharing — where unrelated data on the same cache line causes invalidation traffic between cores — is one of the primary performance killers in concurrent data structures. The queue addresses this in two places:

- **Each data slot** is padded and aligned to 64 bytes (one cache line on x86-64), so adjacent slots don't share a cache line. A producer writing to slot N doesn't invalidate the cache for a consumer reading slot N-1.
- **The head and tail counters** are each wrapped in a `PaddedAtomic` struct aligned to 64 bytes, ensuring they occupy separate cache lines. Without this, producer threads incrementing `tail` would constantly invalidate the cache line for consumer threads reading `head`, even though these are logically independent.

### Compile-Time Constraints

The queue capacity is enforced at compile time to be a power of 2 (`static_assert`), which allows index wrapping via bitwise AND (`pos & (Capacity - 1)`) instead of modulo division. On most architectures, this is a single-cycle operation versus a multi-cycle `div` instruction — a small optimization that matters at high throughput.

## TRACE Feed Simulator

The `fake_trace_generator.py` script simulates a FINRA TRACE-like bond market data feed with realistic characteristics:

- **Valid CUSIPs** with proper check-digit computation (Luhn-variant algorithm)
- **Trade pairing** — configurable probability of emitting matched buy/sell legs with shared control IDs, simulating inter-dealer trades
- **Late-trade detection** — trades reported more than 15 minutes after execution are flagged with `modifier3: "Z"`, mirroring FINRA's actual reporting convention
- **Configurable throughput** — adjustable message rate, rate jitter, and periodic burst modes for stress testing the pipeline under varying load profiles

## Building and Running

### Prerequisites

- C++17 compiler (GCC 7+ or Clang 5+)
- CMake 3.14+
- PostgreSQL with `libpq` development headers
- Python 3 (for the TRACE feed simulator)

### Build

```bash
make build             # Debug build with memory sanitizers (ASan/UBSan/LSan)
make release           # Optimized production build
make test              # Run all unit tests with sanitizer checks
make clean             # Remove build artifacts
```

### Run the Pipeline

```bash
# Terminal 1-3: Start TRACE feed simulators on separate ports
python3 fake_trace_generator.py --tcp --port 5555
python3 fake_trace_generator.py --tcp --port 5556
python3 fake_trace_generator.py --tcp --port 5557

# Terminal 4: Run the pipeline (requires PostgreSQL — see below)
make run
```

The pipeline connects to each TCP feed, parses incoming JSON trade messages, enriches them with issuer metadata from a PostgreSQL lookup table, pushes them through the MPMC queue, and persists them to a `trades` hypertable via consumer threads.

### Database Setup

The system expects a PostgreSQL database named `finance` with an `issuer_info` table (issuer, rating, industry) and a `trades` table. Connection parameters are currently in `main.cpp` — command-line configuration is on the roadmap.

## Tests

Unit tests use GoogleTest and cover the MPMC queue across several dimensions:

- **Single-thread correctness** — basic enqueue/dequeue, FIFO ordering
- **Capacity boundaries** — full queue returns false, wraparound works correctly
- **Multi-threaded stress** — 40 producers × 10,000 items each, 4 consumers, verifying zero data loss across 400,000 total operations
- **Benchmark** — single-producer/single-consumer throughput measurement (ops/sec)

All tests run under AddressSanitizer, UndefinedBehaviorSanitizer, and LeakSanitizer in CI. The stress test is specifically designed to surface race conditions, ABA problems, and memory ordering bugs under high contention.

```bash
make test    # Run tests locally with sanitizers
```

## CI/CD

GitHub Actions runs on every push and pull request:

1. **Debug build with sanitizers** — compile with ASan/UBSan/LSan, run all tests
2. **Release build** — optimized binary uploaded as a CI artifact
3. **Tagged releases** — versioned binaries published to GitHub Releases automatically

## Project Structure

```
├── src/
│   ├── mpmc_queue.h              # Lockless MPMC queue (header-only)
│   └── main.cpp                  # Pipeline: TCP ingest → queue → PostgreSQL
├── tests/
│   ├── test_mpmc_queue.cpp       # Queue correctness, stress, and benchmark tests
│   └── test_print_tuple.cpp      # Tuple pretty-printer tests
├── utils/
│   └── print_tuple.h             # Variadic tuple/pair printer (C++17 metaprogramming)
├── fake_trace_generator.py       # TRACE feed simulator with trade pairing
├── CMakeLists.txt                # Build configuration
├── Makefile                      # Convenience build targets
└── .github/workflows/            # CI/CD pipeline
```
