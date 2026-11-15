# Redis-Like C++ Store — Project Skill

## 1. Project objective

Build a Redis-inspired in-memory key-value database in **C++17**. Develop it incrementally, keeping every milestone small, testable, and understandable.

The goal is not to reproduce all of Redis. The project is intended to teach and implement:

- TCP client/server communication and event-driven I/O
- RESP-like request/response framing
- Command parsing, routing, validation, and execution
- In-memory key-value storage
- TTL-based expiration
- Memory accounting and LRU/LFU eviction
- Append-only-file (AOF) persistence
- Snapshot-based persistence

The architecture is a starting point. Adjust it when implementation reveals a better design, and document important changes.

## 2. High-level architecture

```text
Client (redis-cli / app)
        |
        v
TCP Listener (sockets)
        |
        v
Event Loop (poll initially; epoll/kqueue later)
        |
        v
Protocol Parser (RESP-like)
        |
        v
Command Router
        |
        v
Execution Engine
        |
        +---------------------+
        |                     |
        v                     v
In-Memory Store           TTL / Expiry
(HashMap)                     |
        |                     |
        +----------+----------+
                   |
                   v
             Eviction Policy
                (LRU / LFU)

Execution Engine
        |
        v
Persistence Layer
        |
        +---------------------+
        |                     |
        v                     v
       AOF                 Snapshot
```

### Component responsibilities

| Component | Responsibility |
|---|---|
| TCP listener | Create the listening socket and accept clients |
| Event loop | Monitor socket readiness and drive non-blocking reads/writes |
| Connection | Own per-client input/output buffers and parser state |
| Protocol parser | Convert incoming bytes into structured requests |
| Command router | Map command names to command handlers |
| Execution engine | Validate and execute commands against database state |
| In-memory store | Own key/value entries and expose lookup/mutation operations |
| TTL manager | Track expiration deadlines and remove expired keys |
| Eviction policy | Select keys to remove when memory limits are reached |
| Persistence layer | Coordinate durable writes and startup recovery |
| AOF | Record write operations for replay |
| Snapshot | Save a point-in-time representation of database state |

**Boundary rule:** networking and protocol code must not manipulate the HashMap directly. Requests flow through parsing, routing, and execution before reaching storage.

## 3. Development principles

1. **Build vertically:** finish a small end-to-end feature before adding another subsystem.
2. **Keep components independently testable:** storage works without TCP; protocol parsing works without a database.
3. **Prefer explicit ownership:** make ownership of sockets, connections, buffers, values, and persistence resources clear.
4. **Start single-threaded:** establish correctness and ordering before introducing concurrency.
5. **Use binary-safe data structures:** values and protocol payloads must not rely on null termination.
6. **Test failure paths:** malformed requests, partial reads, disconnects, expired keys, and disk failures are first-class cases.
7. **Measure before optimizing:** add benchmarks after correctness tests exist.
8. **Keep scope controlled:** implement the components in this guide before considering advanced Redis features.
9. **Document Redis differences:** distinguish this educational implementation from Redis's actual behavior and internals.

## 4. Suggested repository structure

Adapt as needed, but preserve clear layer boundaries.

```text
RedisClone/
├── CMakeLists.txt
├── README.md
├── skill.md
├── docs/
│   └── architecture.md
├── include/
│   ├── net/
│   │   ├── tcp_server.h
│   │   ├── event_loop.h
│   │   └── connection.h
│   ├── protocol/
│   │   ├── resp_parser.h
│   │   └── resp_value.h
│   ├── command/
│   │   ├── command.h
│   │   ├── command_router.h
│   │   └── execution_engine.h
│   ├── storage/
│   │   ├── store.h
│   │   └── value.h
│   ├── expiry/
│   │   └── ttl_manager.h
│   ├── eviction/
│   │   ├── eviction_policy.h
│   │   ├── lru_policy.h
│   │   └── lfu_policy.h
│   └── persistence/
│       ├── persistence_manager.h
│       ├── aof.h
│       └── snapshot.h
├── src/
├── tests/
│   ├── protocol/
│   ├── command/
│   ├── storage/
│   ├── expiry/
│   ├── eviction/
│   ├── persistence/
│   └── integration/
└── benchmarks/
```

Use CMake and C++17. GoogleTest is a suitable test framework. Build production components as libraries where practical so unit tests do not need to start the server.

---

# 5. Step-by-step implementation roadmap

## Phase 0 — Project skeleton and tooling

**Goal:** establish a reliable build and test workflow before implementing database behavior.

Tasks:

- [ ] Create the CMake project and explicitly require C++17.
- [ ] Separate headers, implementation files, tests, and documentation.
- [ ] Add GoogleTest and a test executable.
- [ ] Enable useful compiler warnings.
- [ ] Add optional AddressSanitizer and UndefinedBehaviorSanitizer settings.
- [ ] Add a minimal executable that starts and exits cleanly.
- [ ] Document clean build and test commands.

**Completion criteria:** a clean build succeeds, a sample unit test passes, and sanitizer builds can be run locally.

Do not start networking until the build and test workflow is reliable.

## Phase 1 — In-memory key-value store

**Goal:** implement the database core without networking.

Start with a string-to-string store backed initially by:

```cpp
std::unordered_map<std::string, std::string>
```

Hide the container behind a `Store` interface so later features do not depend directly on the HashMap.

Initial operations:

- [ ] `set(key, value)`
- [ ] `get(key)`
- [ ] `del(key)`
- [ ] `exists(key)`
- [ ] `size()`
- [ ] `clear()`

Decide missing-key semantics; for example, `get` may return `std::optional<std::string>`.

Tests:

- [ ] Insert and retrieve a key.
- [ ] Overwrite an existing key.
- [ ] Retrieve a missing key.
- [ ] Delete existing and missing keys.
- [ ] Store empty strings.
- [ ] Store values with spaces and embedded null bytes.
- [ ] Insert many keys.

**Completion criteria:** the store is independently unit-tested and usable without a server.

Do not add lists, sets, hashes, sorted sets, or other Redis data types yet.

## Phase 2 — Command model and execution engine

**Goal:** represent commands as structured data and execute them without sockets.

A parsed request should become an owning representation conceptually similar to:

```text
Command {
    name: "SET"
    arguments: ["user:1", "Kenneth"]
}
```

Tasks:

- [ ] Define command/request and response representations.
- [ ] Normalize command names.
- [ ] Implement a command router.
- [ ] Implement an execution engine.
- [ ] Add `PING`, `ECHO`, `SET`, `GET`, `DEL`, and `EXISTS`.
- [ ] Validate argument counts and values.
- [ ] Define consistent errors for unknown commands and invalid arguments.

The router selects the handler; the execution engine coordinates the handler with the store. Avoid one giant `switch` statement if a handler interface or registry gives a cleaner extension point.

Tests:

- [ ] Valid command execution.
- [ ] Missing and extra arguments.
- [ ] Unknown command.
- [ ] Command-name casing behavior.
- [ ] Correct response type and content.
- [ ] Multiple commands executed against the same store.

**Completion criteria:** tests can execute command sequences without starting a server.

## Phase 3 — RESP-like protocol parser and encoder

**Goal:** convert protocol bytes into requests and responses.

Start with a documented subset of RESP2. Do not claim full RESP compatibility until all supported types and edge cases are implemented.

Initial types:

- [ ] Simple strings
- [ ] Errors
- [ ] Integers
- [ ] Bulk strings
- [ ] Arrays

Support the array-of-bulk-strings form used for ordinary Redis-style command requests.

Parser requirements:

- [ ] Parse from byte buffers; never assume one socket read equals one request.
- [ ] Handle a request split across multiple reads.
- [ ] Handle multiple requests in one read.
- [ ] Distinguish incomplete input from malformed input.
- [ ] Enforce maximum bulk-string and array lengths.
- [ ] Handle binary payloads and framing correctly.
- [ ] Avoid unbounded allocation based on untrusted length prefixes.

Use an explicit parser result such as **Complete**, **Incomplete**, or **Error**. The parser should retain or be given enough buffered input to resume after an incomplete parse.

Encoder requirements:

- [ ] Encode every supported response type.
- [ ] Count bulk-string lengths in bytes.
- [ ] Test empty and binary-safe bulk strings.
- [ ] Test exact CRLF framing.

Tests should split input at meaningful boundaries: before a type marker, inside a length prefix, inside a payload, and before the terminating CRLF.

**Completion criteria:** parser and encoder are independently tested and have no dependency on sockets or storage.

## Phase 4 — TCP server and event loop

**Goal:** allow clients to connect and execute commands over TCP.

Start with a single-threaded, non-blocking server. Hide platform-specific readiness APIs behind an event-loop abstraction. A portable first implementation can use `poll`; Linux can later use `epoll`, while macOS can use `kqueue`.

Tasks:

- [ ] Create, bind, and listen on a configurable socket.
- [ ] Accept client connections.
- [ ] Set sockets to non-blocking mode.
- [ ] Track active connections.
- [ ] Monitor read readiness.
- [ ] Read available bytes into each connection's input buffer.
- [ ] Pass buffered bytes to the protocol parser.
- [ ] Route complete commands to the execution engine.
- [ ] Encode responses and queue them in a per-connection output buffer.
- [ ] Monitor write readiness while output is pending.
- [ ] Handle disconnects, errors, and orderly shutdown.

Important concepts:

- **Partial reads:** one `recv` call may contain part of a request or several requests.
- **Partial writes:** one `send` call may send only part of a response.
- **Backpressure:** slow clients can cause output buffers to grow. Set a maximum and define a policy for slow clients.
- **Connection isolation:** parser state and buffers belong to each connection; one client's incomplete request must not affect another.

Tests:

- [ ] Connect and send `PING`.
- [ ] Send one request in multiple chunks.
- [ ] Send multiple requests in one write.
- [ ] Disconnect halfway through a request.
- [ ] Connect multiple clients.
- [ ] Exercise a client that reads slowly.
- [ ] Verify clean shutdown.

**Completion criteria:** a Redis-style client or custom TCP client can execute supported commands.

## Phase 5 — TTL and key expiration

**Goal:** allow keys to expire after a configured duration.

Conceptual entry model:

```text
Entry {
    value
    optional expiration_deadline
}
```

Use `std::chrono::steady_clock` for in-process deadlines so wall-clock adjustments do not unexpectedly change expiration behavior.

Commands:

- [ ] `EXPIRE key seconds`
- [ ] `PEXPIRE key milliseconds`
- [ ] `TTL key`
- [ ] `PTTL key`
- [ ] `PERSIST key`

Define and test what happens when `SET` overwrites a key that already has a TTL.

Implement expiration in two stages:

1. **Lazy expiration:** when a key is accessed, check its deadline; expired keys behave as absent and are removed.
2. **Active expiration:** periodically inspect a bounded batch or sample of keys and remove expired entries.

Do not scan every key on every request.

Tests:

- [ ] Key without a TTL.
- [ ] Key expires after its deadline.
- [ ] Access to an expired key behaves as a missing key.
- [ ] Remaining TTL behavior.
- [ ] Remove a TTL with `PERSIST`.
- [ ] Overwrite and expiration semantics.
- [ ] Active expiration removes stale entries.

**Completion criteria:** expiration is correct and does not require a full scan on every command.

## Phase 6 — Memory accounting and eviction

**Goal:** enforce a configurable memory limit and remove keys when required.

First define what the limit measures. A practical first version can estimate usage from key bytes, value bytes, and known entry overhead. Document that this is an estimate, not exact process RSS.

Tasks:

- [ ] Add a configurable `maxmemory`.
- [ ] Track estimated usage on insert, overwrite, delete, and expiration.
- [ ] Define behavior when a write exceeds the limit.
- [ ] Introduce an eviction-policy interface.
- [ ] Implement LRU.
- [ ] Implement LFU separately.
- [ ] Test policy behavior and accounting.

### LRU learning path

Start with exact LRU:

- HashMap from key to entry/metadata.
- Doubly linked list ordered by recency.
- Successful access moves the key to the most-recently-used end.
- Eviction removes the least-recently-used key.

A list plus HashMap gives expected O(1) lookup and recency updates. Later, investigate Redis-style approximate LRU sampling, which reduces the overhead of maintaining exact global recency.

### LFU learning path

Treat LFU as a distinct policy:

1. Start with a per-key access counter.
2. Define counter updates and aging.
3. Add decay so old popularity does not dominate forever.
4. Consider probabilistic increments to limit counter growth.
5. Compare with Redis's approximate LFU design.

Do not call a simple exact counter Redis's LFU implementation. Redis uses probabilistic counter updates and counter decay.

Tests:

- [ ] Memory usage increases on insertion.
- [ ] Usage changes correctly on overwrite.
- [ ] Usage decreases on deletion and expiration.
- [ ] LRU evicts the least-recently-used eligible key.
- [ ] LFU favors frequently accessed keys in controlled tests.
- [ ] Expired keys are not retained unnecessarily as eviction candidates.
- [ ] Behavior when one value exceeds the limit is defined.

**Completion criteria:** memory limits are enforced and eviction behavior is testable.

## Phase 7 — AOF persistence

**Goal:** recover database state after restart by recording write operations.

Start with an append-only command log containing enough information to replay supported writes.

Tasks:

- [ ] Define the AOF record format.
- [ ] Append supported write commands.
- [ ] Define when a command is considered committed.
- [ ] Replay AOF records during startup.
- [ ] Handle incomplete trailing records after a crash.
- [ ] Add configurable fsync behavior.
- [ ] Handle disk-full and write errors.
- [ ] Test restart and recovery.

Document durability trade-offs:

- Sync every write: stronger durability, higher latency.
- Sync periodically: bounded potential loss window depending on interval.
- Rely on OS flushing: potentially better performance, weaker guarantees.

Do not claim a write is durable merely because it entered a userspace buffer.

Choose and document the ordering between in-memory mutation and AOF append. If append fails, should the command fail without applying the mutation, or can the mutation remain while reporting a persistence error? Make this a deliberate consistency policy and test it.

Tests:

- [ ] Write commands are recorded.
- [ ] Restart replays records correctly.
- [ ] Multiple writes to one key replay in order.
- [ ] Deletes replay correctly.
- [ ] Incomplete final record is handled safely.
- [ ] AOF write failure is surfaced.
- [ ] Configured fsync behavior is exercised where practical.

**Completion criteria:** restart preserves supported writes according to the documented durability policy.

## Phase 8 — Snapshot persistence

**Goal:** save a point-in-time representation of the store and load it on startup.

AOF records operations; a snapshot records database state at a point in time.

Tasks:

- [ ] Define a versioned snapshot format.
- [ ] Serialize keys, values, and expiration metadata.
- [ ] Add bounds checking and corruption detection.
- [ ] Write snapshots to a temporary file.
- [ ] Flush as required by the durability design.
- [ ] Atomically rename the completed file into place where supported.
- [ ] Load and validate snapshots during startup.
- [ ] Define how snapshot loading and AOF replay combine.

Consistency questions:

- What if the process crashes while writing a snapshot?
- How do we prevent a partial snapshot replacing a valid one?
- How are AOF operations newer than the snapshot replayed?
- How are expired keys treated during recovery?
- How are format changes handled across versions?

Tests:

- [ ] Snapshot and restore an empty store.
- [ ] Snapshot and restore many keys.
- [ ] Preserve binary values.
- [ ] Preserve TTL semantics.
- [ ] Reject or safely handle corrupted snapshots.
- [ ] Recover using a snapshot plus a newer AOF segment.

**Completion criteria:** valid snapshots restore correctly, and interrupted writes do not silently replace the last known-good snapshot.

## Phase 9 — Integration, observability, and performance

**Goal:** make the system easier to operate, debug, and evaluate.

Tasks:

- [ ] Add consistent logs.
- [ ] Add basic statistics: connected clients, commands processed, key count, estimated memory, and evictions.
- [ ] Add integration tests using real TCP connections.
- [ ] Add stress tests for many requests and clients.
- [ ] Benchmark GET, SET, and mixed workloads.
- [ ] Measure parser throughput and event-loop behavior.
- [ ] Measure TTL and eviction overhead.
- [ ] Measure AOF overhead under each durability setting.
- [ ] Document limitations and unsupported commands.

Benchmark release builds with repeatable workloads. Measure latency as well as throughput. Compare persistence-enabled runs with a no-persistence baseline. Optimize only after profiling identifies a bottleneck.

---

## 6. Initial command scope

This is the intended project scope, not a promise of full Redis compatibility.

| Stage | Commands |
|---|---|
| Basic | `PING`, `ECHO` |
| Key/value | `SET`, `GET`, `DEL`, `EXISTS` |
| Expiration | `EXPIRE`, `PEXPIRE`, `TTL`, `PTTL`, `PERSIST` |
| Numeric operation | `INCR` |
| Inspection | `DBSIZE`, optionally a limited `INFO` |

For every command, document argument count and types, success response, missing-key behavior, errors, whether it mutates state, whether it must be persisted, and whether it affects eviction metadata.

Add commands only after existing semantics and tests are stable.

## 7. Cross-cutting engineering decisions

### Threading model

Begin with one event-loop thread and one database execution context. Add worker threads or concurrent command execution only after measuring a bottleneck and defining synchronization and command-ordering guarantees.

### Time handling

Use `steady_clock` for in-process TTL deadlines. Persistence needs a representation meaningful after restart; store an appropriate wall-clock expiration timestamp or remaining lifetime and define recovery behavior carefully.

### Memory ownership

Prefer value types and clear ownership. Avoid raw owning pointers. Use RAII for sockets, file descriptors, and file handles. Make buffer lifetimes explicit.

### Error handling

Distinguish client/protocol errors, invalid command errors, resource-limit errors, internal errors, persistence errors, and network errors. Never silently turn an internal failure into a successful response.

### Configuration

Eventually support bind address/port, maximum clients, request/response size limits, maximum memory, eviction policy, persistence mode and paths, AOF fsync policy, and logging level. Add settings as their features are implemented.

## 8. Testing strategy

| Test layer | What it verifies |
|---|---|
| Unit tests | Store, parser, encoder, TTL manager, eviction policies |
| Component tests | Router and execution engine with a real store |
| Network tests | Partial reads/writes, multiple clients, disconnects |
| Persistence tests | Restart, replay, snapshots, corruption handling |
| Stress tests | Large key counts, many requests, slow clients |
| Benchmarks | Throughput, latency, memory overhead |

Use sanitizer builds for memory and undefined-behavior bugs. Add a regression test for every bug fixed.

## 9. Non-goals for the first version

Keep these out of the initial implementation unless the core system is stable:

- Full Redis command compatibility
- Redis Cluster and sharding
- Replication and Sentinel
- Lua scripting
- Pub/Sub
- Transactions and WATCH/MULTI/EXEC
- TLS
- Authentication and ACLs
- Redis modules
- Multi-threaded command execution
- Advanced data types such as streams and sorted sets

These are possible future extensions, not prerequisites for completing the first version.

## 10. How ChatGPT should guide this project

Use this as the working agreement when continuing the project:

1. **Proceed one step at a time.** Work on the current phase and task only; do not dump the entire implementation at once.
2. **Explain before coding.** Explain the component's purpose, responsibilities, and interactions first.
3. **Prefer TODO-driven implementation.** Give file structure, interfaces, and TODOs first. Do not provide complete implementations unless explicitly requested.
4. **Review the user's code.** Inspect supplied code in context and suggest focused changes rather than replacing everything.
5. **Explain trade-offs.** Discuss ownership, complexity, memory use, latency, failure handling, and testability.
6. **Add tests with every feature.** Define expected behavior and edge cases before considering a phase complete.
7. **Do not skip prerequisites.** Test parser and execution layers independently before integrating networking.
8. **Keep documentation aligned.** Update the architecture and README when design decisions change.
9. **Be precise about Redis differences.** Distinguish this project's simplified behavior from Redis internals.
10. **Wait for the user to finish a step.** Do not move to the next implementation task until the current one is understood and validated.

### Suggested interaction format for each task

1. **Goal** — what we are implementing now.
2. **Why it exists** — its role in the architecture.
3. **Design** — relevant data structures and interfaces.
4. **Files** — files to create or modify.
5. **TODOs** — small implementation tasks in order.
6. **Tests** — what to test and what each test proves.
7. **Review** — inspect the implementation and resolve issues before moving on.

### Current project status

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

**Starting point:** Phase 0, unless an existing repository implementation should be inspected first.
