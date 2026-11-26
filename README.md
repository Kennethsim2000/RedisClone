# Redis-Like C++ Store

A Redis-inspired in-memory key-value database implemented in **C++17**.

This is a step-by-step systems programming project. The goal is to understand how a small Redis-like server is built, from its storage engine and command execution path to networking, expiration, eviction, and persistence.

## Architecture

```text
Client
  -> TCP Listener
  -> Event Loop
  -> RESP-like Protocol Parser
  -> Command Router
  -> Execution Engine
       |-> In-Memory Store (HashMap)
       |-> TTL / Expiry
       |-> Eviction Policy (LRU / LFU)
       |-> Persistence Layer (AOF / Snapshot)
```

The implementation will be developed incrementally. The initial version focuses on a small set of string key-value commands rather than full Redis compatibility.

## Development roadmap

1. Project skeleton and tooling
2. In-memory key-value store
3. Command model and execution engine
4. RESP-like protocol parser and encoder
5. TCP server and event loop
6. TTL and key expiration
7. Memory accounting and LRU/LFU eviction
8. AOF persistence
9. Snapshot persistence
10. Integration tests, observability, and performance benchmarks

## Project guide

See [`skill.md`](./skill.md) for the detailed architecture, phase-by-phase implementation plan, suggested repository structure, testing strategy, design considerations, and the collaboration workflow.

## Planned technology

- C++17
- CMake
- GoogleTest
- Non-blocking sockets behind a networking abstraction
- Sanitizer-enabled development builds (ASan / UBSan)

Platform-specific event-loop implementations may be added as the project evolves (for example, `poll`/`epoll` on Linux or `kqueue` on macOS).

## Design principles

- Build one small, testable component at a time.
- Keep networking, protocol, command execution, storage, and persistence separate.
- Start with a single-threaded execution model; optimize only after measuring.
- Make ownership, error handling, and durability guarantees explicit.
- Test edge cases such as partial network reads, malformed requests, expired keys, and interrupted persistence writes.
- Clearly document where this project intentionally differs from Redis internals and behavior.

## Building and testing

### Prerequisites

- CMake 3.16 or newer
- A C++17-compatible compiler (Apple Clang, Clang, GCC, or MSVC)
- Internet access on the first configure, so CMake can download GoogleTest through FetchContent

### Configure and build

From the repository root:

```bash
cmake -S . -B build
cmake --build build
```

Run the application:

```bash
./build/redis_clone
```

### Run tests

Build the test executable and run all registered tests with CTest:

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

To configure without building the test target:

```bash
cmake -S . -B build -DBUILD_TESTING=OFF
```

## Continuous integration

GitHub Actions runs the CMake build and test suite on every push and pull request targeting any branch. The workflow is defined in [`.github/workflows/ci.yml`](./.github/workflows/ci.yml).

## Current status

- [ ] Phase 0 — Project skeleton and tooling
- [ ] Phase 1 — In-memory key-value store
- [ ] Phase 2 — Command model and execution engine
- [ ] Phase 3 — RESP-like protocol
- [ ] Phase 4 — TCP server and event loop
- [ ] Phase 5 — TTL and expiration
- [ ] Phase 6 — Memory accounting and eviction
- [ ] Phase 7 — AOF persistence
- [ ] Phase 8 — Snapshot persistence
- [ ] Phase 9 — Integration and performance

## Disclaimer

This is an educational Redis-inspired implementation, not a drop-in Redis replacement. Full Redis command compatibility, clustering, replication, authentication, and other advanced features are outside the initial project scope.
